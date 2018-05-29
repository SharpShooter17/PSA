#include "ISAMobile.h"

QMC5883 qmc;

void SetPowerLevel(PowerSideEnum side, int level)
{
  level = constrain(level, -255, 255);
  
  if (side == PowerSideEnum::Right) {
    if (level > 0) {
      // do przodu
      digitalWrite(A_PHASE, 1);
      analogWrite(A_ENABLE, level);
    } else if (level < 0) {
      // do tyłu
      digitalWrite(A_PHASE, 0);
      analogWrite(A_ENABLE, -level);
    } else {
      // stop
      digitalWrite(A_PHASE, 0);
      analogWrite(A_ENABLE, 0);
    }
  }
  
  if (side == PowerSideEnum::Left) {
    if (level > 0) {
      // do przodu
      digitalWrite(B_PHASE, 1);
      analogWrite(B_ENABLE, level);
    } else if (level < 0) {
      // do tyłu
      digitalWrite(B_PHASE, 0);
      analogWrite(B_ENABLE, -level);
    } else {
      // stop
      digitalWrite(B_PHASE, 0);
      analogWrite(B_ENABLE, 0);
    }
  } 
}

volatile unsigned int timer = 0;
volatile unsigned int last = 0;

void setup() {
  // Czujniki ultradźwiekowe
  for (int i = (int)UltraSoundSensor::__first; i <= (int)UltraSoundSensor::__last; i++)
  {
    pinMode(ultrasound_trigger_pin[i], OUTPUT);
    pinMode(ultrasound_echo_pin[i], INPUT);
    
    digitalWrite(ultrasound_trigger_pin[i], 0);
  }
  
  pinMode(A_PHASE, OUTPUT);
  pinMode(A_ENABLE, OUTPUT);
  pinMode(B_PHASE, OUTPUT);
  pinMode(B_ENABLE, OUTPUT);
  pinMode(MODE, OUTPUT);

  digitalWrite(MODE, true);
  SetPowerLevel(PowerSideEnum::Left, 0);
  SetPowerLevel(PowerSideEnum::Right, 0);

  Wire.begin();
  qmc.init();

  Serial.begin(9600);
  Serial1.begin(9600); // HC06

  last = millis();
}

int measureSoundSpeed(int trigger_pin, int echo_pin)
{
  digitalWrite(trigger_pin, false);
  delayMicroseconds(2);

  digitalWrite(trigger_pin, true);
  delayMicroseconds(10);
  digitalWrite(trigger_pin, false);

  // zmierz czas przelotu fali dźwiękowej
  int duration = pulseIn(echo_pin, true, 50 * 1000);
  

  // przelicz czas na odległość (1/2 Vsound(t=20st.C))
  int distance = (int)((float)duration * 0.03438f * 0.5f);
  return distance;
}

const unsigned int sizeDir = 3;
int16_t dir[sizeDir] = {-500, 0, 450};
volatile int16_t i = 0;

void turnLeft()
{
  SetPowerLevel(PowerSideEnum::Right, 100);
  SetPowerLevel(PowerSideEnum::Left, -100);  
  int x;
  while (!(((x - 50) <= dir[i]) && (dir[i] <= (x + 50))))
  {
    qmc.measure();
    x = qmc.getX(); //x - 50 < DIR < x + 50      
  }

  if (i == sizeDir - 1){
    i = 0;
  } else {
    ++i;
  }
}

float dt = 20; //co ile pobiera się próbkę
int ep; //uchyb poprzedni
int en; //uchyb następny
int U; //sygnał sterujący
int C; //część całkująca
int Kp = 2; //wzmocnienie
int Ti = 4; //stała całkowania
int Td = 8; //stała różniczkowania

int pid(int x)
{
  en = x; //tutaj założyłem, że wartość uchybu poznajemy za pomocą odczytu z pinu A2
  C += ((ep + en)/2)*dt;
  U = Kp*(en + (1/Ti)*C/1000 + Td*(en - ep)*1000/dt);
  ep = en;
  return U;
}

volatile bool isTurnLeft = true;

const int minimumU = 0;
const int maximumU = 360;

void loop() 
{
  qmc.measure();
  int x = qmc.getX();
  int y = qmc.getY();
  float heading = atan2(y, x);
  if(heading < 0)
    heading += 2*PI;

  if(heading > 2*PI)
    heading -= 2*PI;

  float headingDegrees = heading * 180/M_PI; 

  if (headingDegrees > 180.0) 
  {
    headingDegrees = 360.0 - headingDegrees;
    isTurnLeft = true;
  } else 
  {
    isTurnLeft = false;
  }

  int p = pid(headingDegrees);

  if (p <= maximumU && p >= minimumU){
    int s = map(p, minimumU, maximumU, 0, 105);
    SetPowerLevel(PowerSideEnum::Right, isTurnLeft ? 150 + s : 150);
    SetPowerLevel(PowerSideEnum::Left, !isTurnLeft ? 150 + s : 150);
  }
  
  delay(dt);
}
