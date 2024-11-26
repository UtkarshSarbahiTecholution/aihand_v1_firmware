#include <ESP32Servo.h>

Servo servo1;
Servo servo2;
Servo servo3;

int pos1 = 0;
int pos2 = 0;
int pos3 = 0;
int servoPin1 = 18; 
int servoPin2 = 21;
int servoPin3 = 25;
int feedbackPin1 = 12;
int feedbackPin2 = 13;
int feedbackPin3 = 14;
int adcValue1;
int adcValue2;
int adcValue3;

float position_mm1;
float position_mm2;
float position_mm3;

void printValue(int feedbackPin, float &position_mm);

void setup()
{
  Serial.begin(115200);
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  servo1.setPeriodHertz(50);
  servo1.attach(servoPin1, 500, 2500);
  servo2.setPeriodHertz(50);
  servo2.attach(servoPin2, 500, 2500);
  servo3.setPeriodHertz(50);
  servo3.attach(servoPin3, 500, 2500);
}

void loop()
{
  for (pos1 = 0, pos2 = 0; pos1 <= 180 && pos2 <= 180; pos1 += 1, pos2 += 1)
  {
    servo1.write(pos1);
    servo2.write(pos2);
    delay(15);
  }
  delay(6000);
  printValue(feedbackPin1, position_mm1);
  printValue(feedbackPin2, position_mm2);

  for (pos3 = 0; pos3 <= 180; pos3 += 1)
  {
    servo3.write(pos3);
    delay(15);
  }
  delay(6000);
  printValue(feedbackPin3, position_mm3);

  for (pos3 = 180; pos3 >= 0; pos3 -= 1)
  {
    servo3.write(pos3);
    delay(15);
  }
  delay(6000);
  printValue(feedbackPin3, position_mm3);

  for (pos1 = 180, pos2 = 180; pos1 >= 0 && pos2 >= 0; pos1 -= 1, pos2 -= 1)
  {
    servo1.write(pos1);
    servo2.write(pos2);
    delay(15);
  }
  delay(6000);
  printValue(feedbackPin1, position_mm1);
  printValue(feedbackPin2, position_mm2);
}

void printValue(int feedbackPin, float &position_mm)
{
    int adcValue = analogRead(feedbackPin);
    float voltage = adcValue * (3.3 / 4095.0);
    position_mm = (voltage / 3.3) * 30.0;
    position_mm = 30.0 - position_mm;
    Serial.print("Position (mm): ");
    Serial.print(position_mm);
    Serial.println(" mm");
}

