#include <Wire.h>
#include <EggBus.h>
#include <AQERF_Remote.h>
#include <RF12.h>
#include <NanodeMAC.h>
#include <SoftReset.h>
#include <avr/wdt.h>

#define SENSOR_PACKET_DELAY 5000L
#define TRANSMIT_STATE_SEND_TEMPERATURE 1
#define TRANSMIT_STATE_SEND_HUMIDITY    2
#define TRANSMIT_STATE_POLL_EGG_BUS     3
#define TRANSMIT_STATE_WAITING          4
static uint8_t transmit_state = TRANSMIT_STATE_WAITING;
char sensor_type_temperature[]  = "Temperature";
char sensor_units_temperature[] = "deg C";
char sensor_type_humidity[]     = "Humidity";   
char sensor_units_humidity[]    = "%";   
uint8_t eggbus_sensor_index     = 0;
char feed_name[32] = {0}; 

// read your MAC address
static uint8_t mymac[6] = {0,0,0,0,0,0};
NanodeMAC mac(mymac);
EggBus eggBus;

#define SEND_RAW        0
#define SEND_CALCULATED 1
byte send_type = SEND_RAW;

// create an AQERF_Base object to handle the RF Link
// informing it what the unit MAC address is
AQERF_Remote rflink(mymac); 

void printMAC(uint8_t * mac);

// support variables
long previous_millis = 0;
int need_to_send = 0;

void setup(){
    randomSeed(analogRead(0));

    Serial.begin(115200);
    Serial.println(F("\n[Air Quality Egg - Remote - v1.02]"));
    Serial.print(F("Unit Address: "));
    printlnMAC(mymac);
    Serial.print(F("Last Paired Base: "));
    printlnMAC(rflink.getBaseStationAddress());

    for(;;){ // until some condition results in a break
        if(rflink.pair()){
            Serial.println(F("Pairing Successful"));
            Serial.print(F("Base Station Address: "));
            printlnMAC(rflink.getBaseStationAddress());

            // after pairing succeeds, ensure that the pairing phase is complete before 
            // proceeding to runtime behavior (i.e. loop)
            delay(AQERF_PAIRING_DURATION_MS); 

            break; // go to loop
        }
        else{
            Serial.println(F("Pairing Failed"));
            // check to see if we already know who are base station is from EEPROM
            // if we do, then use it and proceed to runtime behavior (i.e. loop)
            if(rflink.previouslyPaired()){
                Serial.println(F("Assuming previously paired Base Station"));
                break;
            }
            else{
                // otherwise fall through and re-attempt pairing
                Serial.println(F("Retrying")); 
            }
        }
    }
    
    previous_millis = millis();
}  

void loop(){
    unsigned long currentMillis = millis(); 

    if(currentMillis - previous_millis > rflink.getTransmitInterval()) {
        if(transmit_state != TRANSMIT_STATE_WAITING){
          Serial.println(F("Something is taking longer than expected, Resetting"));
          Serial.flush();
          delay(1000);
          soft_restart(); 
        }
        
        
        previous_millis = currentMillis;              
        need_to_send = 0;   
        transmit_state = TRANSMIT_STATE_SEND_TEMPERATURE;
        
        eggBus.init();        
    }
    
    if(need_to_send == 0){
      switch(transmit_state){
      case TRANSMIT_STATE_SEND_TEMPERATURE:
        setupTemperaturePacket();
        need_to_send = 1;    
        transmit_state = TRANSMIT_STATE_SEND_HUMIDITY;
        break;
      case TRANSMIT_STATE_SEND_HUMIDITY:
        setupHumidityPacket();
        need_to_send = 1;    
        eggbus_sensor_index = 0;
        if(eggBus.next()){
          transmit_state = TRANSMIT_STATE_POLL_EGG_BUS;
        }
        else{
          transmit_state = TRANSMIT_STATE_WAITING;
        }
        break;
      case TRANSMIT_STATE_POLL_EGG_BUS:
        if(eggbus_sensor_index < eggBus.getNumSensors()){ // there are more sensors at the current address
          if(SEND_RAW == send_type){
            setupEggBusPacketRaw(); 
            send_type = SEND_CALCULATED;          
          }
          else{
            setupEggBusPacket();
            send_type = SEND_RAW;
            eggbus_sensor_index++;            
          }
          
          need_to_send = 1;          
        }
        else if(eggBus.next()){ // there are more sensors on the bus
          eggbus_sensor_index = 0;
          setupEggBusPacket();
          eggbus_sensor_index++;
          need_to_send = 1;    
        }
        else{ // there are no sensors left on the bus
          need_to_send = 0;
          transmit_state = TRANSMIT_STATE_WAITING;
        }
        break;
      case TRANSMIT_STATE_WAITING:
        // nothing to do here ...
        break;
      default: 
        Serial.print(F("Transmit State = "));
        Serial.print(transmit_state);
        Serial.println(F("... Inconceivable!"));
        break;    
      }
    }

    if(rflink.clearToSend() && need_to_send){
        
        // transmit the packet
        rflink.transmit();
    
        need_to_send = 0;
        Serial.print(F("Transmitted Packet: "));
        Serial.println(feed_name);
        
        delay(SENSOR_PACKET_DELAY);
    }     
}

void printlnMAC(uint8_t * mac){
    for(uint8_t i = 0; i < 6; i++){
        if(mac[i] < 10) Serial.print(F("0"));
        Serial.print(mac[i], HEX);
        if(i != 5){        
            Serial.print(F(":"));
        }
    }
    Serial.println();

}

void setupTemperaturePacket(){
    memcpy(feed_name, 0, 32);    
    strncpy(feed_name, sensor_type_temperature, 20);  
    rflink.setPacketType(AQERF_PACKET_TYPE_REMOTE_STATION_DATUM);
    rflink.setRemoteFirmwareVersion(1);
    rflink.setRemoteStationAddress(mymac);
    rflink.setSourceSensorAddress(mymac);
    rflink.setSensorIndex(0);
    rflink.setSensorType(sensor_type_temperature);
    rflink.setSensorUnits(sensor_units_temperature);
    rflink.setSensorValue(getTemperature()); 
}

void setupHumidityPacket(){
    memcpy(feed_name, 0, 32);    
    strncpy(feed_name, sensor_type_humidity, 20);  
    rflink.setPacketType(AQERF_PACKET_TYPE_REMOTE_STATION_DATUM);
    rflink.setRemoteFirmwareVersion(1);
    rflink.setRemoteStationAddress(mymac);
    rflink.setSourceSensorAddress(mymac);
    rflink.setSensorIndex(1);
    rflink.setSensorType(sensor_type_humidity);
    rflink.setSensorUnits(sensor_units_humidity);
    rflink.setSensorValue(getHumidity());  
}

void setupEggBusPacket(){
    memcpy(feed_name, 0, 32);    
    strncpy(feed_name, eggBus.getSensorType(eggbus_sensor_index), 20);
    rflink.setPacketType(AQERF_PACKET_TYPE_REMOTE_STATION_DATUM);
    rflink.setRemoteFirmwareVersion(eggBus.getFirmwareVersion());
    rflink.setRemoteStationAddress(mymac);
    rflink.setSourceSensorAddress(eggBus.getSensorAddress());
    rflink.setSensorIndex(eggbus_sensor_index);
    rflink.setSensorType(eggBus.getSensorType(eggbus_sensor_index));
    rflink.setSensorUnits(eggBus.getSensorUnits(eggbus_sensor_index));
    rflink.setSensorValue(eggBus.getSensorValue(eggbus_sensor_index));    
}

void setupEggBusPacketRaw(){ 
    char * ohms = "ohms";
    uint32_t measured_value = eggBus.getSensorIndependentVariableMeasure(eggbus_sensor_index);
    float i_scaler = eggBus.getIndependentScaler(eggbus_sensor_index);
    uint32_t r0 = eggBus.getSensorR0(eggbus_sensor_index);
    memcpy(feed_name, 0, 32);
    uint32_t resistance = 0xffffffff;    
    if(measured_value != 0xffffffff){
      resistance = (uint32_t) (measured_value * i_scaler * r0); 
    }
    
    strncpy(feed_name, eggBus.getSensorType(eggbus_sensor_index), 20);
    strcat(feed_name, "_raw");
    
    rflink.setPacketType(AQERF_PACKET_TYPE_REMOTE_STATION_DATUM);
    rflink.setRemoteFirmwareVersion(eggBus.getFirmwareVersion());
    rflink.setRemoteStationAddress(mymac);
    rflink.setSourceSensorAddress(eggBus.getSensorAddress());
    rflink.setSensorIndex(eggbus_sensor_index);
    rflink.setSensorType(feed_name);
    rflink.setSensorUnits(ohms);
    rflink.setSensorValue(resistance);         
}
