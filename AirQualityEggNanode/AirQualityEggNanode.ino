
/*

*** NEED TO REPLACE TEMP/HUMIDITY WITH DHT22 ***
*** NEED TO REMOVE CS_MQ7, REPLACE WITH E2V***
*** also need to remove Button from this code ***

>>> Air Quality Egg v01 <<<
 
 Nanode based environmental sensor ethernet connected unit. 
 Measures NO2, CO, air quality, humidity, temperature.
 
 reset hack connects digital pin 3 to RESET pin. must be disconnected when programming.
 
 uses ethercard (Nanode) library from 
 http://github.com/jcw/ethercard
 
 and CS_MQ7 (CO) sensor and library from
 http://citizensensor.cc/make
 http://github.com/jmsaavedra/Citizen-Sensor/tree/master/sensors
 
 jos.ph 2012
 http://pachube.com
 http://citizensensor.cc
 
*/

#include <EtherCard.h>
#include <Wire.h>
#include <CS_MQ7.h>

#define FEED    "48091" //unique feed id -- Egg feeds are below:
//unit01: 48091 // unit02: 48306 // unit03: 48307 // unit04: 48308 // unit05: 48309 // unit06: 48310 //
#define APIKEY  "7HsgaVRMCZ5FOSGypykT72YyKvKSAKxQbXdIanBxeEFBYz0g"

//timer vars
const int transmitFrequency = 10; //time to wait between sending data in seconds
unsigned long currTime; //holds ms passed since board started

// analog sensor input pins
const int No2SensorPin = A0;
const int CoSensorPin = A1;
const int qualitySensorPin = A2;
const int humiditySensorPin = A3;
const int buttonPin = 7;

//sensor value vars
int currNo2, currCo, currQuality, currHumidity, currTemp, currButton = 0;

boolean debug = true;

//reset for when ethernet times out and never comes back
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
  } //else we are transmitting!
}


