/*
>>> Air Quality Egg v01 <<<
 
 Nanode based environmental sensor ethernet connected unit. 
 Measures NO2, CO, air quality, humidity, temperature.
 
 jos.ph 2012
 http://pachube.com
 http://citizensensor.cc
 */

// pachube feed info!
#define FEED    "48091"
#define APIKEY  "7HsgaVRMCZ5FOSGypykT72YyKvKSAKxQbXdIanBxeEFBYz0g"

// LED and button pins
const int buttonLed = 7;
const int buttonPin  = 2;
const int LedRunning = 10;

// analog sensor input pins
const int No2SensorPin = A0;
const int CoSensorPin = A1;
const int qualitySensorPin = A2;
const int humiditySensorPin = A3;

// sensor values
int currNo2 = 0; 
int currButton = 1;
int currCo = 2; 
int currQuality = 3;  
int currHumidity = 4; 
int currTemp = 5; 

// timer vars
long unsigned int currTime = 0;
long unsigned int prevTime = 0;
long unsigned int timeStamp = 5;
// update interval in seconds
const int updateInterval = 15;

// set true for verbose output to serial monitor.
boolean debug = true;

//-----------------NANODE STUFF------------
#include <EtherCard.h>

//----- vars
char website[] PROGMEM = "api.pachube.com";

byte Ethernet::buffer[700];
//uint32_t timer;
Stash stash;

// ethernet interface mac address, must be unique on the LAN
//NOTE: Auto MAC detection code needs to be implemented here
byte mymac[] = { 
  0x74,0x69,0x69,0x2D,0x30,0x31 };
//----------------END NANODE STUFF ---------

void setup(){
  Serial.begin(57600);
  //sensorsSetup();
  nanodeSetup();

  //setupLeds();

  //pinMode(buttonPin, INPUT);
  //delay(15);
  //int _button = digitalRead(buttonPin);
}

void loop(){

  nanodeUpdate();
  // updateSensors();

  currTime = int(millis()/1000);

  if (currTime > prevTime){ //every second
    prevTime = currTime;
    Serial.println(currTime); //print time elapsed
    //updateSensors();   //update sensor readings!
    //breatheRunningLed();
  }

  if (currTime > timeStamp + updateInterval){ //time send data interval
    timeStamp = currTime;
    if (debug) Serial.println(">>> Attempting Send <<<");
    //send all data to Pachube feed!
    nanodeSend(currNo2, currCo, currQuality, currHumidity, currTemp, currButton);
  }
}




