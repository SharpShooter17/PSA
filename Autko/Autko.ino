#include <Wire.h> 
#include "Definitions.h"

#define addr 0x1E
const unsigned int number_of_mesures = 5;

const unsigned int frontSensorTrigger = 8;
const unsigned int frontSensorEcho = 9;
const unsigned int backSensorTrigger = 10;
const unsigned int backSensorEcho = 11;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.beginTransmission(addr); //start talking
  Wire.write(0x02); // Set the Register
  Wire.write(0x00); // Tell the HMC5883 to Continuously Measure
  Wire.endTransmission();

  pinMode(A_ENABLE, OUTPUT);  //ustawienie pinów do PWM na output
  pinMode(B_ENABLE, OUTPUT);

  pinMode(A_PHASE, OUTPUT);   //ustawienie pinów do kierunku jazdy na output
  pinMode(B_PHASE, OUTPUT);

  pinMode(MODE, OUTPUT);      //ustawienie pinu rozdzaju sterowania na output
  digitalWrite(MODE, LOW);

  pinMode(frontSensorTrigger, OUTPUT);
  pinMode(frontSensorEcho, INPUT);

  pinMode(backSensorTrigger, OUTPUT);
  pinMode(backSensorEcho, INPUT);

  Serial.begin(9600);
}

float measure( int trigger, int echo )
{
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);

  float result = pulseIn(echo, HIGH) / 58.0f;

  if (result < 5.0f) 
  {
    return measure(trigger, echo);
  }

  return result > 200.0 ? 200.0 : result;
}

volatile float distFront = 0.0f;
volatile float distBack = 0.0f;

volatile float frontMeasures[number_of_mesures];
volatile float backMeasures[number_of_mesures];
volatile int mesure_index = 0;

float minimum(int count, volatile float measures[])
{
  if (count < 2)
  {
    return min( measures[0], measures[1]);
  }
  
  float result = measures[0];
  for (int i = 1; i < count; i++)
  {
    result = min(result, measures[i]);
  }
  return result;
}


float remove_min_and_count_average(float minimum, int count, volatile float measures[])
{
  float sum = 0.0;
  for (int i = 0; i < count; i++)
  {
    sum += measures[i];
  }
  sum -= minimum;

  return sum / (float)(count - 1.0);
}

void average()
{
  frontMeasures[mesure_index] = measure(frontSensorTrigger, frontSensorEcho);
  backMeasures[mesure_index] = measure(backSensorTrigger, backSensorEcho);
  ++mesure_index;
  if ( mesure_index == number_of_mesures )
  {
    mesure_index = 0;
  }
  float minFront = minimum(number_of_mesures, frontMeasures);
  float minBack = minimum(number_of_mesures, backMeasures);
  
  distFront = remove_min_and_count_average(minFront, number_of_mesures, frontMeasures);
  distBack = remove_min_and_count_average(minBack, number_of_mesures, backMeasures);
}

void setLeftSpeed( short speed )
{
  if (speed < 0)
  {
    digitalWrite(B_PHASE, LOW);
  }
  else 
  {
    digitalWrite(B_PHASE, HIGH);
  }
  
  analogWrite(B_ENABLE, speed < 0 ? -speed : speed);
}

void setRightSpeed( short speed )
{
  if (speed < 0)
  {
    digitalWrite(A_PHASE, LOW);
  }
  else 
  {
    digitalWrite(A_PHASE, HIGH);
  }
  analogWrite(A_ENABLE, speed < 0 ? -speed : speed);
}

void setSpeed(short speed)
{
  setLeftSpeed(speed);
  setRightSpeed(speed);
}

volatile static bool straight = true; 

void loop() 
{
  average();

  int x,y,z;

  Wire.beginTransmission(addr);
  Wire.write(0x03);
  Wire.endTransmission();
  Wire.requestFrom(addr, 6);
  if(6<=Wire.available()){
   x = Wire.read()<<8; //MSB  x 
   x |= Wire.read(); //LSB  x
   z = Wire.read()<<8; //MSB  z
   z |= Wire.read(); //LSB z
   y = Wire.read()<<8; //MSB y
   y |= Wire.read(); //LSB y
  }
  // Show Values
  Serial.print("X Value: ");
  Serial.println(x);
  Serial.print("Y Value: ");
  Serial.println(y);
  Serial.print("Z Value: ");
  Serial.println(z);
  Serial.println();
  delay(500);
}



