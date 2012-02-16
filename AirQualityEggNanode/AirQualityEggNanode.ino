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
                        //unit01: 48091 // unit02: 48306 // unit03: 48307 // unit04: 48308 // unit05: 48309 // unit06: 48310 //
#define APIKEY  "7HsgaVRMCZ5FOSGypykT72YyKvKSAKxQbXdIanBxeEFBYz0g"

//timer vars
const int transmitFrequency = 15; //time to wait between sending data in seconds
unsigned long currTime; //holds ms passed since board started

// analog sensor input pins
const int No2SensorPin = A0;
const int CoSensorPin = A1;
const int qualitySensorPin = A2;
const int humiditySensorPin = A3;

// sensor values
int currCo = 0;      int currNo2 = 0; 
int currQuality = 0; int currHumidity = 0; 
int currTemp = 0;    int currButton = 0;

//LEDs
const int statusLed = 9;
const int buttonLed = 5;
const int buttonPin = 7;

boolean debug = true;
const int resetterPin = 3; //when pulled low, will reset board.

void setup () {
  digitalWrite(resetterPin, HIGH); //this is a hack!
  
  pinMode(resetterPin, OUTPUT);  
  pinMode(buttonPin, INPUT);
  
  Serial.begin(9600); 

  ledSetup();
  nanodeSetup(); //nanode ethernet stup stuff
  Wire.begin(); 
}

void loop () {
  currTime = millis();
  
  nanodeUpdate(); //checking for received data
  
  buttonUpdate(); //separate from sensors, we want to check it all the time
  
  ledUpdate();
  
  //note: transmitTime() contains sending function 
  if( !transmitTime() ){   //if we are not transmitting
    if(currTime%2000 == 0){  //print the currTime every second
      Serial.print("currTime: ");  
      Serial.println(currTime/1000);
      readSensors(); //update sensor values every second
    }
  } //else we are transmitting
}

