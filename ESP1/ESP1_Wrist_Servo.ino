#include <ESP32Servo.h>

Servo myservo;

int pos = 0;
int servoPin = 23;

void setup()
{
  Serial.begin(115200);
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	myservo.setPeriodHertz(50);
	myservo.attach(servoPin, 500, 2500);
}

void loop()
{
  for (pos = 180; pos >= 100; pos -= 5)
  {
    myservo.write(pos);
    delay(200);
  }
  delay(1000);
}
