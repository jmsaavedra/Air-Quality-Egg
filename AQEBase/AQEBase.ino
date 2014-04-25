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
uint8_t pairing_rx_count = 0;

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
    
    Serial.println(F("\n[Air Quality Egg - Base Egg - v2.03]"));
    Serial.print(F("Unit Address: "));
    printlnMAC(mymac);
    
    rgb.bounceColor(yellow, 3000);
    rflink.pairInit();
    rflink.setPairingRxCallback(pairingRx);
    while(!rflink.pair()){
      rgb.render(); 
    }
    rgb.stop_animation();
    rgb.setColor(black);
    
    if(pairing_rx_count > 0){
      blinkColorN(blue, 3, 1000);
    }
    else{
      blinkColorN(magenta, 3, 1000);
    }
    
    Serial.println(F("Pairing complete"));    
    
    setupNanode();
    
    blinkColorN(cyan, 3, 1000);
    
    activateWithCosm();  
  
    blinkColorN(green, 3, 1000);      
    
    markCosmResponse();
    
    wdt_enable(WDTO_8S);
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
    wdt_reset();
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

void pairingRx(uint8_t * packet){
  Serial.print(millis());
  Serial.println(F(" Received Packet During Pairing"));
  for(int i = 0; i < 8; i++){
    Serial.print(packet[i]);
    Serial.print(F(" "));
  }
  Serial.println();
  
  pairing_rx_count++;
  Serial.print(F("Pairing Event Count: "));
  Serial.println(pairing_rx_count);
}

void blinkColorN(uint8_t * color, uint8_t num_times, uint32_t period_ms){
    for(uint16_t ii = 0; ii < num_times; ii++){
      rgb.setColor(black);
      delay(period_ms / 2);
      rgb.setColor(color);
      delay(period_ms / 2);       
    }
    
    rgb.setColor(black);
}
