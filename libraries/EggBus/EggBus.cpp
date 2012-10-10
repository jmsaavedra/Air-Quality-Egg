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

#include "EggBus.h"

#include "../Wire/Wire.h"
extern "C" { 
#include "../Wire/utility/twi.h"  // from Wire library, so we can do bus scanning
}

EggBus::EggBus(){
  Wire.begin();
  init();
}

void EggBus::init(){
  currentBusNumber  = 0; 
  currentBusAddress = 0;
}

/*
  next returns 0 if there are no more devices found on the bus
  otherwise it returns the address of the next device found on the bus
  which will always be a number between 1 and 127  
*/
uint8_t EggBus::next(){
  uint8_t rc = 0xff;
  uint8_t data = 0; // not used, just an address to feed to twi_writeTo()
  for( uint8_t busNumber = currentBusNumber; busNumber < 3; busNumber++ ){  
    //switch I2C Mux to the requisit bus number
    clearBus(); 
    i2cBusSwitch(busNumber);
    
    for( uint8_t addr = currentBusAddress + 1; addr <= 127; addr++ ) {      
      if(addr == 0x70) continue; // this is the I2C Mux skip it
      
      rc = twi_writeTo(addr, &data, 0, 1, 0);
      if(rc == 0){
        // device found
        currentBusNumber  = busNumber;
        currentBusAddress = addr;
        // read something arbitrarily from the device
        // this is a bug I haven't figured out - but the first read always
        // seems to fail after the polling mechanism, so clear the bus here (by doing a normal read)
        clearBus();
        return addr;
      }
    }
    //Serial.print("No more devices found on bus #");
    //Serial.println(busNumber, DEC);    
  }
  return 0;
}

/*
  gets the sensor address as a byte array pointer
  the pointer is only valid until another 
  call that overwrites it (e.g. getSensor*)  
*/
uint8_t * EggBus::getSensorAddress(){
  i2cGetValue(currentBusAddress, METADATA_BASE_OFFSET + METADATA_MODULE_ID_FIELD_OFFSET, 6);
  return buffer;
}

/*
  gets the number of sensors
*/  
uint8_t EggBus::getNumSensors(){
  i2cGetValue(currentBusAddress, METADATA_BASE_OFFSET + METADATA_SENSOR_COUNT_FIELD_OFFSET, 1);
  return buffer[0];
}

/*
  gets the sensor type of the index as a string
  the pointer is only valid until another 
  call that overwrites it (e.g. getSensor*)
*/
char * EggBus::getSensorType(uint8_t sensorIndex){
  i2cGetValue(currentBusAddress, SENSOR_DATA_BASE_OFFSET + sensorIndex * SENSOR_DATA_ADDRESS_BLOCK_SIZE + SENSOR_TYPE_FIELD_OFFSET, 16);
  return (char *) buffer;
}
  
/*
  returns the sensor value of the index as a 32-bit integer
*/
uint32_t EggBus::getSensorValue(uint8_t sensorIndex){
  i2cGetValue(currentBusAddress, SENSOR_DATA_BASE_OFFSET + sensorIndex * SENSOR_DATA_ADDRESS_BLOCK_SIZE + SENSOR_RAW_VALUE_SENSED_RESISTANCE_OFFSET, 4);
  return buf_to_value(buffer);
}

/*
  gets the sensor units of the index as a string
  the pointer is only valid until another 
  call that overwrites it (e.g. getSensor*)
*/
char * EggBus::getSensorUnits(uint8_t sensorIndex){
  i2cGetValue(currentBusAddress, SENSOR_DATA_BASE_OFFSET + sensorIndex * SENSOR_DATA_ADDRESS_BLOCK_SIZE + SENSOR_UNITS_FIELD_OFFSET, 16);
  return (char *) buffer;
}


void EggBus::i2cWriteAddressRegister(uint8_t slave_address, uint16_t register_address){
  
  /*
    In order to read a value from the Memory Map, the Nanode 
    must write the target Sensor Module’s I2C address to bus with the Write bit set to 0 (SLA+W), 
    then write the “READ” command to the bus (0x11), 
    then write the address to be read to the bus (high byte then low byte), 
    and finally an I2C stop condition. 
  */  
  
  Wire.beginTransmission(slave_address); 
  Wire.write(CMD_READ);                        // sends READ command
  Wire.write(high_byte(register_address));     // sends register address high byte
  Wire.write(low_byte(register_address));      // sends register address low byte  
  Wire.endTransmission();                      // stop transmitting    
}

void EggBus::i2cReadRegisterValue(uint8_t slave_address, uint8_t * buf, uint8_t response_length){
  
  /* 
    The Nanode then writes the Sensor Module’s I2C address to bus with the Write bit set to 1 (SLA+R), 
    and clocks in the expected number of bytes as expected based on the address requested, 
    and finally issues an I2C stop condition.  
  */
  
  uint8_t index = 0;
  Wire.requestFrom(slave_address, response_length);   
  while(Wire.available()){    // slave may send less than requested
    buf[index++] = Wire.read();
  }  
}

void EggBus::i2cGetValue(uint8_t slave_address, uint16_t register_address, uint8_t response_length){
  i2cWriteAddressRegister(slave_address, register_address);
  delay(10); // this is definitely necessary (though shorter may be ok too)
  i2cReadRegisterValue(slave_address, buffer, response_length);  
  delay(10); // this may not be necessary
}

uint8_t EggBus::high_byte(uint16_t value){
  return ((value >> 8) & 0xff);
}

uint8_t EggBus::low_byte(uint16_t value){
  return (value & 0xff); 
}

uint32_t EggBus::buf_to_value(uint8_t * buf){
  uint8_t index = 1;
  uint32_t ret = buf[0];  
  for(index = 1; index < 4; index++){
    ret = (ret << 8);         // make space for the next byte
    ret = (ret | buf[index]); // slide in the next byte
  }
  
  return ret;
}

void EggBus::clearBus(){
  i2cGetValue(currentBusAddress, METADATA_BASE_OFFSET + METADATA_SENSOR_COUNT_FIELD_OFFSET, 1);
}

void EggBus::i2cBusSwitch(uint8_t busNumber){
  uint8_t ctrl_reg = 0;
  if(busNumber == 1){
    ctrl_reg = 4;
  }
  else if(busNumber == 2){
    ctrl_reg = 5;
  }
  
  Wire.beginTransmission(0x70); // this is the address of the I2C Mux
  Wire.write(ctrl_reg);                        
  Wire.endTransmission();          

  // you could theoretically read back the value to make sure it took at this point
  // and if not... maybe try again or something
}

void EggBus::getRawValue(uint8_t sensor_index, uint32_t * adc_result, uint32_t * low_side_resistance){
  i2cGetValue(currentBusAddress, SENSOR_DATA_BASE_OFFSET + sensor_index * SENSOR_DATA_ADDRESS_BLOCK_SIZE + SENSOR_RAW_VALUE_FIELD_OFFSET, 8);
  *adc_result = buf_to_value(buffer);
  *low_side_resistance = buf_to_value(buffer + 4);
}

