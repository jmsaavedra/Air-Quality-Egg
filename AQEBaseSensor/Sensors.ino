/* sensor vars and read functions */
#include <Wire.h>
#include <EggBus.h>
#include <DHT.h>
#include <EtherCard.h>
#include <SoftReset.h>
#include <avr/wdt.h>
#include "MemoryLocations.h"

#define DHTPIN 17 //analog pin 3
#define DHTTYPE DHT22  

extern char website[] PROGMEM;

DHT dht(DHTPIN, DHTTYPE);

EggBus eggBus;
Stash stash;
static byte tcp_session;
static byte cosmResponseExpected = 0;
static byte cosmResponseReceived = 0;

static float current_sensor_value = 0.0;
static char * current_sensor_type;
uint8_t * current_source_sensor_address;
uint32_t current_sensor_index;
char * current_sensor_units;

void sensorSetup(){
  //Serial.println("--- DHT22 BEGIN ---");
  dht.begin(); 
  delay(500);
}

// this takes *at least* 5 seconds to accomodate the DHT22
void postSensorData(){  
  char temperature[]       = "Temperature";
  char temperature_units[] = "deg C";
  char humidity[]          = "Humidity";
  char humidity_units[]    = "%";  
  
  // didn't get a response from Cosm since the last post... restart
  if(cosmResponseReceived == 0 && cosmResponseExpected == 1){
     Serial.println(F("No Response from received from Cosm, restarting"));
     delay(1000);
     soft_restart();
  }
  
  char address_buffer[18] = {0};
  byte sd = stash.create();  
  
  stash.print(F("{\"datastreams\":["));
  
  float currHumidity = getHumidity();
  Serial.print(F("Humidity: "));
  Serial.println(currHumidity);
  delay(2500);  
  
  current_sensor_value = currHumidity; 
  current_sensor_units = humidity_units;
  current_sensor_type  = humidity;
  current_source_sensor_address = mymac;
  current_sensor_index = 1;
  addSensorToStash(&stash, false);
  
  float currTemp = getTemperature();
  Serial.print(F("Temperature: "));
  Serial.println(currTemp);  
  delay(2500);

  current_sensor_value = currTemp; 
  current_sensor_units = temperature_units;
  current_sensor_type  = temperature;
  current_source_sensor_address = mymac;
  current_sensor_index = 0;
  addSensorToStash(&stash, true);
  
  uint8_t   egg_bus_address;
  eggBus.init();
  while((egg_bus_address = eggBus.next())){
    Serial.println(F("==========================="));
    Serial.print(F("Egg Bus Address: 0x"));
    Serial.println(egg_bus_address, HEX);
    Serial.print(F("  Sensor Address: "));
    stringConvertMAC(eggBus.getSensorAddress(), address_buffer, '_');
    Serial.println(address_buffer);
    
    uint8_t numSensors = eggBus.getNumSensors();

    for(uint8_t ii = 0; ii < numSensors; ii++){
      char sensorType[16] = {0}, units[16] = {0};
            
      strncpy(sensorType, eggBus.getSensorType(ii), 15);
      strncpy(units, eggBus.getSensorUnits(ii), 15);      
      Serial.println(F("---------------------------"));
      Serial.print(F("Sensor Type: "));
      Serial.println(sensorType);
      Serial.print(F("Sensor Units: "));
      Serial.println(units);
      Serial.print(F("  Sensor Index: "));
      Serial.println(ii, DEC);
      Serial.print(F("  Sensor Value: "));
      Serial.println(eggBus.getSensorValue(ii), DEC);
      
      current_sensor_value = eggBus.getSensorValue(ii); 
      current_sensor_units = eggBus.getSensorUnits(ii);
      current_sensor_type  = eggBus.getSensorType(ii);
      current_source_sensor_address = eggBus.getSensorAddress();
      current_sensor_index = ii;    
      addSensorToStash(&stash, true);

    }
  }
  delay(1000); // delay between each reading
  

  stash.print(F("]}"));
  stash.save();
  Serial.println(F("Preparing stash"));  
  Stash::prepare(PSTR("PUT http://$F/v2/feeds/$E.csv HTTP/1.0" "\r\n"
    "Host: $F" "\r\n"
    "X-ApiKey: $E" "\r\n"
    "Content-Length: $D" "\r\n"
    "\r\n"
    "$H"),
  website, 
  (const void *) FEED_ID_EEPROM_ADDRESS, 
  website, 
  (const void *) API_KEY_EEPROM_ADDRESS, 
  stash.size(), 
  sd);

  Serial.println(F("Sending data to Cosm"));
  
  tcp_session = ether.tcpSend();  
  Serial.println(F("Data sent"));
  cosmResponseExpected = 1;
  cosmResponseReceived = 0;
  
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
  
  return h;
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


void checkCosmReply(){
  const char *reply = ether.tcpReply(tcp_session);
  if(reply != 0){
    Serial.println(F(">>> RESPONSE RECEIVED ---"));
    Serial.println(reply);
    cosmResponseReceived = 1;
  }  
}

void addSensorToStash(Stash * stash, boolean prepend_comma){
  char temp[64] = {0};
  char temp2[8] = {0};
  char delimiter[2] = "_";
  boolean isRaw = false;
  
  int sensor_type_length = strlen(current_sensor_type);
  strcat(temp, current_sensor_type);
  strcat(temp, delimiter);
  stringConvertMAC(current_source_sensor_address, temp + sensor_type_length + 1,  '-');
  strcat(temp, delimiter);
  itoa(current_sensor_index, temp2, 10);
  strcat(temp, temp2);
  
  if(prepend_comma){
    stash->print(F(",")); 
  }
  
  stash->print(F("{\"id\": \""));
  stash->print(temp);
  stash->print(F("\",\"current_value\":\""));
  stash->print(current_sensor_value, 8);
  stash->print(F("\",\"tags\":[\"aqe:sensor_type="));

  if(strstr_P(current_sensor_type, PSTR("_raw")) != NULL){
    memset(temp, 0, 32);
    strncpy(temp, current_sensor_type, strlen(current_sensor_type) - 4); // always ends in "_raw" if it has it
    stash->print(temp);    
    isRaw = true;    
  }
  else{
    stash->print(current_sensor_type);
  }  
  
  stash->print(F("\",\"aqe:sensor_address="));
  memset(temp, 0, 32);
  stringConvertMAC(current_source_sensor_address, temp, ':');
  stash->print(temp);
  
  stash->print(F("\",\"aqe:sensor_index="));
  stash->print(current_sensor_index);  
  
  stash->print(F("\",\"aqe:data_origin="));  
  if(isRaw){
    stash->print(F("raw"));
  }
  else{
    stash->print(F("computed"));    
  }
  
  stash->print(F("\"],\"unit\":{\"label\":\""));
  stash->print(current_sensor_units);
  stash->print(F("\",\"symbol\":\""));
  stash->print(current_sensor_units);
  stash->print(F("\"}}]}")); 
}

void stringConvertMAC(uint8_t * mac, char * buf, char delimiter){
    int bufIdx = 0;
    char hex[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    for(uint8_t i = 0; i < 6; i++){
        buf[bufIdx++] = hex[mac[i] >>   8];
        buf[bufIdx++] = hex[mac[i] & 0x0f];
        if(i != 5){        
            buf[bufIdx++] = delimiter;
        }
    }
}
