

//RESET function is at the bottom

void setupLeds(){
  pinMode(LedRunning, OUTPUT);
  pinMode(buttonLed, OUTPUT);

  for(int i=0; i<5; i++){
    digitalWrite(LedRunning, HIGH);
    digitalWrite(buttonLed, LOW);
    delay(150);
    digitalWrite(LedRunning, LOW);
    digitalWrite(buttonLed, HIGH);
    delay(150); 
  }
  digitalWrite(LedRunning, LOW); 
  digitalWrite(buttonLed, LOW); 
}

void breatheRunningLed(){
  for(int i=0; i<175; i++){
    analogWrite(LedRunning, i);
    delay(3);
  }
  for(int i=174; i>0; i--){
    analogWrite(LedRunning, i);
    delay(3);
  }
  analogWrite(LedRunning, 0);
}

void flashSending(){
  //pinMode(buttonLed, OUTPUT);
  for(int i=0; i<3; i++){
    digitalWrite(LedRunning, HIGH);
    //digitalWrite(buttonLed, HIGH);
    delay(250);
    digitalWrite(LedRunning, LOW);
    //digitalWrite(buttonLed, LOW);
    delay(250); 
  }
}

/*
void setButtonLed(int currButton){
 
 digitalWrite(buttonLed, currButton);
}
 
 
 void flashErrorLed(){
 pinMode(buttonLed, OUTPUT);
 pinMode(LedRunning, OUTPUT);
 for(int i=0; i<12; i++){
 digitalWrite(buttonLed, HIGH);
 digitalWrite(LedRunning, HIGH);
 delay(100);
 digitalWrite(buttonLed, LOW);
 digitalWrite(LedRunning, LOW);
 delay(100); 
 }
 }
 
 
 void softwareReset(){
 //this function will pull the RESET PIN to LOW, causing the entire Arduino
 //and Ethernet Shield to reset themselves. 
 Serial.println("Resetting board");
 lcdResetNow();
 digitalWrite(softwareResetPin, LOW);
 }
 */

