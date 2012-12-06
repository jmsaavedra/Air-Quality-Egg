/* nanode ethernet vars and functions */

//uses ethercard library from 
//https://github.com/jcw/ethercard

//----- vars
char website[] PROGMEM = "api.cosm.com";

// ethernet interface mac address, must be unique on the LAN
// add in find nanode MAC code!
byte mymac[] = { 
  0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[700];
unsigned long timer = 60000;
Stash stash; //holds data to send

//for counting how long it's been since last successful connection:
unsigned long lastResponseTime = 0;

//----- setup
void nanodeSetup(){
  Serial.println("\n[webClient]");
  timer = 65; //initial wait time in seconds

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println( "Failed to access Ethernet controller");
  if (!ether.dhcpSetup())
    Serial.println("DHCP failed");

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("Gateway:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  

  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");

  ether.printIp("SRV: ", ether.hisip);
}

void nanodeUpdate(){
  digitalWrite(resetterPin, HIGH);
  ether.packetLoop(ether.packetReceive());

  if ( ether.msgReceived() ){
    Serial.println(">>> RESPONSE RECEIVED ---\n\n");
    lastResponseTime = currTime;
    flashStatusLed();
  }

  if (currTime - lastResponseTime > (10*60000)){ // we have not connected in 10 min
    Serial.println("RESET ME");
    flashAllLed();
    nanodeReset();
  }
}

//----- check time, sendData if we've hit timer
boolean transmitTime(){
  if (currTime > timer) { //we've hit our transmit timer limit
    //tranmsitting = true;
    nanodeSendData();      //send out all curr data!
    timer = currTime + (transmitFrequency*1000); //reset timer
    return true;
  } 
  else { 
    return false;
  }
}

//----- send data!
void nanodeSendData(){

  Serial.println("-----BEGIN ATTEMPT SEND DATA-----");
  Serial.print("sending No2      = ");
  Serial.println(currNo2);
  Serial.print("sending CO       = ");
  Serial.println(currCo);
  Serial.print("sending Quality  = ");
  Serial.println(currQuality);
  Serial.print("sending Humidity = ");
  Serial.println(currHumidity);
  Serial.print("sending Temp     = ");
  Serial.println(currTemp);
  Serial.print("sending Button   = ");
  Serial.println(currButton);
  Serial.println("--------------------------------");

  //byte to hold stringIDs and data
  byte sd = stash.create();

  stash.print("NO2,");
  stash.println((word)currNo2);

  stash.print("CO,");
  stash.println((word)currCo);

  stash.print("airQuality,");
  stash.println((word)currQuality);

  stash.print("humidity,");
  stash.println((word)currHumidity);

  stash.print("temperature,");
  stash.println((word)currTemp);

  stash.print("button,");
  stash.println((word)currButton);

  stash.save();

  // generate the header with payload - note that the stash size is used,
  // and that a "stash descriptor" is passed in as argument using "$H"
  Stash::prepare(PSTR("PUT http://$F/v2/feeds/$F.csv HTTP/1.0" "\r\n"
    "Host: $F" "\r\n"
    "X-ApiKey: $F" "\r\n"
    "Content-Length: $D" "\r\n"
    "\r\n"
    "$H"),
  website, PSTR(FEED), website, PSTR(APIKEY), stash.size(), sd);

  // send the packet - this also releases all stash buffers once done
  ether.tcpSend();

  //flashStatusLed();
}

void nanodeReset(){
  digitalWrite(resetterPin, LOW);
}





