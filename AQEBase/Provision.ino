#include <sha1.h>
#include <SoftReset.h>
#include <StackPaint.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include "MemoryLocations.h"

extern uint8_t mymac[6];           // defined in Nanode.ino
extern char website[] PROGMEM;

#define PROVISIONING_STATUS_GOOD 0x73
boolean activated = false;

//--- for generating the SHA1 Hash
static void convertByteArrayToAsciiHex(uint8_t* hash, char * returnString, uint8_t byte_array_length) {
  const char digit2Ascii[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
  for (int i=0; i<byte_array_length; i++) {
    returnString[2*i    ] = digit2Ascii[hash[i]>>4];
    returnString[2*i + 1] = digit2Ascii[hash[i]&0xf];
  }
  returnString[2*(byte_array_length-1) + 2] = '\0';
}

// computes the activation_url
void computeActivationUrl(char * activation_url){
  
  //Air Quality Egg v01 Product Secret:
  //e54c89cf2b916934a288304b6f7630e5b9aad8c8
  uint8_t deviceKey[] ={ 
    0xe5,0x4c,0x89,0xcf,0x2b,0x91,0x69,0x34,0xa2,0x88,
    0x30,0x4b,0x6f,0x76,0x30,0xe5,0xb9,0xaa,0xd8,0xc8
  };
  
  //--- pull MAC address from Nanode, stick it in serialNumber  
  char serial_number[18] = {0};
  Serial.print(F("Serial #: "));
  for(uint8_t ii = 0; ii < 6; ii++){
    convertByteArrayToAsciiHex(mymac + ii, serial_number + 3*ii, 1);
    if(ii == 5) serial_number[3*ii+2] = '\0';
    else serial_number[3*ii+2] = ':';
  }
  Serial.println(serial_number);  
  
  //--- generate SHA1
  Serial.println(F("compute sha: "));
  
  #define HMAC_LENGTH 20
  Sha1.initHmac(deviceKey,HMAC_LENGTH); 
  Sha1.print(serial_number);
  
  convertByteArrayToAsciiHex(Sha1.resultHmac(), activation_url, HMAC_LENGTH);
  
  PRINT_STACK_SPACE;  
  
  strcat_P(activation_url, PSTR("/activate"));
  Serial.println(activation_url);  
  
  activation_url[ACTIVATION_URL_LENGTH-1] = '\0'; // ensure null terminated  
}

// called when the client request is complete
static void provisioningCallback (byte status, word off, word len) {
  boolean foundFeedId = false;
  boolean foundApiKey = false;
  uint8_t api_key_strlen = 0;
  uint8_t feed_id_strlen = 0;
  
  const char* cbuf = ((const char *) Ethernet::buffer + off);
  
  Serial.println(F(">>> PROVISIONING CALLBACK"));  
  Serial.println((int) status);
  Serial.println((int) off);
  Serial.println((int) len);

  
  char* token = strtok((char *)cbuf, ":,\"\n"); //tokenize

  while ( token != 0 ){

    Serial.print(F("Token: "));
    Serial.println(token);

    //--- setting the apikey and feedid (after they've been found)
    if (foundApiKey){
      // strncpy(apiKey, token, API_KEY_LENGTH);
      // put the token in EEPROM to be retrieved later
      setApiKeyInEEPROM(token);
      api_key_strlen = strlen(token);
      foundApiKey = false;
    } 
    if (foundFeedId){ 
      // strncpy(feedId, token, FEED_ID_LENGTH);
      // put the token in EEPROM to be retrieved later
      setFeedIdInEEPROM(token);
      feed_id_strlen = strlen(token);
      foundFeedId = false;
    }

    //--- looking for the apikey and feedid
    if(strncmp_P(token, PSTR("apikey"),6)==0){
      Serial.print(F(">>>FOUND APIKEY\n"));
      foundApiKey = true; //the next token will be the apikey
    } 
    if(strncmp_P(token,PSTR("feed_id"),6)==0){
      Serial.print(F(">>>FOUND FEED_ID\n"));
      foundFeedId = true; //the next token will be the feedid
    }

    if(feed_id_strlen > 0 && api_key_strlen > 0){
      activated = true;
      Serial.print(F("API KEY LENGTH = "));
      Serial.println(api_key_strlen);        
      Serial.print(F("FEED LENGTH = "));
      Serial.println(feed_id_strlen);
      eeprom_write_byte((uint8_t *) ACTIVATION_STATUS_EEPROM_ADDRESS, PROVISIONING_STATUS_GOOD);   
      rgb.setColor(green);
      delay(10000);
      soft_restart();     
    }
    
    token = strtok (0, ":,\"\n"); // advance token pointer
  }

  Serial.println(F("..."));
}

#define MAX_ACTIVATION_ATTEMPTS         4   // about 1 minutes worth of attempts
#define INITIAL_PACKET_DELAY_MS      10000L // 10 seconds
#define ACTIVATION_RETRY_INTERVAL_MS 15000L // retry every 15 seconds
void activateWithCosm(){
  uint8_t test = eeprom_read_byte((const uint8_t *) ACTIVATION_STATUS_EEPROM_ADDRESS);
  if(test == PROVISIONING_STATUS_GOOD){
    Serial.println(F("Previously provisioned"));
    return;   
  }
  else{ 
    Serial.print(F("Uninitialized - test value = "));
    Serial.println(test, HEX);
    ether.persistTcpConnection(true);
    doProvisioning();
    ether.persistTcpConnection(false);
  }
}

void doProvisioning(){
  uint32_t timer = INITIAL_PACKET_DELAY_MS; // initial waiting period
  uint32_t current_time = 0;
  uint16_t numAttempts = 0;  
  char activation_url[ACTIVATION_URL_LENGTH];  
 
  computeActivationUrl(activation_url);
 
  activated = false;
  for(;;){        
    if(numAttempts == MAX_ACTIVATION_ATTEMPTS){
      Serial.print(F("Provisioning failed "));
      Serial.print(MAX_ACTIVATION_ATTEMPTS);
      Serial.println(F(" times, restarting"));
      Serial.flush();
      rgb.setColor(red);
      delay(10000);      
      soft_restart(); // better reset at this point...
    }
    
    ether.packetLoop(ether.packetReceive());
    
    current_time = millis();
    if (current_time > timer) {
      timer = current_time + ACTIVATION_RETRY_INTERVAL_MS;
      Serial.println();
      if(!activated){
        Serial.print(F("<<< REQ "));
        ether.browseUrl(PSTR("/v2/devices/"), activation_url, website, provisioningCallback);
        numAttempts++;
        //URL hit is something like: 
        //http://api.cosm.com/v2/devices/0aad07fc55fb297074a48d0cb9ca950e0d92ed72/activate
      }  
      else {
        Serial.println(F(">>ACTIVATED<<"));
        // the callback function will have stored the api key and feed id in EEPROM
        return; // we are activated
      }
      PRINT_STACK_SPACE; 
    }     
  }   
}

void setApiKeyInEEPROM(char * api_key){
  eeprom_write_block (api_key, (void *) API_KEY_EEPROM_ADDRESS, API_KEY_LENGTH);
  Serial.print(F("Wrote API KEY to EEPROM: "));
  Serial.println(api_key);
}

void setFeedIdInEEPROM(char * feed_id){
  eeprom_write_block (feed_id, (void *) FEED_ID_EEPROM_ADDRESS, FEED_ID_LENGTH);
  Serial.print(F("Wrote FEED ID to EEPROM: "));
  Serial.println(feed_id);
}
