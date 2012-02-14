
//--------- MQ-7 CO gas sensor setup ---------
//#include <CS_MQ7.h>
//CS_MQ7 MQ7(4, 10);  
// 4 = digital Pin connected to "tog" from sensor board
// 3 = digital Pin connected to LED Power Cycle Indicator

//--------- temperature sensor setup ---------
#include <Wire.h>
int tmp102Address = 0x48; //2-wire address

//time in milliseconds between each analog sensor reading
const int sensorDebounceTime = 15;

void sensorsSetup(){
  Wire.begin(); 
  if(debug) Serial.println("--- sensorSetup complete");
}

//--------- update loop ---------
void updateSensors(){

 // MQ7.CoPwrCycler(); //for CO sensor power cycling

  //sensors
  currNo2 = getNo2();
  delay(sensorDebounceTime);
  currCo = getCO();
  delay(sensorDebounceTime);
  currQuality = getQuality();
  delay(sensorDebounceTime);
  currHumidity = getHumidity();
  delay(sensorDebounceTime);
  currTemp = getTemperature();
  delay(sensorDebounceTime);

  //button
  //currButton = getButton(); 
  delay(sensorDebounceTime);

  if (debug) {
    Serial.println("//--- sensor reading ---//");
    Serial.print("currNo2   = ");
    Serial.println(currNo2);
    Serial.print("currCO    = ");
    Serial.println(currCo);
    Serial.print("currQual  = ");
    Serial.println(currQuality);
    Serial.print("currHum   = ");
    Serial.println(currHumidity);
    Serial.print("currTemp  = ");
    Serial.println(currTemp);
    Serial.print("currButton  = ");
    Serial.println(currButton);
    Serial.println("//---- end  reading ----//");
  }
}

//--------- e2v MiCS-2170 NO2 sensor ---------
int getNo2(){
  int thisReading;

  //set reference voltage to 3.3 here?
  thisReading = analogRead(No2SensorPin);
  return thisReading;
}

//--------- MQ-7 carbon monixide sensor ---------
//breakout info: http://citizensensor.cc/make
int getCO(){
  int thisReading = 1;
/*
  if (MQ7.currentState() == LOW){ //not heating, ready to read.
    thisReading = analogRead(CoSensorPin);
  } 
  else { //heating, leave value where it was
    thisReading = currCo;
  }
 */ 
  return thisReading;
}

//--------- MQ-135 air quality sensor ---------
int getQuality(){
  int thisReading;

  thisReading = analogRead(qualitySensorPin);
  return thisReading;
}

//--------- Sparkfun humidity sensor ---------
int getHumidity(){
  int thisReading;

  thisReading = analogRead(humiditySensorPin);
  return thisReading;
}

//--------- Sparkfun i2c temperature sensor ---------
int getTemperature(){
  int thisReading;
  Wire.requestFrom(tmp102Address,2); 

  byte MSB = Wire.read();
  byte LSB = Wire.read();

  //it's a 12bit int, using two's compliment for negative
  int TemperatureSum = ((MSB << 8) | LSB) >> 4; 

  float celsius = TemperatureSum*0.0625;

  thisReading = int(celsius);

  return thisReading;
}





