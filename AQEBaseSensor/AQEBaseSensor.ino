#include <EtherCard.h>
#include <StackPaint.h>
#include <string.h>
#include <StackPaint.h>
#include <SoftReset.h>
#include <avr/wdt.h>

#define COSM_POST_INTERVAL 30000L
long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  PRINT_STACK_SPACE;
  //--- setup ethernet shield
  Serial.println(F("\n[Air Quality Egg - Base + Sensors]"));
  sensorSetup();
  setupNanode();
  activateWithCosm();
}

void loop () {
  unsigned long currentMillis = millis();
  ether.packetLoop(ether.packetReceive());
  
  if(currentMillis - previousMillis > COSM_POST_INTERVAL) {
    postSensorData();
    PRINT_STACK_SPACE;
    previousMillis = currentMillis;   
  }
  
  checkCosmReply();
  
  if(StackCount() == 0){ // we are probably broken
    Serial.println(F("Stack Overflow Likely, restarting"));
    Serial.flush();
    delay(1000);
    soft_restart(); 
  }
}

