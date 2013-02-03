/* Copyright (C) 2012 by Victor Aprea <victor.aprea@wickeddevice.com>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#include "AQERF_Base.h"

// Packet Types
#define AQERF_PACKET_TYPE_BASE_STATION_ADVERTISEMENT 0x55
#define AQERF_PACKET_TYPE_REMOTE_STATION_DATUM       0x33
#define AQERF_PACKET_TYPE_OFFSET                     0

// Offset Definitions for PACKET_TYPE_BASE_STATION_ADVERTISEMENT
#define AQERF_BASE_STATION_ADVERTISEMENT_PACKET_LENGTH 7
#define AQERF_BASE_ADDRESS_OFFSET                      1
#define AQERF_BASE_ADDRESS_LENGTH                      6

// Offset Definitions for PACKET_TYPE_REMOTE_STATION_DATUM
#define AQERF_REMOTE_STATION_DATUM_PACKET_LENGTH     58

#define AQERF_REMOTE_FIRMWARE_VERSION_OFFSET         1
#define AQERF_REMOTE_FIRMWARE_VERSION_LENGTH         2

#define AQERF_DESTINATION_BASE_ADDRESS_OFFSET        3
#define AQERF_DESTINATION_BASE_ADDRESS_LENGTH        6

#define AQERF_REMOTE_STATION_ADDRESS_OFFSET          9
#define AQERF_REMOTE_STATION_ADDRESS_LENGTH          6

#define AQERF_SOURCE_SENSOR_ADDRESS_OFFSET           15
#define AQERF_SOURCE_SENSOR_ADDRESS_LENGTH           6

#define AQERF_SENSOR_INDEX_OFFSET                    21
#define AQERF_SENSOR_INDEX_LENGTH                    1

#define AQERF_SENSOR_TYPE_OFFSET                     22
#define AQERF_SENSOR_TYPE_LENGTH                     16

#define AQERF_SENSOR_UNITS_OFFSET                    38
#define AQERF_SENSOR_UNITS_LENGTH                    16

#define AQERF_SENSOR_VALUE_OFFSET                    54
#define AQERF_SENSOR_VALUE_LENGTH                    4

/*
typedef struct{
    uint8_t  packet_type;
    uint16_t remote_station_firmware_version;
    uint8_t  destination_base_address[6];
    uint8_t  remote_station_address[6];
    uint8_t  source_sensor_address[6];
    uint8_t  sensor_index;
    char     sensor_type[16];
    char     sensor_units[16];
    uint32_t sensor_value;
} aqerf_datum_packet_t;

typedef struct{
    uint8_t packet_type;
    uint8_t base_station_address[6];
} aqerf_base_station_advertisement_packet_t;
*/

AQERF_Base::AQERF_Base(uint8_t * mac){
    pairingRxCallback = 0;
    memcpy(base_station_address, mac, 6);
    rf12_initialize('A', RF12_433MHZ); // 'A' is arbitrary, doesn't matter for broadcast
}

#define PAIRING_DURATION_MS            10000L
#define PAIRING_BROADCAST_INTERVAL_MS  2000L

void AQERF_Base::pairInit(void){
    uint32_t current_time = millis();
    previous_time = 0;
    end_time = current_time + PAIRING_DURATION_MS;    
    need_to_send = 1;

    packet[AQERF_PACKET_TYPE_OFFSET] = AQERF_PACKET_TYPE_BASE_STATION_ADVERTISEMENT;
    memcpy(packet + AQERF_BASE_ADDRESS_OFFSET, base_station_address, AQERF_BASE_ADDRESS_LENGTH);
}

void AQERF_Base::setPairingRxCallback(void (*fp)(uint8_t *)){
    pairingRxCallback = fp;
}

boolean AQERF_Base::pair(){
    uint32_t current_time = millis();
    boolean pairing_done = false;
    
    if (rf12_recvDone()) {   // incoming data is present
        if(0 == rf12_crc){   // otherwise the data is unreliable                    
            if(pairingRxCallback != 0){
                pairingRxCallback(packet);
            }
        }
    }    
    
    if(current_time < end_time){       
        if(current_time - previous_time > PAIRING_BROADCAST_INTERVAL_MS) {
            previous_time = current_time; 
            need_to_send = 1;
        }

        if(need_to_send && rf12_canSend()){
            rf12_sendStart(0, packet, AQERF_BASE_STATION_ADVERTISEMENT_PACKET_LENGTH);
            need_to_send = 0;
        }

        pairing_done = false;
    }       
    else{
        pairing_done = true;
    }

    return pairing_done;
}

// this function must be called frequently in order to keep the RF12 state machine moving
uint8_t AQERF_Base::dataReceived(void){
    uint8_t ret = 0;
    if (rf12_recvDone()) {   // incoming data is present
        if(0 == rf12_crc){   // otherwise the data is unreliable
            if(AQERF_PACKET_TYPE_REMOTE_STATION_DATUM == rf12_data[AQERF_PACKET_TYPE_OFFSET]){ // if its the right type of packet
                if(0 == memcmp(base_station_address, (const void*) (rf12_data + AQERF_DESTINATION_BASE_ADDRESS_OFFSET), 6)){ // if its addressed to me
                    // copy the data into the packet buffer
                    memcpy(packet, (const void *)rf12_data, AQERF_REMOTE_STATION_DATUM_PACKET_LENGTH);              
                    ret = 1;
                }
            }
        }
    }
        
    return ret;
}

// these functions are only valid immediately after dataReceived returns 1
uint8_t AQERF_Base::getPacketType(void){
    return packet[AQERF_PACKET_TYPE_OFFSET];
}

uint16_t AQERF_Base::getRemoteFirmwareVersion(void){
    uint16_t ret = (uint16_t) packet[AQERF_REMOTE_FIRMWARE_VERSION_OFFSET];
    ret <<= 8;    
    ret |= packet[AQERF_REMOTE_FIRMWARE_VERSION_OFFSET + 1];
    return ret;
}

uint8_t * AQERF_Base::getRemoteStationAddress(void){
    return packet + AQERF_REMOTE_STATION_ADDRESS_OFFSET;
}

uint8_t * AQERF_Base::getSourceSensorAddress(void){
    return packet + AQERF_SOURCE_SENSOR_ADDRESS_OFFSET;
}

uint8_t AQERF_Base::getSensorIndex(void){
    return packet[AQERF_SENSOR_INDEX_OFFSET];
}

char * AQERF_Base::getSensorType(void){
    return ((char *) (packet + AQERF_SENSOR_TYPE_OFFSET));
}

char * AQERF_Base::getSensorUnits(void){
    return ((char *) (packet + AQERF_SENSOR_UNITS_OFFSET));
}

int32_t AQERF_Base::getSensorValue(void){
    int32_t ret = packet[AQERF_SENSOR_VALUE_OFFSET];
    for(uint8_t ii = 1; ii < 4; ii++){
        ret <<= 8;
        ret |= packet[AQERF_SENSOR_VALUE_OFFSET + ii];
    }
    return ret;
}

