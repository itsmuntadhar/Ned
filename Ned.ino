/*
This code below was written by Muntadhar Haydar (@mrmhk97).
As of March 18, 2017.
Last Modification was on March 19, 2017.

TODO:: 
Implement Auto/Manual Control modes for development, debugging and presenting.
*/

#include <DS1302.h>
#include <Sunrise.h>

const int kSclkPin = 10;  // Serial Clock
const int kIoPin   = 11;  // Input/Output
const int kCePin   = 12;  // Chip Enable

#define TIMEZONE  3         //Time zone in hours
#define LATITUDE  44.361343 //These are for Najaf, Iq. Yours can be obtained easily from any maps provider like Bing or Google.
#define LONGITUDE 32.011302

//Comment the next definition unless debugging or developing.
//#define DEBUG

unsigned long timer; //TODO:: Add something!
int datetime[6]; //An array to store the Datetime.
Sunrise mySunrise(LONGITUDE, LATITUDE, TIMEZONE); //Instance of the library.    
#ifdef DEBUG 
char buffer[80]; //We need it only to print the times for debugging.
#endif
bool isItDayTime = false; //The name says everything :3

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
  mySunrise.Actual(); //Setting calculation method. Acutal, Civil, Astro
  CalculateSunTimes(); //Call the function for the first time.
}

void loop()
{
  int threshold = 60000; //In final product, the board will check and Write every minute.
  #ifdef DEBUG
  threshold = 1000; //Under development, the board will check and Write every second.
  #endif
  if (millis() - timer >= threshold)
  {
    timer = millis(); //Keep timer up with millis().
    CalculateSunTimes(); //Do IT!
  }
}

void CalculateSunTimes()
{
    Time time = rtc.time();
    int t; //An integer used to see if there is a result from Sunrise Lib.
    byte Times[4] = { 0, 0, 0, 0 }; //0: Sunrise hour, 1: minute, 2: Sunset hour, 3: minute.
    t = mySunrise.Rise(time.mon, time.date); //Attempt to calculate sunrise time.
    if (t >= 0) //We have it :)
    {
      Times[0] = mySunrise.Hour(); //Just store the data in our array.
      Times[1] = mySunrise.Minute();
    }
    else { /*Either Pole*/ }
    t = mySunrise.Set(time.mon, time.date); //Attempt to calculate sunset time.
    if (t >= 0) //We have it :)
    {
      Times[2] = mySunrise.Hour();
      Times[3] = mySunrise.Minute();
    }
    else { /*Either Pole*/ }

    int currentMinutes = datetime[4] + (datetime[3] * 60); //converting it into minutes makes easier to compare.
    isItDayTime = currentMinutes > (Times[0] * 60) + Times[1] && currentMinutes < (Times[2] * 60) + Times[3];
    /*
    i.e. 
    if current minutes, aka time as of now, is greater than Sunrise time and lesser than Sunset time then it's Day time, and night if it is not.
    */
    WriteOutput(!isItDayTime);
    #ifdef DEBUG
    sprintf(buffer, "As of %02d/%02d, approximately, sun rises at %02d:%02d and sets at %02d:%02d\n", time.date, time.mon, Times[0], Times[1], Times[2], Times[3]); //Print the times for debugging.
    Serial.print(buffer);
    #endif
}

void WriteOutput(int level)
{
  int i = 2;
  while (i < 14)
  {
#ifdef DEBUG
    if (i == 13) break;
#endif
    digitalWrite(i, level);
    i += i == 9 ? 4 : 1;
  }
}

