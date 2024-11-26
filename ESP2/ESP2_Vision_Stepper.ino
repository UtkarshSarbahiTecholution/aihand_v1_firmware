#define DIR_PIN_3 25
#define STEP_PIN_3 23
#define STEPS_PER_REVOLUTION 80

void setup() 
{
  pinMode(DIR_PIN_3, OUTPUT);
  pinMode(STEP_PIN_3, OUTPUT);
}

void loop() 
{
  digitalWrite(DIR_PIN_3, LOW);
  moveStepper(STEPS_PER_REVOLUTION);
  delay(1000);

  digitalWrite(DIR_PIN_3, HIGH);
  moveStepper(STEPS_PER_REVOLUTION);
  delay(1000);
}

void moveStepper(int steps) 
{
  for (int i = 0; i < steps; i++) 
  {
    digitalWrite(STEP_PIN_3, HIGH);
    delayMicroseconds(2000); 
    digitalWrite(STEP_PIN_3, LOW);
    delayMicroseconds(2000);
 }
}