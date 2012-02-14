/*
>>> Air Quality Egg v01 <<<
 
 Nanode based environmental sensor ethernet connected unit. 
 Measures NO2, CO, air quality, humidity, temperature.
 
 uses ethercard (Nanode) library from 
 http://github.com/jcw/ethercard
 
 and CS_MQ7 (CO sensor) library from
 http://citizensensor.cc/make
 
 jos.ph 2012
 http://pachube.com
 http://citizensensor.cc
 
 */

#include <EtherCard.h>
#include <Wire.h>
#include <CS_MQ7.h>

#define FEED    "48091" //unique feed id
#define APIKEY  "7HsgaVRMCZ5FOSGypykT72YyKvKSAKxQbXdIanBxeEFBYz0g"

//timer vars
const int transmitFrequency = 60; //time to wait between calls in seconds
unsigned long int currTime;

// analog sensor input pins
const int No2SensorPin = A0;
const int CoSensorPin = A1;
const int qualitySensorPin = A2;
const int humiditySensorPin = A3;

// sensor values
int currCo = 0; 
int currNo2 = 0; 
int currQuality = 0;  
int currHumidity = 0; 
int currTemp = 0;
int currButton = 0;

//LEDs
const int statusLed = 5;
const int buttonLed = 10;
const int buttonPin = 7;

boolean debug = true;

void setup () {
  Serial.begin(57600);
  
  ledSetup();
  
  pinMode(buttonPin, INPUT);
  
  //sensorsSetup();
  nanodeSetup(); //nanode ethernet stup stuff
  Wire.begin(); 
}

void loop () {
  currTime = millis();
  
  nanodeUpdate();
  
  buttonUpdate(); //separate from sensors, we want to check all the time
  
  //note: transmitTime() contains sending function
  if( !transmitTime() ){   //if transmitTime returns false
    if(currTime%1000 == 0){  //print the currTime every second
      Serial.print("elapsedTime: ");  
      Serial.println(currTime/1000);

      readSensors();
      breathStatusLed(); 
    }
  } else flashStatusLed(); //we are transmitting data!
}

