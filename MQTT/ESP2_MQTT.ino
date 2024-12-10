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

#define FSR_THRESHOLD 1000

const char* ssid = "Techolution";
const char* password = "techolutionisthebe$t";
const char* mqtt_server = "192.168.1.100";
const int mqtt_port = 1820;

WiFiClient espClient;
PubSubClient client(espClient);

void setupWiFi();
void reconnect();
void publishFSRValues(int link, const char* side, JsonDocument& doc);
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

  handleFSR(1, "Left", 0, 9, 1);
  handleFSR(1, "Right", 9, 18, 1); 
  handleFSR(2, "Left", 0, 6, 2); 
  handleFSR(2, "Right", 6, 12, 2);
  handleFSR(3, "Left", 0, 3, 3);    
  handleFSR(3, "Right", 3, 6, 3);   

  delay(500);
}

void handleFSR(int link, const char* side, int startChannel, int endChannel, int mux) 
{
  StaticJsonDocument<512> doc;
  JsonArray fsrValues = doc.createNestedArray("fsr_values");

  bool sendData = false;

  for (int channel = startChannel; channel < endChannel; channel++) 
  {
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
    publishFSRValues(link, side, doc);
  }
}

void publishFSRValues(int link, const char* side, JsonDocument& doc) 
{
  doc["link"] = link;
  doc["side"] = side;

  char buffer[512];
  serializeJson(doc, buffer);

  String topic = "fsr/link" + String(link) + "/" + side;
  client.publish(topic.c_str(), buffer);

  Serial.printf("Published to %s: %s\n", topic.c_str(), buffer);
}

void selectMuxChannel(int mux, int channel) 
{
  if (mux == 1) 
  {
    digitalWrite(S0_MUX1, channel & 0x01);
    digitalWrite(S1_MUX1, (channel >> 1) & 0x01);
    digitalWrite(S2_MUX1, (channel >> 2) & 0x01);
    digitalWrite(S3_MUX1, (channel >> 3) & 0x01);
  } 
  else if (mux == 2) 
  {
    digitalWrite(S0_MUX2, channel & 0x01);
    digitalWrite(S1_MUX2, (channel >> 1) & 0x01);
    digitalWrite(S2_MUX2, (channel >> 2) & 0x01);
    digitalWrite(S3_MUX2, (channel >> 3) & 0x01);
  } 
  else if (mux == 3) 
  {
    digitalWrite(S0_MUX3, channel & 0x01);
    digitalWrite(S1_MUX3, (channel >> 1) & 0x01);
    digitalWrite(S2_MUX3, (channel >> 2) & 0x01);
    digitalWrite(S3_MUX3, (channel >> 3) & 0x01);
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
    } 
    else 
    {
      Serial.printf("Failed (rc=%d). Retrying in 5 seconds...\n", client.state());
      delay(5000);
    }
  }
}
