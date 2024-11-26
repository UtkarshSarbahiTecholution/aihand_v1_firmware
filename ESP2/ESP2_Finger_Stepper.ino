#define DIR_PIN_1 19
#define STEP_PIN_1 18
#define DIR_PIN_2 22
#define STEP_PIN_2 21
#define STEPS_PER_REVOLUTION 1400

void setup()
{
  pinMode(DIR_PIN_1, OUTPUT);
  pinMode(STEP_PIN_1, OUTPUT);
  pinMode(DIR_PIN_2, OUTPUT);
  pinMode(STEP_PIN_2, OUTPUT);
}

void loop() 
{
  digitalWrite(DIR_PIN_1, LOW);
  digitalWrite(DIR_PIN_2, LOW);
  moveStepper(STEPS_PER_REVOLUTION);
  delay(2000);

  digitalWrite(DIR_PIN_1, HIGH);
  digitalWrite(DIR_PIN_2, HIGH);
  moveStepper(STEPS_PER_REVOLUTION);
  delay(2000);
}

void moveStepper(int steps) 
{
  for (int i = 0; i < steps; i++) 
  {
    digitalWrite(STEP_PIN_1, HIGH);
    digitalWrite(STEP_PIN_2, HIGH);
    delayMicroseconds(1000);
    digitalWrite(STEP_PIN_1, LOW);
    digitalWrite(STEP_PIN_2, LOW);
    delayMicroseconds(1000);
  }
}
