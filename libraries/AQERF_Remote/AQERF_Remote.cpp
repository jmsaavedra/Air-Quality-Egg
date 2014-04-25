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

#include "AQERF_Remote.h"
#include <string.h>
#include <avr/eeprom.h>

// Packet Types
#define AQERF_PACKET_TYPE_BASE_STATION_ADVERTISEMENT 0x55
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

#define AQERF_EEPROM_LAST_KNOWN_BASE_ADDRESS 42
AQERF_Remote::AQERF_Remote(uint8_t * mac){
    transmit_interval = AQERF_SENSOR_DATUM_TRANSMIT_INTERVAL;
    unit_address = mac;
    
    rf12_initialize('A', RF12_433MHZ);
    //Read the last known Base Address out of EEPROM into base_station_address
    eeprom_read_block(base_station_address, (const void *) AQERF_EEPROM_LAST_KNOWN_BASE_ADDRESS, 6);
    
}

uint8_t AQERF_Remote::pair(){
    // try this for a period of time up to the pairing duration
    uint8_t return_value = 0;
    uint8_t rx_base_station_address[6];   
    uint8_t comparison = 0; 
    eeprom_read_block(base_station_address, (const void *) AQERF_EEPROM_LAST_KNOWN_BASE_ADDRESS, 6);
    for(uint32_t ii = 0; ii < AQERF_PAIRING_DURATION_MS; ii++){
        if (rf12_recvDone()) {   // incoming data is present
            if(0 == rf12_crc){   // otherwise the data is unreliable
                if(AQERF_PACKET_TYPE_BASE_STATION_ADVERTISEMENT == rf12_data[AQERF_PACKET_TYPE_OFFSET]){
                    // we have a pairing ... 
                    //   store the advertised base address in RAM and EEPROM
                    memcpy(rx_base_station_address, (const void *) (rf12_data + AQERF_BASE_ADDRESS_OFFSET), AQERF_BASE_ADDRESS_LENGTH);
                    
                    // ping back to the base to let it know you are all set by echoing the packet with your address
                    memcpy(packet + AQERF_BASE_ADDRESS_OFFSET, unit_address, AQERF_BASE_ADDRESS_LENGTH);

                    delay(200);
                    
                    while(!rf12_canSend()){
                      rf12_recvDone();
                    }
                    
                    rf12_sendStart(0, packet, AQERF_BASE_STATION_ADVERTISEMENT_PACKET_LENGTH);
                    
                    //for(uint8_t jj = 0; jj < 100; jj++){
                    //  rf12_recvDone();  // ensures the packet gets sent                  
                    //}     
                    
                    Serial.println(F("Sent ACK"));
                    
                    //Store the base_station_address to EEPROM         
                    comparison = 1;
                    for(uint8_t jj = 0; jj < 6; jj++){
                      if(rx_base_station_address[jj] != base_station_address[jj]){
                        comparison = 0;
                      }
                    }   
                    
                    if(comparison == 0){     
                      Serial.println(F("Overwriting EEPROM"));
                      eeprom_write_block(rx_base_station_address, (void *) AQERF_EEPROM_LAST_KNOWN_BASE_ADDRESS, 6);
                      eeprom_read_block(base_station_address, (const void *) AQERF_EEPROM_LAST_KNOWN_BASE_ADDRESS, 6);                      
                    }
                     
                    return_value = 1;
                }
            }
        }
        delay(1);
    }
    
    return return_value;
}

// must be called often to keep the driver moving
uint8_t AQERF_Remote::clearToSend(void){
    rf12_recvDone(); 
    return rf12_canSend();
}

void AQERF_Remote::transmit(void){
    // put the base station address into the packet before sending
    memcpy(packet + AQERF_DESTINATION_BASE_ADDRESS_OFFSET, base_station_address, AQERF_DESTINATION_BASE_ADDRESS_LENGTH);
    rf12_sendStart(0, packet, AQERF_REMOTE_STATION_DATUM_PACKET_LENGTH);
}

uint8_t * AQERF_Remote::getBaseStationAddress(void){
    return base_station_address;
}



void AQERF_Remote::setPacketType(uint8_t packet_type){
    packet[AQERF_PACKET_TYPE_OFFSET] = packet_type;
}

void AQERF_Remote::setRemoteFirmwareVersion(uint16_t remote_firmware_version){
    packet[AQERF_REMOTE_FIRMWARE_VERSION_OFFSET]     = remote_firmware_version >> 8;
    packet[AQERF_REMOTE_FIRMWARE_VERSION_OFFSET + 1] = remote_firmware_version & 0xff;    
}

void AQERF_Remote::setRemoteStationAddress(uint8_t * remote_station_address){
    memcpy(packet + AQERF_REMOTE_STATION_ADDRESS_OFFSET, remote_station_address, AQERF_REMOTE_STATION_ADDRESS_LENGTH);
}

void AQERF_Remote::setSourceSensorAddress(uint8_t * sensor_address){
    memcpy(packet + AQERF_SOURCE_SENSOR_ADDRESS_OFFSET, sensor_address, AQERF_SOURCE_SENSOR_ADDRESS_LENGTH);
}

void AQERF_Remote::setSensorIndex(uint8_t sensor_index){
    packet[AQERF_SENSOR_INDEX_OFFSET] = sensor_index;
}

void AQERF_Remote::setSensorType(char * sensor_type){
    strncpy((char *) (packet + AQERF_SENSOR_TYPE_OFFSET), sensor_type, AQERF_SENSOR_TYPE_LENGTH - 1);
    packet[AQERF_SENSOR_TYPE_OFFSET + AQERF_SENSOR_TYPE_LENGTH - 1] = '\0'; // guarantee null terminated
}

void AQERF_Remote::setSensorUnits(char * sensor_units){
    strncpy((char *) (packet + AQERF_SENSOR_UNITS_OFFSET), sensor_units, AQERF_SENSOR_UNITS_LENGTH - 1);
    packet[AQERF_SENSOR_UNITS_OFFSET + AQERF_SENSOR_UNITS_LENGTH - 1] = '\0'; // guarantee null terminated
}

void AQERF_Remote::setSensorValue(int32_t sensor_value){        
    packet[AQERF_SENSOR_VALUE_OFFSET]     = (sensor_value >> 24) & 0xff;
    packet[AQERF_SENSOR_VALUE_OFFSET + 1] = (sensor_value >> 16) & 0xff;        
    packet[AQERF_SENSOR_VALUE_OFFSET + 2] = (sensor_value >>  8) & 0xff;        
    packet[AQERF_SENSOR_VALUE_OFFSET + 3] = (sensor_value >>  0) & 0xff;        
}

uint32_t AQERF_Remote::getTransmitInterval(void){
    return transmit_interval;
}

void AQERF_Remote::setTransmitInterval(uint32_t tx_interval){
    transmit_interval = tx_interval;
}

// this should only be called after pairing has been attempted at least once
uint8_t AQERF_Remote::previouslyPaired(void){
    const uint8_t uninitialized_eeprom[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if(0 == memcmp(base_station_address, uninitialized_eeprom, 6)){
        return 0;
    }
    return 1;
}
