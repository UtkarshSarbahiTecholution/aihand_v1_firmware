#include <SCServo.h>
#include <WiFi.h>
#include <PubSubClient.h>
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

byte ID[6] = {2, 5, 3, 7, 4, 6};
s16 positionSync[6] = {0};
u16 speedSync[6] = {1000, 1000, 1000, 1000, 1000, 1000};
byte accelerationSync[6] = {20, 20, 20, 20, 20, 20};
bool isMoving[6] = {false};

void handleMQTT();
void updateActuatorCommand(const String& topic, const JsonDocument& doc);
void publishFeedback(int index);
void callback(char* topic, byte* payload, unsigned int length);
void setupWiFi();
void reconnect();

void setup()
{
  Serial1.begin(1000000, SERIAL_8N1, S_RXD, S_TXD);
  Serial.begin(115200);
  sms_sts.pSerial = &Serial1;
  setupWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  for (int i = 0; i < 6; i++)
  {
    client.subscribe(("actuator/" + String(ID[i]) + "/command").c_str());
  }
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
    if (moving != isMoving[i]) 
    {
      isMoving[i] = moving;
      publishFeedback(i);
    }
  }

  delay(100);
}

void setupWiFi() 
{
  Serial.println("\nConnecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected. IP: " + WiFi.localIP().toString());
}

void reconnect() 
{
  while (!client.connected()) 
  {
    Serial.print("Reconnecting to MQTT...");
    if (client.connect("ESP32_Servo_Client")) 
    {
      Serial.println("connected");
      for (int i = 0; i < 6; i++) 
      {
        client.subscribe(("actuator/" + String(ID[i]) + "/command").c_str());
      }
    } 
    else 
    {
      Serial.print("failed (rc=");
      Serial.print(client.state());
      Serial.println("), retrying...");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error)
  {
    Serial.println("JSON Parsing Failed: " + String(error.c_str()));
    return;
  }

  for (int i = 0; i < 6; i++) 
  {
    String targetTopic = "actuator/" + String(ID[i]) + "/command";
    if (String(topic) == targetTopic)
    {
      updateActuatorCommand(targetTopic, doc);
      break;
    }
  }
}

void updateActuatorCommand(const String& topic, const JsonDocument& doc) 
{
  int actuatorIndex = -1;

  for (int i = 0; i < 6; i++) 
  {
    if (topic.endsWith(String(ID[i]) + "/command")) 
    {
      actuatorIndex = i;
      break;
    }
  }

  if (actuatorIndex != -1) 
  {
    positionSync[actuatorIndex] = doc["position"] | positionSync[actuatorIndex];
    speedSync[actuatorIndex] = doc["speed"] | speedSync[actuatorIndex];
    accelerationSync[actuatorIndex] = doc["acceleration"] | accelerationSync[actuatorIndex];

    Serial.print("Updated Actuator ");
    Serial.print(ID[actuatorIndex]);
    Serial.print(" - Position: ");
    Serial.print(positionSync[actuatorIndex]);
    Serial.print(", Speed: ");
    Serial.print(speedSync[actuatorIndex]);
    Serial.print(", Acceleration: ");
    Serial.println(accelerationSync[actuatorIndex]);
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

  Serial.print("Published Feedback for Actuator ");
  Serial.println(ID[index]);
}
