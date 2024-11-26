#include <SCServo.h>

#define S_RXD 16
#define S_TXD 17

SMS_STS sms_sts;

int position;
int speed;
int load;
int voltage;
int temperature;
int movement;
int current;

byte ID[6];
s16 positionSync[6];
u16 speedSync[6];
byte accelerationSync[6];

void getFeedback(int servoID);

void setup()
{
  Serial1.begin(1000000, SERIAL_8N1, S_RXD, S_TXD);
  Serial.begin(115200);
  sms_sts.pSerial = &Serial1;
  delay(1000);
  ID[0] = 2;
  ID[1] = 5;
  ID[2] = 3;
  ID[3] = 7;
  ID[4] = 4;
  ID[5] = 6;
  speedSync[0] = 1000;
  speedSync[1] = 1000;
  speedSync[2] = 1000;
  speedSync[3] = 1000;
  speedSync[4] = 1000;
  speedSync[5] = 1000;
  accelerationSync[0] = 20;
  accelerationSync[1] = 20;
  accelerationSync[2] = 20;
  accelerationSync[3] = 20;
  accelerationSync[4] = 20;
  accelerationSync[5] = 20;
}

void loop() 
{
  positionSync[0] = 1900;
  positionSync[1] = 2700;
  positionSync[2] = 3700;
  positionSync[3] = 300;
  positionSync[4] = 750;
  positionSync[5] = 950;
  sms_sts.SyncWritePosEx(ID, 6, positionSync, speedSync, accelerationSync);
  // getFeedback(2);
  // getFeedback(5);
  // getFeedback(3);
  // getFeedback(7);
  // getFeedback(4);
  // getFeedback(6);
  delay(2000);
  // delay(500);

  positionSync[0] = 2300;
  positionSync[1] = 2400;
  positionSync[2] = 4000;
  positionSync[3] = 200;
  positionSync[4] = 450;
  positionSync[5] = 1250;
  sms_sts.SyncWritePosEx(ID, 6, positionSync, speedSync, accelerationSync);
  // getFeedback(2);
  // getFeedback(5);
  // getFeedback(3);
  // getFeedback(7);
  // getFeedback(4);
  // getFeedback(6);
  delay(2000);
  // delay(500);

  // sms_sts.WritePosEx(2, 1900, 1500, 1500);
  // sms_sts.WritePosEx(5, 2700, 1500, 1500);
}

void getFeedback(int servoID)
{
  if (sms_sts.FeedBack(servoID) != -1)
  {
    Serial.print("#### Servo ID: ");
    Serial.print(servoID, DEC);
    Serial.println(" ####");

    position = sms_sts.ReadPos(servoID);
    speed = sms_sts.ReadSpeed(servoID);
    load = sms_sts.ReadLoad(servoID);
    voltage = sms_sts.ReadVoltage(servoID);
    temperature = sms_sts.ReadTemper(servoID);
    movement = sms_sts.ReadMove(servoID);
    current = sms_sts.ReadCurrent(servoID);

    Serial.print("Position: ");
    Serial.println(position, DEC);
    Serial.print("Speed: ");
    Serial.println(speed, DEC);
    Serial.print("Load: ");
    Serial.println(load, DEC);
    Serial.print("Voltage: ");
    Serial.println(voltage, DEC);
    Serial.print("Temperature: ");
    Serial.println(temperature, DEC);
    Serial.print("Movement: ");
    Serial.println(movement, DEC);
    Serial.print("Current: ");
    Serial.println(current, DEC);
    delay(500);

    Serial.println();
  }
}
