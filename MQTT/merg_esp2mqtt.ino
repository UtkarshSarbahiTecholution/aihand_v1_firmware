#include <WiFi.h>
#include <PubSubClient.h>

// Stepper motor pins and configurations
#define DIR_PIN_1 19
#define STEP_PIN_1 18
#define DIR_PIN_2 22
#define STEP_PIN_2 21
#define DIR_PIN_3 25
#define STEP_PIN_3 23
#define STEPS_PER_REVOLUTION 1400

// WiFi and MQTT server settings
const char* ssid = "AI_Hand_Network_2.4";
const char* password = "adminroot";
const char* mqtt_server = "192.168.0.177";
const int mqtt_port = 1820;

// MQTT topics
const char* topic_dir1 = "stepper/dir1";
const char* topic_dir2 = "stepper/dir2";
const char* topic_dir3 = "stepper/dir3";
const char* topic_steps1 = "stepper/steps1";
const char* topic_steps2 = "stepper/steps2";
const char* topic_steps3 = "stepper/steps3";

// Variables to hold stepper states
int direction1 = LOW;
int direction2 = LOW;
int direction3 = LOW;
int steps1 = STEPS_PER_REVOLUTION;
int steps2 = STEPS_PER_REVOLUTION;
int steps3 = STEPS_PER_REVOLUTION;

// MQTT and WiFi clients
WiFiClient espClient;
PubSubClient client(espClient);

// Function to move stepper motors
void moveStepper(int steps, int dirPin, int stepPin) {
  digitalWrite(dirPin, (dirPin == DIR_PIN_1) ? direction1 :
                         (dirPin == DIR_PIN_2) ? direction2 : direction3);
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1000);
  }
}

// MQTT callback function
void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  int value = msg.toInt();

  if (strcmp(topic, topic_dir1) == 0) direction1 = value;
  else if (strcmp(topic, topic_dir2) == 0) direction2 = value;
  else if (strcmp(topic, topic_dir3) == 0) direction3 = value;
  else if (strcmp(topic, topic_steps1) == 0) steps1 = value;
  else if (strcmp(topic, topic_steps2) == 0) steps2 = value;
  else if (strcmp(topic, topic_steps3) == 0) steps3 = value;
}

// Reconnect to MQTT server
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32_Stepper")) {
      Serial.println("Connected");
      client.subscribe(topic_dir1);
      client.subscribe(topic_dir2);
      client.subscribe(topic_dir3);
      client.subscribe(topic_steps1);
      client.subscribe(topic_steps2);
      client.subscribe(topic_steps3);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// WiFi setup
void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void setup() {
  Serial.begin(115200);

  // Initialize stepper motor pins
  pinMode(DIR_PIN_1, OUTPUT);
  pinMode(STEP_PIN_1, OUTPUT);
  pinMode(DIR_PIN_2, OUTPUT);
  pinMode(STEP_PIN_2, OUTPUT);
  pinMode(DIR_PIN_3, OUTPUT);
  pinMode(STEP_PIN_3, OUTPUT);

  // Setup WiFi and MQTT
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Move stepper motors based on MQTT commands
  moveStepper(steps1, DIR_PIN_1, STEP_PIN_1);
  delay(1000);
  moveStepper(steps2, DIR_PIN_2, STEP_PIN_2);
  delay(1000);
  moveStepper(steps3, DIR_PIN_3, STEP_PIN_3);
  delay(1000);
}
