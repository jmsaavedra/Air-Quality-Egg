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

#ifndef _EGG_BUS_LIB_H
#define _EGG_BUS_LIB_H

#if ARDUINO >= 100
  #include <Arduino.h>  // Arduino 1.0
#else
  #include <WProgram.h> // Arduino 0022
#endif

#include <stdint.h>

#define  MAX_RESPONSE_LENGTH            (16)  
#define  CMD_READ                       (0x11)
#define  CMD_WRITE                      (0x33)

// BASE ADDRESSES
#define METADATA_BASE_OFFSET             (0)
#define SENSOR_DATA_BASE_OFFSET          (32)
#define SENSOR_DATA_ADDRESS_BLOCK_SIZE   (256)      
#define DEBUG_BASE_OFFSET                (65408)

// METADATA FIELD OFFSETS
#define METADATA_SENSOR_COUNT_FIELD_OFFSET (0)
#define METADATA_MODULE_ID_FIELD_OFFSET    (1)
#define METADATA_VERSION_FIELD_OFFSET      (7)

// SENSOR DATA FIELD OFFSETS
#define SENSOR_TYPE_FIELD_OFFSET                  (0)
#define SENSOR_UNITS_FIELD_OFFSET                 (16)
#define SENSOR_R0_FIELD_OFFSET                    (32)
#define SENSOR_MEASURED_INDEPENDENT_OFFSET        (36)
#define SENSOR_TABLE_X_SCALER_OFFSET              (40)
#define SENSOR_RAW_VALUE_FIELD_OFFSET             (44)
#define SENSOR_TABLE_Y_SCALER_OFFSET              (48)
#define SENSOR_INDEPENDENT_SCALER_OFFSET          (52)
#define SENSOR_COMPUTED_MAPPING_TABLE_OFFSET      (56)
#define SENSOR_COMPUTED_MAPPING_TABLE_ROW_SIZE     (8)
#define SENSOR_COMPUTED_MAPPING_TABLE_TERMINATOR (0xff)

// DEBUG DATA FIELD OFFSETS
#define DEBUG_NO2_HEATER_V_PLUS              (0)
#define DEBUG_NO2_HEATER_V_MINUS             (4)
#define DEBUG_NO2_HEATER_POWER_MW            (8)
#define DEBUG_NO2_DIGIPOT_VALUE             (12)
#define DEBUG_CO_HEATER_V_PLUS              (16)
#define DEBUG_CO_HEATER_V_MINUS             (20)
#define DEBUG_CO_HEATER_POWER_MW            (24)
#define DEBUG_CO_DIGIPOT_VALUE              (28)
#define DEBUG_DIGIPOT_STATUS                (32)

class EggBus {
 private:
  uint8_t currentBusNumber;   // 0, 1, 2 for the three busses implied by the I2C Mux
  uint8_t currentBusAddress;  // 1 .. 127 (0 is reserved on I2C for "general call"
  uint8_t buffer[16];         // storage space for the current address and strings
  
  void i2cGetValue(uint8_t slave_address, uint16_t register_address, uint8_t response_length);
  void i2cWriteRegister(uint8_t slave_address, uint8_t * data_to_write, uint8_t num_bytes);
  void i2cWriteAddressRegister(uint8_t slave_address, uint16_t register_address);
  void i2cReadRegisterValue(uint8_t slave_address, uint8_t * buf, uint8_t response_length);
  uint8_t high_byte(uint16_t value);
  uint8_t low_byte(uint16_t value);  
  uint32_t buf_to_value(uint8_t * buf);  
  float buf_to_fvalue(uint8_t * buf);
  void clearBus();
  void i2cBusSwitch(uint8_t busNumber);
  
 public:
  EggBus();
  void init();
  uint8_t next(); 
  uint8_t * getSensorAddress();
  uint8_t getNumSensors();
  uint8_t getFirmwareVersion();
  char * getSensorType(uint8_t sensorIndex);
  float getSensorValue(uint8_t sensorIndex);
  char * getSensorUnits(uint8_t sensorIndex);
  uint32_t getSensorIndependentVariableMeasure(uint8_t sensorIndex);
  float getTableXScaler(uint8_t sensorIndex);  
  float getTableYScaler(uint8_t sensorIndex);
  float getIndependentScaler(uint8_t sensorIndex);
  bool getTableRow(uint8_t sensorIndex, uint8_t row_number, uint8_t * xval, uint8_t *y_val); 
  uint32_t getSensorR0(uint8_t sensorIndex);  
  void setSensorR0(uint8_t sensorIndex, uint32_t value);
};

#endif /*_EGG_BUS_LIB_H */
