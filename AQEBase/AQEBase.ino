#include <AQERF_Base.h>
#include <RF12.h>
#include <NanodeMAC.h>
#include <EtherCard.h>
#include <StackPaint.h>
#include <string.h>
#include <StackPaint.h>
#include <RGB_LED.h>
#include <SoftReset.h>
#include <avr/wdt.h>

// read your MAC address
static uint8_t mymac[6] = {0,0,0,0,0,0};
NanodeMAC mac(mymac);
RGB_LED rgb;

// create an AQERF_Base object to handle the RF Link
// informing it what the unit MAC address is
AQERF_Base rflink(mymac); 

void printReceivedData();
void printMAC(uint8_t * mac);

uint8_t black[3]    = {0x00,0x00,0x00};
uint8_t red[3]      = {0xff,0x00,0x00};
uint8_t green[3]    = {0x00,0xff,0x00};
uint8_t blue[3]     = {0x00,0x00,0xff};
uint8_t cyan[3]     = {0x00,0xff,0xff};
uint8_t yellow[3]   = {0xff,0xff,0x09};
uint8_t magenta[3]  = {0xff,0x00,0xff};
uint8_t white[3]    = {0xff,0xff,0xff};
uint8_t * color_cycle[] = { green, blue, cyan, magenta };
uint8_t color_cycle_index = 0;

void setup(){
    Serial.begin(115200);
    PRINT_STACK_SPACE;
    
    Serial.println(F("\n[Air Quality Egg - Base Egg - v1.01]"));
    Serial.print(F("Unit Address: "));
    printlnMAC(mymac);
    
    rgb.bounceColor(yellow, 3000);
    rflink.pairInit();
    while(!rflink.pair()){
      rgb.render(); 
    }
    rgb.stop_animation();
    rgb.setColor(black);
    
    Serial.println(F("Pairing complete"));    
    
    setupNanode();
    activateWithCosm();    
    markCosmResponse();
}

void loop(){
  
    ether.packetLoop(ether.packetReceive());    
    checkCosmReply();  
  
    if(rflink.dataReceived()){    
        rgb.bounceColorN(color_cycle[color_cycle_index++], 1000, 1);
        if(color_cycle_index > 3) color_cycle_index = 0;
        printReceivedData();
        postSensorData();
    }
    
    if(haventHeardFromCosmLately()){
      Serial.println(F("Too Long Since Last Response From Cosm, Resetting"));
      Serial.flush();
      rgb.setColor(yellow);
      delay(10000);      
      soft_restart(); 
    }
    
    rgb.render();
}

void printReceivedData(){
    Serial.print(F("Packet Received @ "));
    Serial.println(millis());
    Serial.print(F("  Packet Type: "));
    Serial.println(rflink.getPacketType(), HEX);
    Serial.print(F("  Remote Firmware Version: "));
    Serial.println(rflink.getRemoteFirmwareVersion(), DEC);
    Serial.print(F("  Remote Station Address: "));
    printlnMAC(rflink.getRemoteStationAddress());
    Serial.print(F("  Source Sensor Address: "));
    printlnMAC(rflink.getSourceSensorAddress());
    Serial.print(F("  Sensor Index: "));
    Serial.println(rflink.getSensorIndex(), DEC);    
    Serial.print(F("  Sensor Type: "));
    Serial.println(rflink.getSensorType());
    Serial.print(F("  Sensor Units: "));
    Serial.println(rflink.getSensorUnits());
    Serial.print(F("  Sensor Value: "));
    Serial.println(rflink.getSensorValue());
    Serial.println();
}

void printlnMAC(uint8_t * mac){
    for(uint8_t i = 0; i < 6; i++){
        if(mac[i] < 10) Serial.print(F("0"));
        Serial.print(mac[i], HEX);
        if(i != 5){        
            Serial.print(F("_"));
        }
    }
    Serial.println();
}
