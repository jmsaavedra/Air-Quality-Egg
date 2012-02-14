int currButtonVal = 0;
int prevButtonVal = 0;
int buttonPin = 7;
int currButton = 0;

void setup(){
  Serial.begin(57600); 
  pinMode(buttonPin, INPUT);
}

void loop(){

  int myButton = getButton();
  Serial.println(myButton); 
}

int getButton(){
  currButtonVal = digitalRead(buttonPin);  //read our button

    if(currButtonVal != prevButtonVal){ //check for state change
    prevButtonVal = currButtonVal;  //set prev to curr

    if(prevButtonVal == 1){ 
      currButton ++ ;           //add one on each press 
      //(this is for if a counter is desired in the future)

      if(currButton > 1){       //count between 0 and 1.
        currButton = 0;
      }
    }
  }
  //setButtonLed(currButton);
  return(currButton);
}

