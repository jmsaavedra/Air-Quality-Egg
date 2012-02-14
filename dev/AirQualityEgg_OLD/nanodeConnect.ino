//nanode connection codez


  
//----- setup
void nanodeSetup(){
  //timer = 15; //initial wait time in seconds
  Serial.println("\n[webClient]");
  
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println( "Failed to access Ethernet controller");
  if (!ether.dhcpSetup())
    Serial.println("DHCP failed");

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  

  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");

  ether.printIp("SRV: ", ether.hisip);
  //delay(3000);
  //if(debug) Serial.println("--- nanode setup end");
}

//----- update nanode packets
void nanodeUpdate(){
    ether.packetLoop(ether.packetReceive()); //needs to stay near top of loop
    //if(debug) Serial.println("--- nanode update end");
}

//----- send data!
void nanodeSend(int currNo2, int currCo, int currQuality, int currHumidity, int currTemp, int currButton){
  if(debug) Serial.println("nanodeSend start");
  if(debug){
    Serial.println("-----ATTEMPT SEND DATA-----");
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
    Serial.print("sending Button     = ");
    Serial.println(currButton);
    Serial.println("---------------------------");
  }

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
  
  //stash.print("button,");
  //stash.println((word)currButton);

  stash.save();

  if(debug) Serial.println("--- stash saved");
  // generate the header with payload - note that the stash size is used,
  // and that a "stash descriptor" is passed in as argument using "$H"
  Stash::prepare(PSTR("PUT http://$F/v2/feeds/$F.csv HTTP/1.0" "\r\n"
    "Host: $F" "\r\n"
    "X-PachubeApiKey: $F" "\r\n"
    "Content-Length: $D" "\r\n"
    "\r\n"
    "$H"),
  website, PSTR(FEED), website, PSTR(APIKEY), stash.size(), sd);

  // send the packet - this also releases all stash buffers once done
  ether.tcpSend();
  if(debug) Serial.println("--- tcp sent");  
  //flashSending(); //flash LED
}

