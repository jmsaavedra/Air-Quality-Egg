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
 
 #include "DHT.h"

#define DHTPIN 2     // what pin we're connected to

#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
#include <EtherCard.h>

// change these settings to match your own setup
#define FEED    "51752"
#define APIKEY  "dMQT02KQVbLd9Rh1T_SN-nc6O5CSAKw5ZXU2cjBBTDVUND0g"

DHT dht(DHTPIN, DHTTYPE);

// ethernet interface mac address, must be unique on the LAN
byte mymac[] = { 
  0x74,0x69,0x69,0x2D,0x30,0x31 };

const int transmitFrequency = 10; //time to wait between calls in seconds

String unitID = "unit01_"; //name of unique datastreams (prefix)

unsigned long int currTime;

float h, hInit;
float t, tInit;

void setup () {
  Serial.begin(57600);
  Serial.println("\n[webClient]");

  pinMode(5, OUTPUT); //LED on dPin 5

  dht.begin();
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
   
    delay(15); //debounce
    //myAnalogVal = random(0, 1023);  
  }
  
    // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  hInit = dht.readHumidity();
  tInit = dht.readTemperature();

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(tInit) || isnan(hInit) || (tInit==0 && hInit==0)) {
    Serial.println("Failed to read from DHT");
  } else {
    h = hInit;
    t = tInit;
    Serial.print("Humidity: "); 
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: "); 
    Serial.print(t);
    Serial.println(" *C");
  }
}





