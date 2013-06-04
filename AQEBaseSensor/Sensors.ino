
/* sensor vars and read functions */
#include <EtherCard.h>
#include <SoftReset.h>
#include <avr/wdt.h>
#include <DHT.h>
#include <string.h>
#include "MemoryLocations.h" 

#define AQE_BASESENSOR_FIRMWARE_VERSION 0x22

extern char website[] PROGMEM;
extern uint8_t eggbus_sensor_index;

long previousCosmResponseTimestamp = 0;
const long TOO_LONG_FOR_COSM_RESPONSE = 600000; // 10 minutes

Stash stash;
static byte tcp_session;

#define DHTPIN 17
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

void sensorSetup(){
  //Serial.println("--- DHT22 BEGIN ---");
  dht.begin(); 
  delay(500);
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

char current_sensor_type[16] = {0};
char current_sensor_units[16] = {0};
uint8_t current_sensor_address[6] = {0};
float current_sensor_value;
uint8_t current_sensor_index;
uint8_t current_firmware_version;

void postSensorData(){  
  byte sd = stash.create();  
  char temp[64] = {0};
  char temp2[8] = {0};
  char delimiter[2] = "_";
  boolean isRaw = false;
  boolean isR0 = false;  
  int sensor_type_length = strlen(current_sensor_type);
  strcat(temp, current_sensor_type);
  strcat(temp, delimiter);
  stringConvertMAC(current_sensor_address, temp + sensor_type_length + 1,  '-');
  strcat(temp, delimiter);
  itoa(current_sensor_index, temp2, 10);
  strcat(temp, temp2);
  
  stash.print(F("{\"datastreams\":[{\"id\": \""));
  stash.print(temp);
  stash.print(F("\",\"current_value\":\""));
  stash.print(current_sensor_value, 8);
  stash.print(F("\",\"tags\":[\"aqe:sensor_type="));

  if(strstr_P(current_sensor_type, PSTR("_raw")) != NULL){
    memset(temp, 0, 64);
    strncpy(temp, current_sensor_type, strlen(current_sensor_type) - 4); // always ends in "_raw" if it has it
    stash.print(temp);    
    isRaw = true;    
  }
  else if(strstr_P(current_sensor_type, PSTR("_r0")) != NULL){
    memset(temp, 0, 64);
    strncpy(temp, current_sensor_type, strlen(current_sensor_type) - 3); // always ends in "_r0" if it has it
    stash.print(temp);    
    isR0 = true;     
  }
  else{
    stash.print(current_sensor_type);
  }  
  
  stash.print(F("\",\"aqe:sensor_address="));
  memset(temp, 0, 64);
  stringConvertMAC(current_sensor_address, temp, ':');
  stash.print(temp);
  
  stash.print(F("\",\"aqe:sensor_index="));
  stash.print(current_sensor_index);  
  
  stash.print(F("\",\"aqe:firmware_version="));
  stash.print(current_firmware_version);    
  
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
  
  stash.print(F("\"],\"unit\":{\"label\":\""));
  stash.print(current_sensor_units);
  stash.print(F("\",\"symbol\":\""));
  stash.print(current_sensor_units);
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
  Serial.print(F("Data sent for "));  
  Serial.println(current_sensor_type);
}

void addressToString(uint8_t * address, char * target, char delimiter){
  uint8_t target_index = 0;
  for(uint8_t jj = 0; jj < 6; jj++){
    snprintf_P(target + target_index, 3, PSTR("%02X"), address[jj]);
    target_index+=2;
    if(jj != 5 ) target[target_index++] = delimiter;
  }
}

uint8_t checkCosmReply(){
  const char *reply = ether.tcpReply(tcp_session);
  if(reply != 0){
    Serial.println(F(">>> RESPONSE RECEIVED ---"));
    markCosmResponse();
    return 1;
  }  
  return 0;
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

void clearDataVariables(){
  memset(current_sensor_type, 0, 16);
  memset(current_sensor_units, 0, 16);
  memset(current_sensor_address, 0, 6);
  current_sensor_value = 0;
  current_sensor_index = 0;
  current_firmware_version = 0;
}

void sendTemperature(){
  clearDataVariables();
  strcpy_P(current_sensor_type, PSTR("Temperature"));
  strcpy_P(current_sensor_units, PSTR("deg C"));
  memcpy(current_sensor_address, mymac, 6);
  current_sensor_value = getTemperature();
  current_sensor_index = 1; 
  current_firmware_version = AQE_BASESENSOR_FIRMWARE_VERSION;
}

void sendHumidity(){
  clearDataVariables();
  strcpy_P(current_sensor_type, PSTR("Humidity"));
  strcpy_P(current_sensor_units, PSTR("%"));
  memcpy(current_sensor_address, mymac, 6);
  current_sensor_value = getHumidity();
  current_sensor_index = 0; 
  current_firmware_version = AQE_BASESENSOR_FIRMWARE_VERSION;
}

void sendEggBus(uint8_t sendType){
  clearDataVariables();
  
  strcpy(current_sensor_type, eggBus.getSensorType(eggbus_sensor_index));
  memcpy(current_sensor_address, eggBus.getSensorAddress(), 6);
  current_sensor_index = eggbus_sensor_index; 
  current_firmware_version = eggBus.getFirmwareVersion();  
  
  if(sendType == SEND_RAW){
      strcpy_P(current_sensor_units, PSTR("ohms"));
      strcat_P(current_sensor_type, PSTR("_raw"));
      current_sensor_value = (uint32_t) eggBus.getSensorResistance(eggbus_sensor_index);
  }
  else if(sendType == SEND_R0){
      strcpy_P(current_sensor_units, PSTR("ohms"));
      strcat_P(current_sensor_type, PSTR("_r0"));
      current_sensor_value = eggBus.getSensorR0(eggbus_sensor_index);    
  }
  else{
      strcpy(current_sensor_units, eggBus.getSensorUnits(eggbus_sensor_index));   
      current_sensor_value = eggBus.getSensorValue(eggbus_sensor_index);
  }
}


