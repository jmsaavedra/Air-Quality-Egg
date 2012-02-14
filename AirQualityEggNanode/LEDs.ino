
void ledSetup(){
  pinMode(statusLed, OUTPUT);
  pinMode(buttonLed, OUTPUT);

  for(int i=0; i<5; i++){
    digitalWrite(statusLed, HIGH);
    digitalWrite(buttonLed, LOW);
    delay(150);
    digitalWrite(statusLed, LOW);
    digitalWrite(buttonLed, HIGH);
    delay(150); 
  }
  digitalWrite(statusLed, LOW); 
  digitalWrite(buttonLed, LOW); 
}

void breathStatusLed(){
  for(int i=0; i<175; i++){
    analogWrite(statusLed, i);
    delay(3);
  }
  for(int i=174; i>0; i--){
    analogWrite(statusLed, i);
    delay(3);
  }
  analogWrite(statusLed, 0);
}

void flashStatusLed(){
  //pinMode(buttonLed, OUTPUT);
  for(int i=0; i<3; i++){
    digitalWrite(statusLed, HIGH);
    //digitalWrite(buttonLed, HIGH);
    delay(250);
    digitalWrite(statusLed, LOW);
    //digitalWrite(buttonLed, LOW);
    delay(250); 
  }
}
