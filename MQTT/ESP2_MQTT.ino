#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define S0_MUX1 17
#define S1_MUX1 16
#define S2_MUX1 14
#define S3_MUX1 13
#define COM_OUT_MUX1 33

#define S0_MUX2 23
#define S1_MUX2 22
#define S2_MUX2 21
#define S3_MUX2 19
#define COM_OUT_MUX2 32

#define S0_MUX3 25
#define S1_MUX3 4
#define S2_MUX3 27
#define S3_MUX3 26
#define COM_OUT_MUX3 34

#define FSR_THRESHOLD 2000

const char* ssid = "Techolution";
const char* password = "techolutionisthebe$t";
const char* mqtt_server = "192.168.1.100";
const int mqtt_port = 1820;

WiFiClient espClient;
PubSubClient client(espClient);

const int link3_left[] = { 3, 0, 3, 1, 3, 2, 3, 3, 3, 4, 3, 5, 3, 6, 3, 7, 3, 8 };
const int link2_left[] = { 2, 0, 2, 1, 2, 2, 2, 3, 2, 4, 2, 5 };
const int link1_left[] = { 3, 9, 3, 11 };

const int link3_right[] = { 1, 4, 1, 8, 1, 3, 1, 9, 1, 10, 1, 5, 1, 6, 1, 11, 1, 7 };
const int link2_right[] = { 2, 6, 2, 7, 2, 8, 2, 11, 2, 9, 2, 10 };
const int link1_right[] = { 1, 0, 1, 2 };

void setupWiFi();
void reconnect();
void handleFSR(const char* linkName, const char* side, const int* group, int size);
void publishFSRValues(const char* linkName, const char* side, JsonDocument& doc);
void selectMuxChannel(int mux, int channel);
int readMux(int mux, int channel);

void setup() 
{
  Serial.begin(115200);
  setupWiFi();
  client.setServer(mqtt_server, mqtt_port);

  pinMode(S0_MUX1, OUTPUT);
  pinMode(S1_MUX1, OUTPUT);
  pinMode(S2_MUX1, OUTPUT);
  pinMode(S3_MUX1, OUTPUT);
  pinMode(COM_OUT_MUX1, INPUT);

  pinMode(S0_MUX2, OUTPUT);
  pinMode(S1_MUX2, OUTPUT);
  pinMode(S2_MUX2, OUTPUT);
  pinMode(S3_MUX2, OUTPUT);
  pinMode(COM_OUT_MUX2, INPUT);

  pinMode(S0_MUX3, OUTPUT);
  pinMode(S1_MUX3, OUTPUT);
  pinMode(S2_MUX3, OUTPUT);
  pinMode(S3_MUX3, OUTPUT);
  pinMode(COM_OUT_MUX3, INPUT);

  Serial.println("Setup complete.");
}

void loop() 
{
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();

  handleFSR("3", "left", link3_left, sizeof(link3_left) / sizeof(link3_left[0]));
  handleFSR("2", "left", link2_left, sizeof(link2_left) / sizeof(link2_left[0]));
  handleFSR("1", "left", link1_left, sizeof(link1_left) / sizeof(link1_left[0]));

  handleFSR("3", "right", link3_right, sizeof(link3_right) / sizeof(link3_right[0]));
  handleFSR("2", "right", link2_right, sizeof(link2_right) / sizeof(link2_right[0]));
  handleFSR("1", "right", link1_right, sizeof(link1_right) / sizeof(link1_right[0]));

  delay(500);
}

void handleFSR(const char* linkName, const char* side, const int* group, int size) 
{
  StaticJsonDocument<512> doc;
  JsonArray fsrValues = doc.createNestedArray("fsr_values");

  bool sendData = false;

  for (int i = 0; i < size; i += 2) 
  {
    int mux = group[i];
    int channel = group[i + 1];
    int fsrValue = readMux(mux, channel);

    if (fsrValue > FSR_THRESHOLD) 
    {
      sendData = true;
      fsrValues.add(fsrValue);
    } 
    else 
    {
      fsrValues.add(0);
    }
  }

  if (sendData) 
  {
    publishFSRValues(linkName, side, doc);
  }
}

void publishFSRValues(const char* linkName, const char* side, JsonDocument& doc) 
{
  doc["link"] = linkName;
  doc["side"] = side;

  char buffer[512];
  serializeJson(doc, buffer);

  String topic = "fsr/link" + String(linkName) + "/" + side;
  client.publish(topic.c_str(), buffer);

  Serial.printf("Published to %s: %s\n", topic.c_str(), buffer);
}

void selectMuxChannel(int mux, int channel) 
{
  switch (mux) 
  {
    case 1:
      digitalWrite(S0_MUX1, channel & 0x01);
      digitalWrite(S1_MUX1, (channel >> 1) & 0x01);
      digitalWrite(S2_MUX1, (channel >> 2) & 0x01);
      digitalWrite(S3_MUX1, (channel >> 3) & 0x01);
      break;
    case 2:
      digitalWrite(S0_MUX2, channel & 0x01);
      digitalWrite(S1_MUX2, (channel >> 1) & 0x01);
      digitalWrite(S2_MUX2, (channel >> 2) & 0x01);
      digitalWrite(S3_MUX2, (channel >> 3) & 0x01);
      break;
    case 3:
      digitalWrite(S0_MUX3, channel & 0x01);
      digitalWrite(S1_MUX3, (channel >> 1) & 0x01);
      digitalWrite(S2_MUX3, (channel >> 2) & 0x01);
      digitalWrite(S3_MUX3, (channel >> 3) & 0x01);
      break;
  }
}

int readMux(int mux, int channel) 
{
  selectMuxChannel(mux, channel);
  delayMicroseconds(5);
  if (mux == 1)
  {
    return analogRead(COM_OUT_MUX1);
  } 
  if (mux == 2)
  {
    return analogRead(COM_OUT_MUX2);
  } 
  if (mux == 3)
  {
    return analogRead(COM_OUT_MUX3);
  } 
  return 0;
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
    if (client.connect("ESP32_FSR_Client")) 
    {
      Serial.println("Connected to MQTT broker.");
    } else 
    {
      Serial.printf("Failed (rc=%d). Retrying in 5 seconds...\n", client.state());
      delay(5000);
    }
  }
}
