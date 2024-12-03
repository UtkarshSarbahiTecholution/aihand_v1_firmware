#include <SCServo.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define S_RXD 16
#define S_TXD 17

const char* mqtt_server = "192.168.0.177";
const int mqtt_port = 1820;
const char* ssid = "AI_Hand_Network_2.4";
const char* password = "adminroot";

SMS_STS sms_sts;
WiFiClient espClient;
PubSubClient client(espClient);

byte ID[6] = {2, 5, 3, 7, 4, 6};
s16 positionSync[6] = {2300, 2400, 4000, 200, 450, 1250};
u16 speedSync[6] = {1000, 1000, 1000, 1000, 1000, 1000};
byte accelerationSync[6] = {20, 20, 20, 20, 20, 20};

Servo myservo, servo1, servo2, servo3;
int pos = 20, pos1 = 20, pos2 = 20, pos3 = 20;
int servoPin = 23, servoPin1 = 18, servoPin2 = 21, servoPin3 = 25;

void setup_wifi() {
  delay(10);
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  
  int value = msg.toInt();
  
  if (strcmp(topic, "gyro/motor1") == 0) positionSync[0] = value;
  else if (strcmp(topic, "gyro/motor2") == 0) positionSync[1] = value;
  else if (strcmp(topic, "gyro/motor3") == 0) positionSync[2] = value;
  else if (strcmp(topic, "gyro/motor4") == 0) positionSync[3] = value;
  else if (strcmp(topic, "gyro/motor5") == 0) positionSync[4] = value;
  else if (strcmp(topic, "gyro/motor6") == 0) positionSync[5] = value;
  else if (strcmp(topic, "gyro/servo1") == 0) pos = value;
  else if (strcmp(topic, "gyro/servo2") == 0) pos1 = value;
  else if (strcmp(topic, "gyro/servo3") == 0) pos2 = value;
  else if (strcmp(topic, "gyro/servo4") == 0) pos3 = value;

  sms_sts.SyncWritePosEx(ID, 6, positionSync, speedSync, accelerationSync);
  myservo.write(pos);
  servo1.write(pos1);
  servo2.write(pos2);
  servo3.write(pos3);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32_Client")) {
      Serial.println("connected");
      client.subscribe("gyro/motor1");
      client.subscribe("gyro/motor2");
      client.subscribe("gyro/motor3");
      client.subscribe("gyro/motor4");
      client.subscribe("gyro/motor5");
      client.subscribe("gyro/motor6");
      client.subscribe("gyro/servo1");
      client.subscribe("gyro/servo2");
      client.subscribe("gyro/servo3");
      client.subscribe("gyro/servo4");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(1000000, SERIAL_8N1, S_RXD, S_TXD);
  sms_sts.pSerial = &Serial1;
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  myservo.setPeriodHertz(50);
  myservo.attach(servoPin, 500, 2500);
  servo1.setPeriodHertz(50);
  servo1.attach(servoPin1, 500, 2500);
  servo2.setPeriodHertz(50);
  servo2.attach(servoPin2, 500, 2500);
  servo3.setPeriodHertz(50);
  servo3.attach(servoPin3, 500, 2500);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
