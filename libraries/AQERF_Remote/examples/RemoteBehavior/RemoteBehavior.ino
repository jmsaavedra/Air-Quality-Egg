#include <AQERF_Remote.h>
#include <RF12.h>
#include <NanodeMAC.h>

// read your MAC address
static uint8_t mymac[6] = {0,0,0,0,0,0};
NanodeMAC mac(mymac);

// create an AQERF_Base object to handle the RF Link
// informing it what the unit MAC address is
AQERF_Remote rflink(mymac); 

void printMAC(uint8_t * mac);

// support variables
long previous_millis = 0;
int need_to_send = 0;

// dummy data
uint32_t sensor_value = 0;
uint8_t  sensor_address[6] = {0xde, 0xfa, 0xce, 0xbe, 0xad, 0xed};
char     sensor_type[16] =   "abc def gef...!";
char     sensor_units[16] =  "ijk lmn opq...!";

void setup(){
    Serial.begin(9600);
    Serial.println("AQE Remote RF Unit Test");
    Serial.print("Unit Address: ");
    printlnMAC(mymac);
    Serial.print("Last Paired Base: ");
    printlnMAC(rflink.getBaseStationAddress());

    for(;;){ // until some condition results in a break
        if(rflink.pair()){
            Serial.println("Pairing Successful");
            Serial.print("Base Station Address: ");
            printlnMAC(rflink.getBaseStationAddress());

            // after pairing succeeds, ensure that the pairing phase is complete before 
            // proceeding to runtime behavior (i.e. loop)
            delay(AQERF_PAIRING_DURATION_MS); 

            break; // go to loop
        }
        else{
            Serial.println("Pairing Failed");
            // check to see if we already know who are base station is from EEPROM
            // if we do, then use it and proceed to runtime behavior (i.e. loop)
            if(rflink.previouslyPaired()){
                break;
            }

            // otherwise fall through and re-attempt pairing
        }
    }
}

void loop(){
    unsigned long currentMillis = millis();

    if(currentMillis - previous_millis > rflink.getTransmitInterval()) {
        previous_millis = currentMillis; 

        // construct the packet
        rflink.setPacketType(AQERF_PACKET_TYPE_REMOTE_STATION_DATUM);
        rflink.setRemoteFirmwareVersion(0x1234);
        rflink.setRemoteStationAddress(mymac);
        rflink.setSourceSensorAddress(sensor_address);
        rflink.setSensorIndex(33);
        rflink.setSensorType(sensor_type);
        rflink.setSensorUnits(sensor_units);
        rflink.setSensorValue(sensor_value);

        need_to_send = 1;
    }

    if(rflink.clearToSend() && need_to_send){
        
        // transmit the packet
        rflink.transmit();
    
        need_to_send = 0;
        Serial.print("Transmitted Packet :: Sensor Value = ");
        Serial.println(sensor_value++);
    }     
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
