#include <Fuzzy.h>
#include <FuzzyComposition.h>
#include <FuzzyInput.h>
#include <FuzzyIO.h>
#include <FuzzyOutput.h>
#include <FuzzyRule.h>
#include <FuzzyRuleAntecedent.h>
#include <FuzzyRuleConsequent.h>
#include <FuzzySet.h>

#include "ISAMobile.h"

QMC5883 qmc;
Fuzzy* fuzzy = new Fuzzy();

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

void fuzzyConfig()
{
  FuzzyInput* distance = new FuzzyInput(1);

  FuzzySet* small = new FuzzySet(10, 40, 40, 60);
  distance->addFuzzySet(small);
  FuzzySet* safe = new FuzzySet(50, 80, 80, 120);
  distance->addFuzzySet(safe);
  FuzzySet* big = new FuzzySet(110, 140, 140, 180);
  distance->addFuzzySet(big);

  fuzzy->addFuzzyInput(distance);

  FuzzyOutput* velocity = new FuzzyOutput(1);

  FuzzySet* slow = new FuzzySet(10, 15, 15, 35);
  velocity->addFuzzySet(slow);
  FuzzySet* average = new FuzzySet(30, 50, 50, 70);
  velocity->addFuzzySet(average);
  FuzzySet* fast = new FuzzySet(60, 80, 80, 105);
  velocity->addFuzzySet(fast);

  fuzzy->addFuzzyOutput(velocity);

  FuzzyRuleAntecedent* ifDistanceSmall = new FuzzyRuleAntecedent();
  ifDistanceSmall->joinSingle(small);
  FuzzyRuleConsequent* thenVelocitySlow = new FuzzyRuleConsequent();
  thenVelocitySlow->addOutput(slow);

  FuzzyRule* fuzzyRule01 = new FuzzyRule(1, ifDistanceSmall, thenVelocitySlow);

  fuzzy->addFuzzyRule(fuzzyRule01);

  FuzzyRuleAntecedent* ifDistanceSafe = new FuzzyRuleAntecedent();
  ifDistanceSafe->joinSingle(safe);
  FuzzyRuleConsequent* thenVelocityAverage = new FuzzyRuleConsequent();
  thenVelocityAverage->addOutput(average);
 
  FuzzyRule* fuzzyRule02 = new FuzzyRule(2, ifDistanceSafe, thenVelocityAverage);

  fuzzy->addFuzzyRule(fuzzyRule02);

  FuzzyRuleAntecedent* ifDistanceBig = new FuzzyRuleAntecedent();
  ifDistanceBig->joinSingle(big);
  FuzzyRuleConsequent* thenVelocityFast = new FuzzyRuleConsequent();
  thenVelocityFast->addOutput(fast);
  
  FuzzyRule* fuzzyRule03 = new FuzzyRule(3, ifDistanceBig, thenVelocityFast);

  fuzzy->addFuzzyRule(fuzzyRule03); // Adding FuzzyRule to Fuzzy object
}

void setup() {
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
  fuzzyConfig();
}


volatile bool isTurnLeft = true;

const int minimumU = 90;
const int maximumU = 200;

void loop()
{
  qmc.measure();
  int x = qmc.getX();
  int y = qmc.getY();
  float heading = atan2(y, x);
  if (heading < 0)
    heading += 2 * PI;

  if (heading > 2 * PI)
    heading -= 2 * PI;

  float headingDegrees = heading * 180 / M_PI;


  if (headingDegrees > 180.0)
  {
    headingDegrees = 360.0 - headingDegrees;
    isTurnLeft = true;
  } else
  {
    isTurnLeft = false;
  }


  float dist = headingDegrees;
  
  fuzzy->setInput(1, dist);
  fuzzy->fuzzify();

  float carSpeed = fuzzy->defuzzify(1);
  //float finalCarSpeed = map(carSpeed, minimumU, maximumU, 0, 105);
  SetPowerLevel(PowerSideEnum::Right, isTurnLeft ? 150 + carSpeed : 150);
  SetPowerLevel(PowerSideEnum::Left, !isTurnLeft ? 150 + carSpeed : 150);

  if (Serial1.available()) {
    Serial1.println(carSpeed);
  }

  delay(20);
}
