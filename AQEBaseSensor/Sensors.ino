
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

void sensorSetup(){
  //Serial.println("--- DHT22 BEGIN ---");
  dht.begin(); 
  delay(500);
}

// this takes *at least* 5 seconds to accomodate the DHT22
void postSensorData(){
  // didn't get a response from Cosm since the last post... restart
  if(cosmResponseReceived == 0 && cosmResponseExpected == 1){
     Serial.println(F("No Response from received from Cosm, restarting"));
     delay(1000);
     soft_restart();
  }
  
  char address_buffer[18] = {0};
  byte sd = stash.create();  
  
  float currHumidity = getHumidity();
  Serial.print(F("Humidity: "));
  Serial.println(currHumidity);
  stash.print(F("humidity,"));
  stash.println(currHumidity);
  delay(2500);  
  
  float currTemp = getTemperature();
  Serial.print(F("Temperature: "));
  Serial.println(currTemp);  
  stash.print(F("temperature,"));
  stash.println(currTemp);
  delay(2500);

  // TODO: post button value
  
  uint8_t   egg_bus_address;
  eggBus.init();
  while((egg_bus_address = eggBus.next())){
    Serial.println(F("==========================="));
    Serial.print(F("Egg Bus Address: 0x"));
    Serial.println(egg_bus_address, HEX);
    Serial.print(F("  Sensor Address: "));
    addressToString(eggBus.getSensorAddress(), address_buffer, '_');
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
      
      stash.print(sensorType);
      stash.print(F("_"));
      stash.print(address_buffer);
      stash.print(F("_"));
      stash.print(ii, HEX);
      stash.print(F(","));
      stash.println(eggBus.getSensorValue(ii));
    }
  }
  delay(1000); // delay between each reading
  
  stash.save();
  Serial.println(F("Preparing stash"));  
  Stash::prepare(PSTR("PUT http://$F/v2/feeds/$E.csv HTTP/1.0" "\r\n"
    "Host: $F" "\r\n"
    "X-PachubeApiKey: $E" "\r\n"
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

void addressToString(uint8_t * address, char * target, char delimiter){
  uint8_t target_index = 0;
  for(uint8_t jj = 0; jj < 6; jj++){
    snprintf_P(target + target_index, 3, PSTR("%02X"), address[jj]);
    target_index+=2;
    if(jj != 5 ) target[target_index++] = delimiter;
  }
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


void checkCosmReply(){
  const char *reply = ether.tcpReply(tcp_session);
  if(reply != 0){
    Serial.println(F(">>> RESPONSE RECEIVED ---"));
    Serial.println(reply);
    cosmResponseReceived = 1;
  }  
}

/*
void getApiKeyFromEEPROM(char * api_key){
  eeprom_read_block (api_key, (const void *) API_KEY_EEPROM_ADDRESS, API_KEY_LENGTH); 
  Serial.print(F("READ API KEY = "));
  Serial.println(api_key);
}

void getFeedIdFromEEPROM(char * feed_id){
  eeprom_read_block (feed_id, (const void *) FEED_ID_EEPROM_ADDRESS, FEED_ID_LENGTH);
  Serial.print(F("READ FEED ID = "));
  Serial.println(feed_id);  
}
*/
