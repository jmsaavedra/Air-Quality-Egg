/*
 Fading
 
 This example shows how to fade an LED using the analogWrite() function.
 
 The circuit:
 * LED attached from digital pin 9 to ground.
 
 Created 1 Nov 2008
 By David A. Mellis
 modified 30 Aug 2011
 By Tom Igoe
 
 http://arduino.cc/en/Tutorial/Fading
 
 This example code is in the public domain.
 
 */


int ledR = 3;    // LED connected to digital pin 9
int ledG = 5;
int ledB = 6;

void setup()  { 
  // nothing happens in setup 
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
} 

void loop(){ 

  faderUp(ledR);
  faderUp(ledG);
  faderUp(ledB);

  faderDown(ledR);
  faderDown(ledG);
  faderDown(ledB);
  
  faderUp(ledR);
  faderDown(ledR);
  
  faderUp(ledG);
  faderDown(ledG);
  
  faderUp(ledB);
  faderDown(ledB);
}


void faderUp(int led){
  // fade in from min to max in increments of 5 points:
  for(int fadeValue = 0 ; fadeValue <= 200; fadeValue +=2) { 
    // sets the value (range from 0 to 255):
    analogWrite(led, fadeValue);    
    // wait for 30 milliseconds to see the dimming effect    
    delay(30);                            
  } 
}

void faderDown(int led){

  // fade out from max to min in increments of 5 points:
  for(int fadeValue = 200 ; fadeValue >= 0; fadeValue -=2) { 
    // sets the value (range from 0 to 255):
    analogWrite(led, fadeValue);       
    // wait for 30 milliseconds to see the dimming effect    
    delay(30);                            
  }
}


