
/* nanode ethernet vars and functions */

//----- vars
char website[] PROGMEM = "api.pachube.com";

byte Ethernet::buffer[700];
uint32_t timer;
Stash stash;

//----- setup
void etherSetup(){
  timer = 15; //initial wait time in seconds
  
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
}

//----- check time
boolean transmitTime(){
  if (currTime > timer) { //we've hit our timer limit
    sendEtherData();      //send out all data!
    timer = currTime + (transmitFrequency*1000); //reset timer
    return true;
  } else return false;
}


//----- send data!
void sendEtherData(){

  Serial.println("-----ATTEMPT SEND DATA-----");
  Serial.print("myAnalog: ");
  Serial.println(myAnalogVal);
  Serial.print("myDigital: ");
  Serial.println(myDigitalVal);
  Serial.println("---------------------------");

  //create stream ID strings
  String stream1 = unitID;
  stream1 = stream1 + "_analog,";
  
  String stream2 = unitID;
  stream2 = stream2 +"_digital," ;

  //byte to hold stringIDs and data
  byte sd = stash.create();
  
  //stash.print("analog,");
  stash.print(stream1);
  stash.println((word)myAnalogVal);
  
  //stash.print("digital,");
  stash.print(stream2);
  stash.println((word)myDigitalVal);
  
  stash.save();

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
}


