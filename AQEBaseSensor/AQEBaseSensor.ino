#include <Wire.h>
#include <EggBus.h>
#include <NanodeMAC.h>
#include <SoftReset.h>
#include <avr/wdt.h>

#define SENSOR_PACKET_DELAY  5000L
#define FORCED_RESET_TIME_MS 600000L

#define TRANSMIT_STATE_SEND_TEMPERATURE 1
#define TRANSMIT_STATE_SEND_HUMIDITY    2
#define TRANSMIT_STATE_POLL_EGG_BUS     3
#define TRANSMIT_STATE_WAITING          4
static uint8_t transmit_state = TRANSMIT_STATE_WAITING;  
uint8_t eggbus_sensor_index     = 0;

// read your MAC address
static uint8_t mymac[6] = {0,0,0,0,0,0};
NanodeMAC mac(mymac);
EggBus eggBus;

#define SEND_RAW        0
#define SEND_CALCULATED 1
#define SEND_R0         2
byte send_type = SEND_RAW;

void printMAC(uint8_t * mac);

// support variables
long cosmPostPreviousMillis = 0;
long cosmPostInterval = 150000L;
long cosmDelayPreviousMillis = 0;
long cosmDelayInterval = 5000L;
long heartbeatPreviousMillis = 0;
long heartbeatInterval = 500L;
byte need_to_send = 0;
byte clear_to_send = 1;

void setup(){
    randomSeed(analogRead(0));

    Serial.begin(115200);
    Serial.println(F("\n[Air Quality Egg - Base Sensor - v2.03]"));
    Serial.print("Unit Address: ");
    printlnMAC(mymac);    
    cosmPostPreviousMillis = 71243411;
    
    setupNanode();
        
}  

void loop(){
    unsigned long currentMillis = millis(); 
    
    loopNanode();

    if(currentMillis - cosmPostPreviousMillis > cosmPostInterval) {
        if(transmit_state != TRANSMIT_STATE_WAITING){
          Serial.println(F("Something is taking longer than expected, Resetting"));
          Serial.flush();
          delay(1000);
          soft_restart(); 
        }
        
        Serial.println(F("Kicking off transmit cycle"));
        cosmPostPreviousMillis = currentMillis;              
        need_to_send = 0;   
        transmit_state = TRANSMIT_STATE_SEND_TEMPERATURE;
        
        eggBus.init();        
    }
    
    if(currentMillis - heartbeatPreviousMillis > heartbeatInterval){
      //Serial.print(transmit_state);
      heartbeatPreviousMillis = currentMillis;
    }    
    
    if(need_to_send == 0){
      switch(transmit_state){
      case TRANSMIT_STATE_SEND_TEMPERATURE:
        sendTemperature();
        need_to_send = 1; 
        Serial.print(millis()); 
        Serial.println(F(" Sent Temperature"));
        transmit_state = TRANSMIT_STATE_SEND_HUMIDITY;
        break;
      case TRANSMIT_STATE_SEND_HUMIDITY:
        sendHumidity();
        need_to_send = 1;          
        eggbus_sensor_index = 0;
        Serial.print(millis());         
        Serial.println(F(" Sent Humidity"));        
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
            sendEggBus(SEND_RAW); 
            Serial.print(millis());             
            Serial.print(F(" Sent Egg Bus Raw :: "));            
            Serial.println(eggBus.getSensorType(eggbus_sensor_index));
            send_type = SEND_R0;          
          }
          else if(SEND_R0 == send_type){
            sendEggBus(SEND_R0); 
            Serial.print(millis());             
            Serial.print(F(" Sent Egg Bus R0 :: "));            
            Serial.println(eggBus.getSensorType(eggbus_sensor_index));
            send_type = SEND_CALCULATED;             
          }
          else{
            sendEggBus(SEND_CALCULATED); 
            Serial.print(millis());             
            Serial.print(F(" Sent Egg Bus Calculated :: "));            
            Serial.println(eggBus.getSensorType(eggbus_sensor_index));            
            send_type = SEND_RAW;
            eggbus_sensor_index++;            
          }
          
          need_to_send = 1;          
        }
        else if(eggBus.next()){ // there are more sensors on the bus
          eggbus_sensor_index = 0;
          need_to_send = 1;    
        }
        else{ // there are no sensors left on the bus
          need_to_send = 0;
          transmit_state = TRANSMIT_STATE_WAITING;
        }
        break;
      case TRANSMIT_STATE_WAITING:
        // reset after about 10 minutes
        if(currentMillis > FORCED_RESET_TIME_MS){
           Serial.println(F("Time for our regularly scheduled Restart!"));
           delay(1000);
           soft_restart(); 
        }
        
        // nothing else to do here ...
        break;
      default: 
        Serial.print(F("Transmit State = "));
        Serial.print(transmit_state);
        Serial.println(F("... Inconceivable!"));
        break;    
      }
    }
    

    // clear to send condition
    if((clear_to_send == 0) && (currentMillis - cosmDelayPreviousMillis > cosmDelayInterval)){        
      clear_to_send = 1;
      cosmDelayPreviousMillis = currentMillis;
    }  

    if(clear_to_send && need_to_send){
      
      // transmit the packet
      postSensorData();  
      Serial.print(millis());       
      Serial.println(F(" Posted Sensor Data"));
      Serial.println();
      need_to_send = 0;
      clear_to_send = 0;
      cosmDelayPreviousMillis = currentMillis;      
    }     
    
    if(haventHeardFromCosmLately()){
      Serial.println(F("Too Long Since Last Response From Cosm, Resetting"));
      delay(1000);
      soft_restart(); 
    }    
    
    checkCosmReply();    
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
