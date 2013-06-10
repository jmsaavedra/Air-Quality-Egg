// Example to make AQE Remote or AQE Shield + NanodeRF transmit readings in standard OpenEnergyMonitor RF packet format
// This sketch can be used with either the NanodeRF_multinode example or the standard RFM12PI + Raspberrypi setup to send
// readings to emoncms.org or another emoncms installation local or remote.
// See: openenergymonitor.org
// Emoncms: emoncms.org
// Adaptation of PollEggBus_EPA for use with openenergymonitor by Trystan Lea

#include <stdint.h>
#include <DHT.h>                                                        // https://github.com/adafruit/DHT-sensor-library.git rename folder to DHT
#include "Wire.h"
#include "EggBus.h"                                                     // https://github.com/jmsaavedra/Air-Quality-Egg.git

#define freq RF12_868MHZ                                                // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 28;                                                  // emonTx RFM12B node ID
const int networkGroup = 1;                                             // emonTx RFM12B wireless network group - needs to be same as emonBase and emonGLCD needs to be same as emonBase and emonGLCD

#include <JeeLib.h>                                                     // Download JeeLib: http://github.com/jcw/jeelib
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

typedef struct { int no2, co, humidity, temperature; } PayloadAQE;      // neat way of packaging data for RF comms
PayloadAQE aqeshield;

EggBus eggBus;

#define DHTPIN 17 //analog pin 3
#define DHTTYPE DHT22  
DHT dht(DHTPIN, DHTTYPE);

float no2,co,humidity,temperature;

void setup(){
  Serial.begin(9600);
  Serial.println(F("Air Quality Egg - OpenEnergyMonitor Sensor Node configuration"));
  
  rf12_initialize(nodeID, freq, networkGroup);
}

void loop(){
  uint8_t   egg_bus_address;
  float i_scaler = 0.0;
  uint32_t r0 = 0;
  uint32_t measured_value = 0;
  
  eggBus.init();
    
  while((egg_bus_address = eggBus.next())){
    uint8_t numSensors = eggBus.getNumSensors();
    for(uint8_t ii = 0; ii < numSensors; ii++){
          
      char type = eggBus.getSensorType(ii)[0];
      if (type=='N') aqeshield.no2 = eggBus.getSensorValue(ii)*10000;  // Multiply by 10000 here and divide by 10000 in emoncms to send 4 decimal places as integer
      if (type=='C') aqeshield.co = eggBus.getSensorValue(ii)*10000;  // Multiply by 10000 here and divide by 10000 in emoncms to send 4 decimal places as integer
    }
  }
  
  aqeshield.humidity = getHumidity()*100;  // Multiply by 100 here and divide by 100 in emoncms to send 2 decimal places as integer
  aqeshield.temperature = getTemperature()*100;  // Multiply by 100 here and divide by 100 in emoncms to send 2 decimal places as integer
  
  // Uncomment to print via serial
  // Serial.println(aqeshield.no2,8);
  // Serial.println(aqeshield.co,8);
  // Serial.println(aqeshield.humidity, 8);
  // Serial.println(aqeshield.temperature, 8);

  // if ready to send + exit loop if it gets stuck as it seems too
  int i = 0; while (!rf12_canSend() && i<10) {rf12_recvDone(); i++;}
  rf12_sendStart(0, &aqeshield, sizeof aqeshield);
  // set the sync mode to 2 if the fuses are still the Arduino default
  // mode 3 (full powerdown) can only be used with 258 CK startup fuses
  rf12_sendWait(2);

  // send value readings every 60s
  delay(60000);

  Serial.println();
}

//--------- DHT22 humidity ---------
float getHumidity(){
  float h = dht.readHumidity();
  if (isnan(h)){
    //failed to get reading from DHT    
    delay(2500);
    h = dht.readHumidity();
    if(isnan(h)){
      return -1; 
    }
  } 
  else return h;
}

//--------- DHT22 temperature ---------
float getTemperature(){
  float t = dht.readTemperature();
  if (isnan(t)){
    //failed to get reading from DHT
    delay(2500);
    t = dht.readTemperature();
    if(isnan(t)){
      return -1; 
    }
  } 
  return t;
}
