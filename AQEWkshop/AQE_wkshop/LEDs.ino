
//LEDs
const int statusLed = 9;
//const int buttonLed = 5;

int ledTime = 0;

void ledSetup(){
  pinMode(statusLed, OUTPUT);
//  pinMode(buttonLed, OUTPUT);

  for(int i=0; i<5; i++){
    digitalWrite(statusLed, HIGH);
//    digitalWrite(buttonLed, LOW);
    delay(150);
    digitalWrite(statusLed, LOW);
//    digitalWrite(buttonLed, HIGH);
    delay(150); 
  }
  digitalWrite(statusLed, LOW); 
//  digitalWrite(buttonLed, LOW); 
  //ledTimeStamp = currTime;
}

void ledUpdate(){

  ledTime = currTime % 2000;

  if (ledTime < 1000){
    analogWrite(statusLed, ledTime/5);
  } 
  else if (ledTime < 1950) {
    analogWrite(statusLed, 200-((ledTime-1000)/5));
  } 
  else {
    analogWrite(statusLed, 0);
  }
}

void flashStatusLed(){
  for(int i=0; i<5; i++){
    digitalWrite(statusLed, HIGH);
    delay(100);
    digitalWrite(statusLed, LOW);
    delay(100);
  }
}

void flashAllLed(){
  for(int i=1; i<10; i++){
    digitalWrite(statusLed, HIGH);
//    digitalWrite(buttonLed, HIGH);
    delay(100/i);
    digitalWrite(statusLed, LOW);
//    digitalWrite(buttonLed, LOW);
    delay(100/i);
  }
}



