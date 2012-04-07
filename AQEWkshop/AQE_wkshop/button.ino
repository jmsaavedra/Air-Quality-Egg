
/* experimental button stuff*/

int currButtonVal = 0;
int prevButtonVal = 1;
//int myDigitalVal;

void buttonUpdate(){
  //--- button and sensor reads
  currButtonVal = digitalRead(buttonPin);  //read our button
  if(currButtonVal != prevButtonVal){ //check for state change
    prevButtonVal = currButtonVal;
    if(prevButtonVal == 1){ 
      currButton ++ ;           //this for if we want to add a counter later on
      if(currButton > 1){       //for now, just count between 0 and 1.
        currButton = 0;
      }
    }
  }
 // digitalWrite(buttonLed, currButton);    //turn LED on/off to represent myDigitalVal
}

