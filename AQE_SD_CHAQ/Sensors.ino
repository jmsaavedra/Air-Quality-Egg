
/* sensor vars and read functions */
#include <EtherCard.h>
#include <SoftReset.h>
#include <avr/wdt.h>
#include <DHT.h>
#include <string.h>
#include "MemoryLocations.h" 

#define AQE_BASESENSOR_FIRMWARE_VERSION 0x1A

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

char current_sensor_units[16] = {0};

uint8_t current_sensor_index;
uint8_t current_firmware_version;


void addressToString(uint8_t * address, char * target, char delimiter){
  uint8_t target_index = 0;
  for(uint8_t jj = 0; jj < 6; jj++){
    snprintf_P(target + target_index, 3, PSTR("%02X"), address[jj]);
    target_index+=2;
    if(jj != 5 ) target[target_index++] = delimiter;
  }
}

void stringConvertMAC(uint8_t * mac, char * buf, char delimiter){
  for(uint8_t ii = 0; ii < 6; ii++){
    convertByteArrayToAsciiHex(mac + ii, buf + 3*ii, 1);
    if(ii == 5) buf[3*ii+2] = '\0';
    else buf[3*ii+2] = delimiter;
  }
}
//--- for generating the SHA1 Hash
static void convertByteArrayToAsciiHex(uint8_t* hash, char * returnString, uint8_t byte_array_length) {
  const char digit2Ascii[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
  for (int i=0; i<byte_array_length; i++) {
    returnString[2*i    ] = digit2Ascii[hash[i]>>4];
    returnString[2*i + 1] = digit2Ascii[hash[i]&0xf];
  }
  returnString[2*(byte_array_length-1) + 2] = '\0';
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
  //memcpy(current_sensor_address, mymac, 6);
  current_sensor_value = getTemperature();
  current_sensor_index = 1; 
  current_firmware_version = AQE_BASESENSOR_FIRMWARE_VERSION;
}

void sendHumidity(){
  clearDataVariables();
  strcpy_P(current_sensor_type, PSTR("Humidity"));
  strcpy_P(current_sensor_units, PSTR("%"));
  //memcpy(current_sensor_address, mymac, 6);
  current_sensor_value = getHumidity();
  current_sensor_index = 0; 
  current_firmware_version = AQE_BASESENSOR_FIRMWARE_VERSION;
}

void sendEggBus(uint8_t sendType, int index){
  clearDataVariables();
  
  strcpy(current_sensor_type, eggBus.getSensorType(index));
  memcpy(current_sensor_address, eggBus.getSensorAddress(), 6);
  current_sensor_index = index; 
  current_firmware_version = eggBus.getFirmwareVersion();  
  
  if(sendType == SEND_RAW){
      strcpy_P(current_sensor_units, PSTR("ohms"));
      strcat_P(current_sensor_type, PSTR("_raw"));
      uint32_t measured_value = eggBus.getSensorIndependentVariableMeasure(index);
      float i_scaler = eggBus.getIndependentScaler(index);
      uint32_t r0 = eggBus.getSensorR0(index);
      uint32_t resistance = 0xffffffff;    
      if(measured_value != 0xffffffff){
        resistance = (uint32_t) (measured_value * i_scaler * r0); 
      }      
      
      current_sensor_value = resistance;
  }
  else{
      strcpy(current_sensor_units, eggBus.getSensorUnits(index));   
      current_sensor_value = eggBus.getSensorValue(index);
  }
}


