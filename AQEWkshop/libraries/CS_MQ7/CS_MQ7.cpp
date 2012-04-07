
/*  
	CS_MQ7_02.h - Library for reading the MQ-7 Carbon Monoxide Sensor
	Breakout, as part of the Citizen Sensor project.	
	http://citizensensor.cc
	
	Released into the public domain.
	
	Created by J Saavedra, October 2010.
	http://jos.ph

*/

#if (ARDUINO >= 100)
    #include "Arduino.h"
#else
    #include <avr/io.h>
    #include "WProgram.h"
#endif

#include "CS_MQ7.h"

CS_MQ7::CS_MQ7(int CoTogPin, int CoIndicatorPin){

	pinMode(CoIndicatorPin, OUTPUT);
	pinMode(CoTogPin, OUTPUT);
	
	_CoIndicatorPin = CoIndicatorPin;
	_CoTogPin = CoTogPin;
    
    indicatorAttached = true; //we are using an LED to show heater
	
	time = 0;
	currTime = 0;
	prevTime = 0;
	currCoPwrTimer = 0;
	CoPwrState = LOW;
  	currCoPwrTimer = 500;
	
}

CS_MQ7::CS_MQ7(int CoTogPin){
    
	pinMode(CoTogPin, OUTPUT);
	
    indicatorAttached = false; //not using an LED
    
	_CoTogPin = CoTogPin;
	
	time = 0;
	currTime = 0;
	prevTime = 0;
	currCoPwrTimer = 0;
	CoPwrState = LOW;
  	currCoPwrTimer = 500;
    
}

void CS_MQ7::CoPwrCycler(){
  
  currTime = millis();
   
  if (currTime - prevTime > currCoPwrTimer){
    prevTime = currTime;
    
    if(CoPwrState == LOW){
      CoPwrState = HIGH;
      currCoPwrTimer = 60000;  //60 seconds at 5v
    }
    else{
      CoPwrState = LOW;
      currCoPwrTimer = 90000;  //90 seconds at 1.4v
    }
    if(indicatorAttached) digitalWrite(_CoIndicatorPin, CoPwrState);
    digitalWrite(_CoTogPin, CoPwrState);
  }
}

boolean CS_MQ7::currentState(){
	
	if(CoPwrState == LOW){
		return false;
	}
	else{
		return true;
	}
}