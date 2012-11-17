#include <AQERF_Base.h>
#include <RF12.h>
#include <NanodeMAC.h>
#include <EtherCard.h>
#include <StackPaint.h>
#include <string.h>
#include <StackPaint.h>
#include <SoftReset.h>
#include <avr/wdt.h>

#define TOO_LONG_FOR_COSM_RESPONSE 600000L // 10 minutes

// read your MAC address
static uint8_t mymac[6] = {0,0,0,0,0,0};
NanodeMAC mac(mymac);

// create an AQERF_Base object to handle the RF Link
// informing it what the unit MAC address is
AQERF_Base rflink(mymac); 

void printReceivedData();
void printMAC(uint8_t * mac);

void setup(){
    Serial.begin(115200);
    PRINT_STACK_SPACE;
    
    Serial.println(F("\n[Air Quality Egg - Base Egg - v1.00]"));
    Serial.print(F("Unit Address: "));
    printlnMAC(mymac);
    
    rflink.pair();
    Serial.println(F("Pairing complete"));    
    
    setupNanode();
    activateWithCosm();    
    
}

void loop(){
    ether.packetLoop(ether.packetReceive());    
    checkCosmReply();  
  
    if(rflink.dataReceived()){
        printReceivedData();
        postSensorData();
    }
    
    if(getTimeSinceLastCosmResponse() > TOO_LONG_FOR_COSM_RESPONSE){
      Serial.println(F("Too Long Since Last Response From Cosm, Resetting"));
      Serial.flush();
      delay(1000);
      soft_restart(); 
    }
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
