#include <stdint.h>
#include <DHT.h>
#include "Wire.h"
#include "EggBus.h"

EggBus eggBus;

#define DHTPIN 17 //analog pin 3
#define DHTTYPE DHT22  
DHT dht(DHTPIN, DHTTYPE);

void setup(){
  Serial.begin(9600);
  Serial.println(F("Air Quality Egg - EPA Serial Build v1.0"));
  Serial.println(F("======================================================================\r\n"));
  Serial.println(F("Timestamp, Sensor Type, Sensor Value, Sensor Units, Sensor Resistance"));
  Serial.println(F("----------------------------------------------------------------------"));  
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
      Serial.print(millis(), DEC);
      Serial.print(F(", "));      
      
      Serial.print(eggBus.getSensorType(ii));
      Serial.print(F(", "));
      
      i_scaler = eggBus.getIndependentScaler(ii);      
      measured_value = eggBus.getSensorIndependentVariableMeasure(ii);
      r0 = eggBus.getSensorR0(ii);
      
      Serial.print(eggBus.getSensorValue(ii), 8);          
      Serial.print(F(", "));      

      Serial.print(eggBus.getSensorUnits(ii));            
      Serial.print(F(", "));      
      
      if(measured_value == 0xffffffff){
        Serial.print(F("OPEN CIRCUIT"));
      }
      else{
        Serial.print(measured_value * i_scaler * r0, 8);
      }      
      
      Serial.println();
    }
  }

  Serial.print(millis(), DEC);
  Serial.print(F(", "));      
  
  Serial.print("Humidity");
  Serial.print(F(", "));
  float currHumidity = getHumidity();
  Serial.print(currHumidity, 8);
  Serial.print(F(", "));      

  Serial.println(F("%, n/a"));
  delay(2500);



  Serial.print(millis(), DEC);
  Serial.print(F(", "));      
  
  Serial.print("Temperature");
  Serial.print(F(", "));
  float currentTemperature = getTemperature();
  Serial.print(currentTemperature, 8);
  Serial.print(F(", "));      

  Serial.println(F("degrees C, n/a"));
  delay(2500);

  Serial.println();
}

void printAddress(uint8_t * address){
  for(uint8_t jj = 0; jj < 6; jj++){
    if(address[jj] < 16) Serial.print("0");
    Serial.print(address[jj], HEX);
    if(jj != 5 ) Serial.print(":");
  }
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
