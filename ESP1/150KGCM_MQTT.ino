#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>

const char* ssid = "Techolution";
const char* password = "techolutionisthebe$t";
const char* mqtt_server = "192.168.1.100";
const int mqtt_port = 1820;

WiFiClient espClient;
PubSubClient client(espClient);
Servo myservo;

const int servoPin = 23;
int pos = 0;
bool positionChanged = false;

void setup_wifi();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);

void setup() 
{
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
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

  if (positionChanged) 
  {
    myservo.write(pos);
    Serial.printf("Servo moved to position: %d\n", pos);
    positionChanged = false;
  }
}

void setup_wifi() 
{
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(250);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi connected. IP Address: " + WiFi.localIP().toString());
}

void reconnect() 
{
  while (!client.connected()) 
  {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32_Servo_Client")) 
    {
      Serial.println("Connected.");
      client.subscribe("actuator/wrist/command");
      Serial.println("Subscribed to: actuator/wrist/command");
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
  StaticJsonDocument<128> doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) 
  {
    Serial.printf("JSON Parsing Failed: %s\n", error.c_str());
    return;
  }

  int newPosition = doc["position"] | -1;
  if (newPosition >= 100 && newPosition <= 180) 
  {
    pos = newPosition;
    positionChanged = true;
    Serial.printf("Received new position: %d\n", pos);
  } 
  else 
  {
    Serial.println("Invalid position. Must be between 100-180.");
  }
}
