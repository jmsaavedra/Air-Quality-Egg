#include <stdint.h>

#include "Wire.h"
#include "EggBus.h"

EggBus eggBus;

void setup(){
  Serial.begin(9600);
}

void loop(){
  pollBus();
  delay(2000);  
}

void printAddress(uint8_t * address){
  for(uint8_t jj = 0; jj < 6; jj++){
    if(address[jj] < 16) Serial.print("0");
    Serial.print(address[jj], HEX);
    if(jj != 5 ) Serial.print(":");
  }
  Serial.println();
}

void pollBus(){
  uint8_t   egg_bus_address;
  eggBus.init();
  while(egg_bus_address = eggBus.next()){
    Serial.println("===========================");
    Serial.print("Egg Bus Address: 0x");
    Serial.println(egg_bus_address, HEX);

    Serial.print("  Sensor Address: ");
    printAddress(eggBus.getSensorAddress());        
  
    uint8_t numSensors = eggBus.getNumSensors();
    for(uint8_t ii = 0; ii < numSensors; ii++){
    
      Serial.println("---------------------------");
      Serial.print("  Sensor Index: ");
      Serial.println(ii, DEC);       
      
      Serial.print("    Sensor Type: ");
      Serial.println(eggBus.getSensorType(ii));
      
      Serial.print("    Sensor Value: ");
      Serial.println(eggBus.getSensorValue(ii), DEC);
      
      Serial.print("    Sensor Units: ");
      Serial.println(eggBus.getSensorUnits(ii));
      
      uint32_t adc_value = -1;
      uint32_t low_side_resistance = -1;
      eggBus.getRawValue(ii, &adc_value, &low_side_resistance);
      
      Serial.print("    Raw Value ADC: ");
      Serial.println(adc_value, DEC);
      Serial.print("    Raw Value Low Side Resistance: ");
      Serial.println(low_side_resistance, DEC);
    }
  } 
}
