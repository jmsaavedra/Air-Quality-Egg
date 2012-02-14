/* 
 
 //----- Simple demo for feeding data to Pachube.
 
 sketch is sending 1 analog and 1 digital sensor readings to Pachube feed: https://pachube.com/feeds/45999
 
 //----- circuit
 
 >>> LED attached to             digitalPin 5.
 >>> button attached to          digitalPin 3.
 >>> analog sensor attached to   analogPin 2.
 
 nanode note: cannot attach anything to digitalPin 8 or digitalPin 9!
 
 2012-01-25 <saavedra@jos.ph> for http://collab.sensemake.rs 
 based on code by:
 2011-07-08 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php
 
 */

#include <EtherCard.h>

// change these settings to match your own setup
//#define FEED    "45999"
//#define APIKEY  "5VCx3ftgRHd6AT2leyay2ySmARPb9ZUsHY-KgtgTLoLKMg0VEqjxYIbRo0KAItkWXOlnDSdQyECjAaQ9Vb307npyLewgxEp3JE8yUN8YdR-mAIMB0KM8MuploVzpVt6T"

#define FEED "48091"
#define APIKEY  "7HsgaVRMCZ5FOSGypykT72YyKvKSAKxQbXdIanBxeEFBYz0g"

// ethernet interface mac address, must be unique on the LAN
byte mymac[] = { 
  0x74,0x69,0x69,0x2D,0x30,0x31 };

const int transmitFrequency = 10; //time to wait between calls in seconds

String unitID = "unit01"; //name of unique datastreams (prefix)

unsigned long int currTime;
int currButtonVal;
int prevButtonVal;
int myDigitalVal;
int myAnalogVal;

void setup () {
  Serial.begin(57600);
  Serial.println("\n[webClient]");

  pinMode(3, INPUT);  //button on dPin 3
  pinMode(5, OUTPUT); //LED on dPin 5
  myDigitalVal = 0;
  myAnalogVal = 0;
  prevButtonVal = 0;

  etherSetup(); //nanode ethernet stup stuff
}

void loop () {
  currTime = millis();

  ether.packetLoop(ether.packetReceive()); //needs to stay near top of loop

  if( !transmitTime() ){         //if transmitTime returns false
    if(currTime%1000 == 0){      //print the currTime every second
      Serial.print("millis: ");  
      Serial.println(currTime/1000);
    }

    //--- button and sensor reads
    currButtonVal = digitalRead(3);  //read our button
    if(currButtonVal != prevButtonVal){ //check for state change
      prevButtonVal = currButtonVal;
      if(prevButtonVal == 1){ 
        myDigitalVal ++ ;           //add one on each press
        if(myDigitalVal > 1){       //count between 0 and 1.
          myDigitalVal = 0;
        }
      }
    }

    digitalWrite(5, myDigitalVal);    //turn LED on/off to represent myDigitalVal
    myAnalogVal = analogRead(A2);     //read analogVal from analogPin 2
    //myAnalogVal = random(0, 1023);  //randomizer
    
    delay(15); //debounce
    //myAnalogVal = random(0, 1023);  
  }
}





