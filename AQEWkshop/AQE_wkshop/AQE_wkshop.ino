
/*
>>> Air Quality Circuit v0.01 <<<
 
 Nanode based environmental sensor ethernet connected unit. 
 Measures NO2, CO, humidity, temperature.
 
 ***circuit***
 - NO2 sensor on A0
 - CO sensor OUT on A1, TOG on D4
 - Status LED on D9
 - Button on D5
 - DHT22 temp/humidity on D7
 - reset hack connects digital pin 3 to RESET pin. must be disconnected when programming.
 
 uses ethercard (Nanode) library from 
 http://github.com/jcw/ethercard
 
 and CS_MQ7 (CO) sensor and library from
 http://citizensensor.cc/make
 http://github.com/jmsaavedra/Citizen-Sensor/tree/master/sensors
 
 jos.ph 2012
 http://pachube.com
 http://citizensensor.cc
 
*/

#include <EtherCard.h> //nanode lib
#include <CS_MQ7.h> //CO lib
#include "DHT.h" //DHT temp/hum lib

#define FEED    "53801" //unique feed id
#define APIKEY  "GBVP8BTnLF2_XuoIYH79yWs9M02SAKxpd3gwdWVFa01oWT0g" //your API key

//timer vars
const int transmitFrequency = 10; //time to wait between sending data in seconds
unsigned long currTime; //holds ms passed since board started

// analog sensor input pins
const int No2SensorPin = A0;
const int CoSensorPin = A1;
const int qualitySensorPin = A2;
//const int humiditySensorPin = A3; /* old */
const int buttonPin = 5;

#define DHTPIN 7     // what pin we're connected to
DHT dht(DHTPIN, DHT22); //instantiate
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


