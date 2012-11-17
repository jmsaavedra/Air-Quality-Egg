
/* sensor vars and read functions */
#include <EtherCard.h>
#include <SoftReset.h>
#include <AQERF_Base.h>
#include <avr/wdt.h>
#include "MemoryLocations.h" 

extern char website[] PROGMEM;
uint32_t last_cosm_response_timestamp = 0;

Stash stash;
static byte tcp_session;

void postSensorData(){  
  byte sd = stash.create();  
  
  stash.print(rflink.getSensorType());
  stash.print(F(","));
  stash.println(rflink.getSensorValue(), 8); 
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
}

void addressToString(uint8_t * address, char * target, char delimiter){
  uint8_t target_index = 0;
  for(uint8_t jj = 0; jj < 6; jj++){
    snprintf_P(target + target_index, 3, PSTR("%02X"), address[jj]);
    target_index+=2;
    if(jj != 5 ) target[target_index++] = delimiter;
  }
}

void checkCosmReply(){
  const char *reply = ether.tcpReply(tcp_session);
  if(reply != 0){
    Serial.println(F(">>> RESPONSE RECEIVED ---"));
    Serial.println(reply);
    last_cosm_response_timestamp = millis();
  }  
}

uint32_t getTimeSinceLastCosmResponse(){
   uint32_t now = millis();
   if(now >= last_cosm_response_timestamp){
     return now - last_cosm_response_timestamp;
   }
   else{
     return ((uint32_t) 0xffffffff) - last_cosm_response_timestamp + now;
   }
}
