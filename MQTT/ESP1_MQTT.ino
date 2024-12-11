#include <SCServo.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>

#define S_RXD 16
#define S_TXD 17

const char* ssid = "Techolution";
const char* password = "techolutionisthebe$t";
const char* mqtt_server = "192.168.1.100";
const int mqtt_port = 1820;

WiFiClient espClient;
PubSubClient client(espClient);
SMS_STS sms_sts;
Servo myservo;

byte ID[6] = {2, 5, 3, 7, 4, 6};
s16 positionSync[6] = {0};
u16 speedSync[6] = {1000, 1000, 1000, 1000, 1000, 1000};
byte accelerationSync[6] = {20, 20, 20, 20, 20, 20};
bool isMoving[6] = {false};

const int servoPin = 23;
int pos = 0;
bool positionChanged = false;

void setupWiFi();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);
void updateActuatorCommand(const String& topic, const JsonDocument& doc);
void publishFeedback(int index);

void setup()
{
  Serial.begin(115200);
  Serial1.begin(1000000, SERIAL_8N1, S_RXD, S_TXD);
  sms_sts.pSerial = &Serial1;
  setupWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  for (int i = 0; i < 6; i++) 
  {
    client.subscribe(("actuator/" + String(ID[i]) + "/command").c_str());
  }

  client.subscribe("actuator/wrist/command");

  ESP32PWM::allocateTimer(0);
  myservo.setPeriodHertz(50);
  myservo.attach(servoPin, 500, 2500);
  Serial.printf("Setup complete.\n");
}

void loop()
{
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();

  sms_sts.SyncWritePosEx(ID, 6, positionSync, speedSync, accelerationSync);

  for (int i = 0; i < 6; i++) 
  {
    bool moving = sms_sts.ReadMove(ID[i]);
    if (moving != isMoving[i] && isMoving[i] != false) 
    {
      isMoving[i] = moving;
      publishFeedback(i);
    }
  }

  if (positionChanged) 
  {
    myservo.write(pos);
    Serial.printf("Servo moved to position: %d\n", pos);
    positionChanged = false;
  }

  delay(10);
}

void setupWiFi() 
{
  Serial.printf("Connecting to Wi-Fi: %s\n", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.printf(".");
  }
  Serial.printf("\nWi-Fi connected. IP Address: %s\n", WiFi.localIP().toString().c_str());
}

void reconnect()
{
  while (!client.connected()) 
  {
    Serial.printf("Connecting to MQTT...");
    if (client.connect("ESP32_Servo_Client")) 
    {
      Serial.printf("Connected.\n");
      for (int i = 0; i < 6; i++) 
      {
        client.subscribe(("actuator/" + String(ID[i]) + "/command").c_str());
      }
      client.subscribe("actuator/wrist/command");
    } 
    else 
    {
      Serial.printf("Failed (rc=%d). Retrying in 5 seconds...\n", client.state());
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, payload, length)) 
  {
    Serial.printf("JSON Parsing Failed.\n");
    return;
  }

  String topicStr = String(topic);

  for (int i = 0; i < 6; i++) 
  {
    String targetTopic = "actuator/" + String(ID[i]) + "/command";
    if (topicStr == targetTopic) 
    {
      updateActuatorCommand(targetTopic, doc);
      return;
    }
  }

  if (topicStr == "actuator/wrist/command") 
  {
    int newPosition = doc["position"] | -1;
    if (newPosition >= 100 && newPosition <= 180) 
    {
      pos = newPosition;
      positionChanged = true;
      Serial.printf("ESP32 Servo Received Position: %d\n", pos);
    } 
    else 
    {
      Serial.printf("Invalid position for ESP32 Servo. Must be between 100-180.\n");
    }
  }
}

void updateActuatorCommand(const String& topic, const JsonDocument& doc) 
{
  for (int i = 0; i < 6; i++) 
  {
    if (topic.endsWith(String(ID[i]) + "/command")) 
    {
      int newPosition = doc["position"] | positionSync[i];
      int newSpeed = doc["speed"] | speedSync[i];
      int newAcceleration = doc["acceleration"] | accelerationSync[i];

      switch (ID[i])
      {
        case 2:
          if (newPosition < 2040 || newPosition > 2670) 
          {
            Serial.printf("Invalid position for Actuator %d. Allowed range: 2040-2670\n", ID[i]);
            return;
          }
          break;
        case 3:
          if (newPosition < 3760 || newPosition > 4040) 
          {
            Serial.printf("Invalid position for Actuator %d. Allowed range: 3760-4040\n", ID[i]);
            return;
          }
          break;
        case 4:
          if (newPosition < 110 || newPosition > 1070) 
          {
            Serial.printf("Invalid position for Actuator %d. Allowed range: 110-1070\n", ID[i]);
            return;
          }
          break;
        case 5:
          if (newPosition < 2420 || newPosition > 2870) 
          {
            Serial.printf("Invalid position for Actuator %d. Allowed range: 2420-2870\n", ID[i]);
            return;
          }
          break;
        case 6:
          if (newPosition < 1850 || newPosition > 2400) 
          {
            Serial.printf("Invalid position for Actuator %d. Allowed range: 1850-2400\n", ID[i]);
            return;
          }
          break;
        case 7:
          if (newPosition < 720 || newPosition > 1620) 
          {
            Serial.printf("Invalid position for Actuator %d. Allowed range: 720-1620\n", ID[i]);
            return;
          }
          break;
      }

      positionSync[i] = newPosition;
      speedSync[i] = newSpeed;
      accelerationSync[i] = newAcceleration;

      Serial.printf("Updated Actuator %d - Position: %d, Speed: %d, Acceleration: %d\n",
                    ID[i], positionSync[i], speedSync[i], accelerationSync[i]);
      return;
    }
  }
}

void publishFeedback(int index) 
{
  StaticJsonDocument<256> doc;
  doc["position"] = sms_sts.ReadPos(ID[index]);
  doc["speed"] = sms_sts.ReadSpeed(ID[index]);
  doc["load"] = sms_sts.ReadLoad(ID[index]);
  doc["voltage"] = sms_sts.ReadVoltage(ID[index]);
  doc["temperature"] = sms_sts.ReadTemper(ID[index]);
  doc["movement"] = sms_sts.ReadMove(ID[index]);
  doc["current"] = sms_sts.ReadCurrent(ID[index]);

  String topic = "actuator/" + String(ID[index]) + "/feedback";
  char buffer[256];
  serializeJson(doc, buffer);
  client.publish(topic.c_str(), buffer);

  Serial.printf("Published Feedback for Actuator %d\n", ID[index]);
}
