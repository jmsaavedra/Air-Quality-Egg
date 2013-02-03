#include <AQERF_Base.h>
#include <RF12.h>
#include <NanodeMAC.h>

// read your MAC address
static uint8_t mymac[6] = {0,0,0,0,0,0};
NanodeMAC mac(mymac);

// create an AQERF_Base object to handle the RF Link
// informing it what the unit MAC address is
AQERF_Base rflink(mymac); 

void printReceivedData();
void printMAC(uint8_t * mac);

void pairingRx(uint8_t * packet){
  Serial.print(millis());
  Serial.println(F(" Received Packet During Pairing"));
  for(int i = 0; i < 8; i++){
    Serial.print(packet[i]);
    Serial.print(F(" "));
  }
  Serial.println();
}

void setup(){
    Serial.begin(9600);
    Serial.println("AQE Base RF Unit Test");
    Serial.print("Unit Address: ");
    printlnMAC(mymac);
    rflink.pairInit();
    rflink.setPairingRxCallback(pairingRx);
    while(!rflink.pair()){
        // do nothing... or 'other stuff'
    }
    Serial.println("Pairing complete");
}

void loop(){
    if(rflink.dataReceived()){
        printReceivedData();
    }
}

void printReceivedData(){
    Serial.print("Packet Received @ ");
    Serial.println(millis());

    Serial.print("  Packet Type: ");
    Serial.println(rflink.getPacketType(), HEX);

    Serial.print("  Remote Firmware Version: ");
    Serial.println(rflink.getRemoteFirmwareVersion(), DEC);

    Serial.print("  Remote Station Address: ");
    printlnMAC(rflink.getRemoteStationAddress());

    Serial.print("  Source Sensor Address: ");
    printlnMAC(rflink.getSourceSensorAddress());

    Serial.print("  Sensor Index: ");
    Serial.println(rflink.getSensorIndex(), DEC);    

    Serial.print("  Sensor Type: ");
    Serial.println(rflink.getSensorType());

    Serial.print("  Sensor Units: ");
    Serial.println(rflink.getSensorUnits());

    Serial.print("  Sensor Value: ");
    Serial.println(rflink.getSensorValue());

    Serial.println();
}

void printlnMAC(uint8_t * mac){
    for(uint8_t i = 0; i < 6; i++){
        if(mac[i] < 10) Serial.print("0");
        Serial.print(mac[i], HEX);
        if(i != 5){        
            Serial.print(":");
        }
    }
    Serial.println();
}
