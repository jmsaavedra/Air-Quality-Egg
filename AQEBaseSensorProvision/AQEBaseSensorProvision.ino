#include <Wire.h>
#include <EggBus.h>
#include <NanodeMAC.h>
#include <SoftReset.h>
#include <avr/wdt.h>

#define SENSOR_PACKET_DELAY  5000L
#define FORCED_RESET_TIME_MS 600000L

#define TRANSMIT_STATE_SEND_TEMPERATURE 1
#define TRANSMIT_STATE_SEND_HUMIDITY    2
#define TRANSMIT_STATE_POLL_EGG_BUS     3
#define TRANSMIT_STATE_WAITING          4
static uint8_t transmit_state = TRANSMIT_STATE_WAITING;  
uint8_t eggbus_sensor_index     = 0;

// read your MAC address
static uint8_t mymac[6] = {0,0,0,0,0,0};
NanodeMAC mac(mymac);
EggBus eggBus;

#define SEND_RAW        0
#define SEND_CALCULATED 1
byte send_type = SEND_RAW;

void printMAC(uint8_t * mac);

// support variables
long cosmPostPreviousMillis = 0;
long cosmPostInterval = 60000L;
long cosmDelayPreviousMillis = 0;
long cosmDelayInterval = 5000L;
long heartbeatPreviousMillis = 0;
long heartbeatInterval = 500L;
byte need_to_send = 0;
byte clear_to_send = 1;

void setup(){

    Serial.begin(115200);
    Serial.println(F("\n[Air Quality Egg - Base Sensor Provisioning - v2.01]"));
    Serial.print("Unit Address: ");
    printlnMAC(mymac);    
    
    setupNanode();
    activateWithCosm();                
}  

void loop(){

}

void printlnMAC(uint8_t * mac){
    for(uint8_t i = 0; i < 6; i++){
        if(mac[i] < 10) Serial.print(F("0"));
        Serial.print(mac[i], HEX);
        if(i != 5){
            Serial.print(F(":"));
        }
    }
    Serial.println();
}

