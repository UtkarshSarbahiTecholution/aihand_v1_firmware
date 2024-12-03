#include <WiFi.h>
#include <PubSubClient.h>

// MQTT and Wi-Fi settings
const char* mqtt_server = "192.168.0.177";
const int mqtt_port = 1820;
const char* ssid = "AI_Hand_Network_2.4";
const char* password = "adminroot";

// MUX Pin Definitions
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

WiFiClient espClient;
PubSubClient client(espClient);

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

void setup_wifi() 
{
    delay(10);
    Serial.println();
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");
}

void reconnect() 
{
    while (!client.connected()) 
    {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32Client")) 
        {
            Serial.println("connected");
        } 
        else 
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            delay(5000);
        }
    }
}

void setup() 
{
    Serial.begin(115200);

    pinMode(S0_MUX1, OUTPUT); 
    pinMode(S1_MUX1, OUTPUT); 
    pinMode(S2_MUX1, OUTPUT); 
    pinMode(S3_MUX1, OUTPUT);

    pinMode(S0_MUX2, OUTPUT); 
    pinMode(S1_MUX2, OUTPUT); 
    pinMode(S2_MUX2, OUTPUT); 
    pinMode(S3_MUX2, OUTPUT);

    pinMode(S0_MUX3, OUTPUT); 
    pinMode(S1_MUX3, OUTPUT); 
    pinMode(S2_MUX3, OUTPUT); 
    pinMode(S3_MUX3, OUTPUT);

    pinMode(COM_OUT_MUX1, INPUT);
    pinMode(COM_OUT_MUX2, INPUT);
    pinMode(COM_OUT_MUX3, INPUT);

    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
}

void loop() 
{
    if (!client.connected()) 
    {
        reconnect();
    }
    client.loop();

    for (int mux = 1; mux <= 3; mux++) 
    {
        for (int channel = 0; channel < 12; channel++) 
        {
            int fsrValue = readMux(mux, channel);
            String topic = "fsr/mux" + String(mux) + "/channel" + String(channel);
            String payload = String(fsrValue);
            
            // Publish the FSR value to the topic
            client.publish(topic.c_str(), payload.c_str());
            
            Serial.print("Published FSR data to ");
            Serial.print(topic);
            Serial.print(": ");
            Serial.println(fsrValue);
            delay(10);
        }
    }
    
}
