#include<ISADefinitions.h>
#include<ISAButtons.h>
#include<ISALiquidCrystal.h>
#include<ISALedControl.h>
ISALedControl lc;
ISAButtons buttons;
ISALiquidCrystal lcd;

const int trigger = USOUND_PING;
const int echo = USOUND_ECHO;

const int benbl = 2;
const int bphase = 3;
const int aenbl = 4;
const int aphase = 5;
const int mode = 6;

void silnik()
{
  static int v = 10;
  static int high = HIGH;
  digitalWrite(bphase, high);
  analogWrite(benbl, 128);
  delay(50);
  if (v < 255){
    v+=5;
  } else {
    v = 10;
    high = !high;
    delay(200);
  }
}

void setup() {
  pinMode(mode, OUTPUT);
  digitalWrite(mode, HIGH);
  pinMode(benbl, OUTPUT);
  pinMode(bphase, OUTPUT);
  pinMode(aenbl, OUTPUT);
  pinMode(aphase, OUTPUT);
  
 /*buttons.init();
 lc.init();
 for (int i = 0; i < 8; i++)
 {
  pinMode(LEDS[i], OUTPUT);
  //analogWrite(LEDS[i], random(0, 30));
 }*/
 /*lcd.begin();

  pinMode(USOUND_PING, OUTPUT);
  pinMode(USOUND_ECHO, INPUT);

  pinMode(BUZZER, OUTPUT);*/
}

bool wieksze(int a, int b)
{
  return a > b;
}

bool mniejsze(int a, int b)
{
  return a < b;
}


void bubbleSort(int tab[8], bool(*func)(int, int)){
  bool swapped;
  int n = sizeof(tab[8]);
  
  do {
    swapped = false;
    for(int i = 0; i < n -1; i++) 
    {
      if (func(tab[i], tab[i+1]))
      {
        int temp =  tab[i+1];
        tab[i+1] = tab[i];
        tab[i] = temp;
        analogWrite(LEDS[i], tab[i]);
        analogWrite(LEDS[i + 1], tab[i + 1]);
        delay(500);
      }
    }
  } while(swapped);
}

void za142()
{
  bubbleSort((int*)LEDS, wieksze);
  while(1);  
}

void zad144KnightRider()
{
  for (int i = 0; i < 8; i++)
  {
    int minJasnosc = 32 / 8;
    analogWrite(LEDS[i], minJasnosc*i);
  }
  while(1)
  {
    bubbleSort((int*)LEDS, wieksze);
    bubbleSort((int*)LEDS, mniejsze);
  }
}

void zgas(){
  for (int i = 0; i < 8; i++){
    digitalWrite(LEDS[i], LOW);
  }
}

void zapal(){
  for (int i = 0; i < 8; i++){
    digitalWrite(LEDS[i], HIGH);
  }
}

void mrugaj(int okres)
{
  zapal();
  delay(okres/2);
  zgas();
  delay(okres/2);
}

void zad172WyswietlMenu(int pos)
{
  String menu[] = { "zgaszone", "okres 0,5s", "okres 1s", "wlaczone" };
  lcd.clear(); 
   if (pos < 2){
    lcd.setCursor(2,0);
    lcd.print(menu[0]);
    lcd.setCursor(2,1);
    lcd.print(menu[1]);
  } 
  else 
  {
    lcd.setCursor(2,0);
    lcd.print(menu[2]);
    lcd.setCursor(2,1);
    lcd.print(menu[3]);
  }
  if (pos == 1 || pos == 3)
  {
    lcd.setCursor(0,1);
  } 
  else if (pos == 0 || pos == 2)
  {
    lcd.setCursor(0,0);
  }
  lcd.print("->");
}

void zad172()
{
  static int opcja = 0;
  static int pos = 0;
  
  switch(opcja)
  {
    case 0: zgas(); break;
    case 1: zgas(); delay(500); zapal(); delay(500); break;
    case 2: zgas(); delay(1000); zapal(); delay(1000); break;
    case 3: zapal(); break;
  }
  bool change = false;
  if (buttons.buttonState(0)){
    if (pos != 3)
    {
      pos = pos + 1;
    } 
    change = true;
    delay(100);
  } else if (buttons.buttonState(1)){
    if (pos != 0)
    {
      pos = pos - 1;
    }
    change = true;
    delay(100);
  } else if (buttons.buttonState(2)){
    opcja = pos;
  } 

  if (change)
  {
    zad172WyswietlMenu(pos); 
  }
}

void zad181()
{
  int val = analogRead(POT);
  analogWrite(LEDS[0], val/4);
}

void zad192()
{
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++) 
    {
      if ((i+j)%2)
        lc.setLed(i,j,255);
    }
  }
}

void zad194()
{
  static int col = rand() % 8;
  static int row = 0;

  lc.setLed(row, col, 255);
  delay(100);
  lc.setLed(row, col, 0);
  if (row != 8){
    row++;
  } else {
    row = 0;
    col = rand() % 8;
  }
}

int led8x8[8][8];

void zad196()
{
  static int col = rand() % 8;
  static int row = 0;

  lc.setLed(row, col, 255);
  delay(100);
  if (row != 8 && led8x8[row + 1][col] == 0 && row != 7){
    lc.setLed(row, col, 0);
    row++;
  } else {
    led8x8[row][col] = 1;
    row = 0;
    col = rand() % 8;
  }
}

void zad193()
{
  static int dirVertical = 1; 
  static int dirHorizontal = 0;
  static int deepX = 7;
  static int deepY = 7;

  static int curX = 0;
  static int curY = 0;

  lc.setLed(curX, curY, 255);
 
  if ( (deepX == curX && dirHorizontal == 1) || (((7 - deepX) == curX) && (dirHorizontal == 1))) {
    dirHorizontal = 0;
    if (deepX == curX){
      dirVertical = -1;
    } else {
      dirVertical = 1;
    }
    deepX--;
  } else if ( (deepY == curY && dirVertical == 1) || (((7 - deepY) == curY) && (dirVertical == -1)) ){
    dirVertical = 0;
    if (deepY == curY){
      dirHorizontal = 1;
    } else {
      dirHorizontal = -1;
    }
    deepY--;
  }
  
  curX += dirHorizontal;
  curY += dirVertical;
  
  delay(100);
}

float dystansCM()
{
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  int pomiar = pulseIn(echo, HIGH);
  return pomiar / 58.0f;
}

void zad21()
{
  float dist = dystansCM();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(dist);
  delay(200);
}

void zad22()
{
  float dist = dystansCM();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(dist);
  

  int buzzerFrequency = ( dist * 10 ) / 100;

 
    digitalWrite(BUZZER, HIGH);
    delay(buzzerFrequency);
    digitalWrite(BUZZER, LOW);

    delay(buzzerFrequency);
}




void loop() {
  silnik();
}
