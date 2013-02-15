#include <Wire.h>
#include <EggBus.h>
#include <NanodeMAC.h>
#include <SoftReset.h>
#include "RTClib.h"
#include <avr/wdt.h>
#include <SD.h>

//#define SENSOR_PACKET_DELAY  5000L
//#define FORCED_RESET_TIME_MS 600000L
//
//#define TRANSMIT_STATE_SEND_TEMPERATURE 1
//#define TRANSMIT_STATE_SEND_HUMIDITY    2
//#define TRANSMIT_STATE_POLL_EGG_BUS     3
//#define TRANSMIT_STATE_WAITING          4
//static uint8_t transmit_state = TRANSMIT_STATE_WAITING;  
uint8_t eggbus_sensor_index     = 0;

// read your MAC address
static uint8_t mymac[6] = {
  0,0,0,0,0,0};
//NanodeMAC mac(mymac);
EggBus eggBus;

#define SEND_RAW        0
#define SEND_CALCULATED 1
byte send_type = SEND_RAW;
float current_sensor_value;
char current_sensor_type[16] = {0};
uint8_t current_sensor_address[6] = {0};

//void printMAC(uint8_t * mac);

// support variables
long cosmPostPreviousMillis = 0;
long cosmPostInterval = 60000L;
//long cosmDelayPreviousMillis = 0;
//long cosmDelayInterval = 5000L;
//long heartbeatPreviousMillis = 0;
//long heartbeatInterval = 500L;
//byte need_to_send = 0;
//byte clear_to_send = 1;

//DATA LOGGER
RTC_DS1307 RTC; //REAL TIME CLOCK

const int chipSelect = 10; // CS for SD CARD

File file;

void setup(){
  //randomSeed(analogRead(0));

  Serial.begin(115200);
  Serial.println(F("\n[Air Quality Egg - CHAQ - v1]"));
  //Serial.print("Unit Address: ");
  //printlnMAC(mymac);    
  cosmPostPreviousMillis = 71243411;
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  DateTime now = RTC.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  //SD CARD STUFF
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(SS, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1) ;
  }
  Serial.println("card initialized.");

  // Open up the file we're going to log to!
  file = SD.open("CHAQ_01.txt", FILE_WRITE); // this is the ID of the EGG!!!!!
  if (!file) {
    Serial.println("error opening file");
    // Wait forever since we cant write data
    while (1) ;
  }
}  

void loop(){
  unsigned long currentMillis = millis(); 


  //loopNanode();

  if(currentMillis - cosmPostPreviousMillis > cosmPostInterval) {

    Serial.println(F("Kicking off transmit cycle"));
    cosmPostPreviousMillis = currentMillis;              
    delay(5000);
    eggBus.init();
    delay(1000);

    //2010-05-20T11:01:43Z
    DateTime timestamp = RTC.now();
    ////FILE
    file.print(timestamp.year(), DEC);
    file.print('-');
    if(timestamp.month() < 10){
      file.print(0);
    }
    file.print(timestamp.month(), DEC);
    file.print('-');
    if(timestamp.day() < 10){
      file.print(0);
    }
    file.print(timestamp.day(), DEC);
    file.print('T');
    if(timestamp.hour() < 10){
      file.print(0);
    }
    file.print(timestamp.hour(), DEC);
    file.print(':');
    if(timestamp.minute() < 10){
      file.print(0);
    }
    file.print(timestamp.minute(), DEC);
    file.print(':');
    if(timestamp.second() < 10){
      file.print(0);
    }
    file.print(timestamp.second(), DEC);
    file.print('Z');
    file.print(',');
    //////////////Serial
    Serial.print(timestamp.year(), DEC);
    Serial.print('-');
    if(timestamp.month() < 10){
      Serial.print(0);
    }
    Serial.print(timestamp.month(), DEC);
    Serial.print('-');
    if(timestamp.day() < 10){
      Serial.print(0);
    }
    Serial.print(timestamp.day(), DEC);
    Serial.print('T');
    if(timestamp.hour() < 10){
      Serial.print(0);
    }
    Serial.print(timestamp.hour(), DEC);
    Serial.print(':');
    if(timestamp.minute() < 10){
      Serial.print(0);
    }
    Serial.print(timestamp.minute(), DEC);
    Serial.print(':');
    if(timestamp.second() < 10){
      Serial.print(0);
    }
    Serial.print(timestamp.second(), DEC);
    Serial.print('Z');
    Serial.print(',');
    sendTemperature();
    file.print(current_sensor_type);
    file.print(',');
    file.print(current_sensor_value);
    file.print(',');
    //
    Serial.print(current_sensor_type);
    Serial.print(',');
    Serial.print(current_sensor_value);
    Serial.print(',');
    sendHumidity();
    file.print(current_sensor_type);
    file.print(',');
    file.print(current_sensor_value);
    //
    Serial.print(current_sensor_type);
    Serial.print(',');
    Serial.print(current_sensor_value);
    while(eggBus.next()){
      delay(1000);
      int numSensors = eggBus.getNumSensors();
      for(int i = 0; i < numSensors; i++){
        char temp[64] = {0};
        sendEggBus(SEND_RAW, i);
        memset(temp, 0, 64);
        stringConvertMAC(eggBus.getSensorAddress(), temp, '-');
        
        file.print(',');
        file.print(eggBus.getSensorType(i));
        file.print('_');
        file.print("raw_");
        file.print(temp);
        file.print(',');
        file.print(current_sensor_value);

        Serial.print(',');
        Serial.print(eggBus.getSensorType(i));
        Serial.print('_');
        Serial.print("raw_");
        Serial.print(temp);
        Serial.print(',');
        Serial.print(current_sensor_value);
        
        sendEggBus(SEND_CALCULATED, i);
        
        memset(temp, 0, 64);
        stringConvertMAC(eggBus.getSensorAddress(), temp, '-');
        file.print(',');
        file.print(eggBus.getSensorType(i));
        file.print('_');
        file.print(temp);
        file.print(',');
        file.print(current_sensor_value);

        Serial.print(',');
        Serial.print(eggBus.getSensorType(i));
        Serial.print('_');
        Serial.print(temp);
        Serial.print(',');
        Serial.print(current_sensor_value); 
      }
    }
    file.println();
    file.flush();
    Serial.println();
  }
}

