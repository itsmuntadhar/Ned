/*
This code below was written by Muntadhar Haydar (@mrmhk97).
As of March 18, 2017.
March 19, 2017:
-Changing RTC Library.
March 20, 2017:
-Fixing a bug caused by changing the RTC Lib.
-Changing Time calculation from minutes to military.
-Implementing Manual Mode in DEBUG mode.
March 23, 2017:
-Fixing a bug regarding checking "isAutoMode".
-Improving code.
*/

#include "DS1302.h"
#include "Sunrise.h"

const int kSclkPin = 10;  // Serial Clock
const int kIoPin   = 11;  // Input/Output
const int kCePin   = 12;  // Chip Enable

#define TIMEZONE  3         //Time zone in hours

#define LATITUDE  44.361343 //These are for Najaf, Iq. Yours can be obtained easily from any maps provider like Bing or Google.
#define LONGITUDE 32.011302

//Comment the next definition unless debugging or developing.
#define DEBUG

unsigned long timer; //TODO:: Add something!
int datetime[6]; //An array to store the Datetime.
Sunrise mySunrise(LONGITUDE, LATITUDE, TIMEZONE); //Instance of the library.    
#ifdef DEBUG 
char buffer[80]; //We need it only to print the output for debugging.
#endif
bool isItDayTime = false, isItAutoMode = true; //The name says everything :3

DS1302 rtc(kCePin, kIoPin, kSclkPin); //RTC Instance.

void setup()
{ 
  for (int i = 2; i < 10; i++) pinMode(i, OUTPUT);
  /*
    These two lines were orignally used, but changed to make the project working on other MCUs easily.
    DDRD |= B11111100; //Setting pins from 2 to 7 as output to connect relays or whatever.
    DDRB |= B00100011; //Setting pins 8, 9 & 13 as output.
  */
  #ifdef DEBUG
  pinMode(13, INPUT_PULLUP);
  Serial.begin(9600); //Serial is only needed for debugging.
  #else
  pinMode(13, OUTPUT);
  #endif
  timer = 0; //Init.
  mySunrise.Actual(); //Setting calculation method. Acutal, Civil, Astro.
  CalculateSunTimes(); //Call the function for the first rtcTime
}

void loop()
{
  int threshold = 60000; //In final product, the board will check and write every minute.
  #ifdef DEBUG
  isItAutoMode = digitalRead(13) == HIGH;
  threshold = 1000; //Under development, the board will check and write every second.
  #endif
  if (millis() - timer >= threshold)
  {
    timer = millis(); //Keep timer up with millis().
    if (isItAutoMode) CalculateSunTimes(); //Do IT!
    #ifdef DEBUG
    else MeasureTime();
    #endif
  }
}

void CalculateSunTimes()
{
    Time rtcTime = rtc.time();
    int t; //An integer used to see if there is a result from Sunrise Lib.
    byte Times[4] = { 0, 0, 0, 0 }; //0: Sunrise hour, 1: minute, 2: Sunset hour, 3: minute.
    t = mySunrise.Rise(rtcTime.mon, rtcTime.date); //Attempt to calculate sunrise time.
    if (t >= 0) //We have it :)
    {
      Times[0] = mySunrise.Hour(); //Just store the data in our array.
      Times[1] = mySunrise.Minute();
    }
    else { /*Either Pole*/ }
    t = mySunrise.Set(rtcTime.mon, rtcTime.date); //Attempt to calculate sunset time.
    if (t >= 0) //We have it :)
    {
      Times[2] = mySunrise.Hour();
      Times[3] = mySunrise.Minute();
    }
    else { /*Either Pole*/ }

    int currentClock = rtcTime.min + (rtcTime.hr * 100); //converting it into "Militray" way of timing makes easier to compare.
    isItDayTime = currentClock > (Times[0] * 100) + Times[1] && currentClock < (Times[2] * 100) + Times[3];
    /*
    i.e. 
    if currentClock, aka time as of now, is greater than Sunrise time AND lesser than Sunset time then it's Day time, and night if it is not.
    */
    WriteOutput(!isItDayTime); //Make changes if needed.
    #ifdef DEBUG
    sprintf(buffer, "Datetime: %02d/%02d/%d %02d:%02d:%02d\n", rtcTime.date, rtcTime.mon, rtcTime.yr, rtcTime.hr, rtcTime.min, rtcTime.sec); //Print the datetime for debugging.
    Serial.print(buffer);
    sprintf(buffer, "Datetime minutes: %d, Sunrise minutes: %d, Sunset minutes %d\n", currentClock, (Times[0] * 100) + Times[1], (Times[2] * 100) + Times[3]); //Print the times for debugging.
    Serial.print(buffer);
    sprintf(buffer, "As of %02d/%02d, approximately, sun rises at %02d:%02d and sets at %02d:%02d\n", rtcTime.date, rtcTime.mon, Times[0], Times[1], Times[2], Times[3]); //Print the times for debugging.
    Serial.print(buffer);
    #endif
}

#ifdef DEBUG
void MeasureTime()
{
  int reading = analogRead(5);
  Serial.println("Manual Mode");
  sprintf(buffer, "Reading from pot: %04d\n", reading);
  Serial.print(buffer);
  isItDayTime = analogRead(5) > 511;
  WriteOutput(!isItDayTime); //Make changes if needed.
}
#endif

void WriteOutput(int level)
{
  int i = 2; //First pin to start with.
  while (i < 14)
  {
#ifdef DEBUG
    if (i == 13) break; //If we are debugging we expect pin 13 as an output.
#endif
    digitalWrite(i, level);
    i += i == 9 ? 4 : 1; //Skip the RTC Pins (10, 11, 12).
  }
}
