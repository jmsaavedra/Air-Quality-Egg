
/* sensor vars and read functions */
#include <EtherCard.h>
#include <SoftReset.h>
#include <AQERF_Base.h>
#include <avr/wdt.h>
#include <string.h>
#include "MemoryLocations.h" 

extern char website[] PROGMEM;

long previousCosmResponseTimestamp = 0;
const long TOO_LONG_FOR_COSM_RESPONSE = 600000; // 10 minutes

Stash stash;
static byte tcp_session;

void postSensorData(){  
  byte sd = stash.create();  
  char temp[64] = {0};
  char temp2[8] = {0};
  char delimiter[2] = "_";
  boolean isRaw = false;
  boolean isR0 = false;
  
  int sensor_type_length = strlen(rflink.getSensorType());
  strcat(temp, rflink.getSensorType());
  strcat(temp, delimiter);
  stringConvertMAC(rflink.getSourceSensorAddress(), temp + sensor_type_length + 1,  '-');
  strcat(temp, delimiter);
  itoa(rflink.getSensorIndex(), temp2, 10);
  strcat(temp, temp2);
  
  stash.print(F("{\"datastreams\":[{\"id\": \""));
  stash.print(temp);
  stash.print(F("\",\"current_value\":\""));
  stash.print(rflink.getSensorValue());
  stash.print(F("\",\"tags\":[\"aqe:sensor_type="));

  Serial.print(F("Sensor Type: "));
  Serial.println(rflink.getSensorType());
  if(strstr_P(rflink.getSensorType(), PSTR("_raw")) != NULL){
    memset(temp, 0, 64);
    strncpy(temp, rflink.getSensorType(), strlen(rflink.getSensorType()) - 4); // always ends in "_raw" if it has it
    stash.print(temp);    
    isRaw = true;    
  }
  else if(strstr_P(rflink.getSensorType(), PSTR("_r0")) != NULL){
    memset(temp, 0, 64);
    strncpy(temp, rflink.getSensorType(), strlen(rflink.getSensorType()) - 3); // always ends in "_r0" if it has it
    stash.print(temp);    
    isR0 = true;    
  }  
  else{
    stash.print(rflink.getSensorType());
  }  
  
  stash.print(F("\",\"aqe:sensor_address="));
  memset(temp, 0, 64);
  stringConvertMAC(rflink.getSourceSensorAddress(), temp, ':');
  stash.print(temp);
  
  stash.print(F("\",\"aqe:sensor_index="));
  stash.print(rflink.getSensorIndex());  
  
  stash.print(F("\",\"aqe:data_origin="));  
  if(isRaw){
    stash.print(F("raw"));
  }
  else if(isR0){
    stash.print(F("r0"));
  }
  else{
    stash.print(F("computed"));    
  }

  stash.print(F("\",\"aqe:firmware_version="));  
  stash.print(rflink.getRemoteFirmwareVersion());
  
  stash.print(F("\"],\"unit\":{\"label\":\""));
  stash.print(rflink.getSensorUnits());
  stash.print(F("\",\"symbol\":\""));
  stash.print(rflink.getSensorUnits());
  stash.print(F("\"}}]}"));
 
  stash.save();
  
  Serial.println(F("Preparing stash"));  
  Stash::prepare(PSTR("PUT http://$F/v2/feeds/$E.json HTTP/1.0" "\r\n"
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
    markCosmResponse();
  }  
  
}

void stringConvertMAC(uint8_t * mac, char * buf, char delimiter){
  for(uint8_t ii = 0; ii < 6; ii++){
    convertByteArrayToAsciiHex(mac + ii, buf + 3*ii, 1);
    if(ii == 5) buf[3*ii+2] = '\0';
    else buf[3*ii+2] = delimiter;
  }
}

boolean haventHeardFromCosmLately(){
  unsigned long currentMillis = millis();  
  if((currentMillis - previousCosmResponseTimestamp) > TOO_LONG_FOR_COSM_RESPONSE){
    return true; 
  }
  return false; 
}

void markCosmResponse(){
   previousCosmResponseTimestamp = millis();

}

