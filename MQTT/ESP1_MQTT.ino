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

byte ID[6] = {2, 3, 4, 5, 6, 7};
s16 positionSync[6] = {2040, 4040, 340, 2870, 1440, 2047};
u16 speedSync[6] = {1000, 1000, 1000, 1000, 1000, 1000};
byte accelerationSync[6] = {20, 20, 20, 20, 20, 20};

const int servoPin = 23;  
int pos = 180;
bool positionChanged = false;

void setupWiFi();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);
void publishFeedbackForAll();
void publishFeedback(int index);

void setup()
{
  Serial.begin(115200);
  Serial1.begin(1000000, SERIAL_8N1, S_RXD, S_TXD);
  sms_sts.pSerial = &Serial1;

  setupWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  client.subscribe("actuator/group/command");
  client.subscribe("actuator/wrist/command");

  ESP32PWM::allocateTimer(0);
  myservo.setPeriodHertz(50); 
  myservo.attach(servoPin, 500, 2500);

  Serial.println("Setup complete.");
}

void loop()
{
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();

  sms_sts.SyncWritePosEx(ID, 6, positionSync, speedSync, accelerationSync);

  if (positionChanged) 
  {
    myservo.write(pos);
    Serial.printf("ESP32 Servo moved to position: %d\n", pos);
    positionChanged = false;
  }
  else
  {
    myservo.write(pos);
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
    Serial.print(".");
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
      Serial.println("Connected.");
      client.subscribe("actuator/group/command");
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
  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, payload, length)) 
  {
    Serial.println("JSON Parsing Failed.");
    return;
  }

  String topicStr = String(topic);

  if (topicStr == "actuator/group/command") 
  {
    JsonArray positions = doc["positions"];
    JsonArray speeds = doc["speeds"];
    JsonArray accelerations = doc["accelerations"];

    if (positions.size() != 6 || speeds.size() != 6 || accelerations.size() != 6) 
    {
      Serial.println("Invalid array size. Must contain exactly 6 values.");
      return;
    }

    for (int i = 0; i < 6; i++) 
    {
      int newPosition = positions[i];
      int newSpeed = speeds[i];
      int newAcceleration = accelerations[i];

      if ((ID[i] == 2 && (newPosition < 2040 || newPosition > 2670)) ||
          (ID[i] == 3 && (newPosition < 3760 || newPosition > 4040)) ||
          (ID[i] == 4 && (newPosition < 110 || newPosition > 1070)) ||
          (ID[i] == 5 && (newPosition < 2420 || newPosition > 2870)) ||
          (ID[i] == 6 && (newPosition < 720 || newPosition > 1620)) ||
          (ID[i] == 7 && (newPosition < 1850 || newPosition > 2400)))
      {
        Serial.printf("Invalid position for Actuator %d: %d\n", ID[i], newPosition);
        return;
      }

      positionSync[i] = newPosition;
      speedSync[i] = newSpeed;
      accelerationSync[i] = newAcceleration;

      Serial.printf("Updated Actuator %d - Position: %d, Speed: %d, Acceleration: %d\n",
                    ID[i], positionSync[i], speedSync[i], accelerationSync[i]);
    }

    publishFeedbackForAll();
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
      Serial.println("Invalid position for ESP32 Servo. Must be between 100-180.");
    }
  }
}

void publishFeedbackForAll() 
{
  StaticJsonDocument<512> doc;
  JsonArray feedbackArray = doc.createNestedArray("feedback");

  for (int i = 0; i < 6; i++) 
  {
    JsonObject servoData = feedbackArray.createNestedObject();
    servoData["ID"] = ID[i];
    servoData["position"] = sms_sts.ReadPos(ID[i]);
    servoData["speed"] = sms_sts.ReadSpeed(ID[i]);
    servoData["load"] = sms_sts.ReadLoad(ID[i]);
    servoData["voltage"] = sms_sts.ReadVoltage(ID[i]);
    servoData["temperature"] = sms_sts.ReadTemper(ID[i]);
    servoData["movement"] = sms_sts.ReadMove(ID[i]);
    servoData["current"] = sms_sts.ReadCurrent(ID[i]);
  }

  char buffer[512];
  serializeJson(doc, buffer);
  client.publish("actuator/group/feedback", buffer);

  Serial.println("Published feedback for all actuators.");
}

