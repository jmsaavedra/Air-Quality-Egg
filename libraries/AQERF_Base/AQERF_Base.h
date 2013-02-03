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

#ifndef _AQERF_BASE_LIB_H
#define _AQERF_BASE_LIB_H

#if ARDUINO >= 100
  #include <Arduino.h>  // Arduino 1.0
#else
  #include <WProgram.h> // Arduino 0022
#endif

#include <stdint.h>
#include "../JeeLib/RF12.h"

class AQERF_Base {
 private:
    uint8_t packet[RF12_MAXDATA];
    uint8_t base_station_address[6];
    uint32_t end_time;
    int32_t previous_time;
    uint8_t need_to_send;
    void (*pairingRxCallback)(uint8_t *);
 public:
    AQERF_Base(uint8_t * mac);
    void setPairingRxCallback(void (*fp)(uint8_t *));
    boolean pair(void);
    void pairInit(void);
    uint8_t dataReceived(void);
    uint8_t getPacketType(void);
    uint16_t getRemoteFirmwareVersion(void);
    uint8_t * getRemoteStationAddress(void);
    uint8_t * getSourceSensorAddress(void);
    uint8_t getSensorIndex(void);
    char * getSensorType(void);
    char * getSensorUnits(void);
    int32_t getSensorValue(void);
};

#endif /* _AQERF_BASE_LIB_H */

