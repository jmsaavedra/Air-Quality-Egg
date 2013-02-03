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

#ifndef _AQERF_REMOTE_LIB_H
#define _AQERF_REMOTE_LIB_H

#if ARDUINO >= 100
  #include <Arduino.h>  // Arduino 1.0
#else
  #include <WProgram.h> // Arduino 0022
#endif

#include <stdint.h>
#include "../JeeLib/RF12.h"

#define AQERF_PAIRING_DURATION_MS                    10000L
#define AQERF_SENSOR_DATUM_TRANSMIT_INTERVAL         60000L
#define AQERF_PACKET_TYPE_REMOTE_STATION_DATUM       0x33

class AQERF_Remote {
 private:
    uint8_t packet[RF12_MAXDATA];
    uint8_t base_station_address[6];
    uint8_t * unit_address;
    uint32_t transmit_interval;
 public:
    AQERF_Remote(uint8_t * mac);
    uint8_t pair();
    void transmit(void);
    uint8_t clearToSend(void);
    uint8_t * getBaseStationAddress(void);
    void setPacketType(uint8_t packet_type);
    void setRemoteFirmwareVersion(uint16_t remote_firmware_version);
    void setRemoteStationAddress(uint8_t * remote_station_address);
    void setSourceSensorAddress(uint8_t * sensor_address);
    void setSensorIndex(uint8_t sensor_index);
    void setSensorType(char * sensor_type);
    void setSensorUnits(char * sensor_units);
    void setSensorValue(int32_t sensor_value);
    uint32_t getTransmitInterval(void);
    void setTransmitInterval(uint32_t tx_interval);
    uint8_t previouslyPaired(void);
};

#endif /* _AQERF_REMOTE_LIB_H */

