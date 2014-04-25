#include <EtherCard.h>
#include <NanodeMAC.h>
#include <StackPaint.h>
#include <SoftReset.h>
#include <stdint.h>

#define ETHERNET_BUFFER_LENGTH 600
extern uint8_t mymac[6];

char website[] PROGMEM = "api.cosm.com";

byte Ethernet::buffer[ETHERNET_BUFFER_LENGTH];

void setupNanode(){
  //char serial_number[18] = {0};
  memset((char *) Ethernet::buffer, 0, 18);
  for(uint8_t ii = 0; ii < 6; ii++){
    convertByteArrayToAsciiHex(mymac + ii, (char *) Ethernet::buffer + 3*ii, 1);
    if(ii == 5) Ethernet::buffer[3*ii+2] = '\0';
    else Ethernet::buffer[3*ii+2] = ':';
  }  
  Serial.println();
  Serial.print(F("Egg Serial #:  "));
  Serial.println((char *) Ethernet::buffer);  
  Serial.println();
  
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println(F("Failed to access Ethernet controller"));
    
  if (!ether.dhcpSetup()){
    Serial.println(F("DHCP failed resetting"));
    soft_restart();
  }
  
  Serial.print(F("IP: "));
  ether.printIp(ether.myip);
  Serial.println();
  
  Serial.print(F("GW: "));
  ether.printIp(ether.gwip);  
  Serial.println();  
  
  Serial.print(F("DNS: "));
  ether.printIp(ether.dnsip);  
  Serial.println();

  PRINT_STACK_SPACE;
  
  if (!ether.dnsLookup(website)){
    Serial.println(F("DNS failed"));
    Serial.println(F("Falling back to Xively Static IP"));
    ether.hisip[0] = 173;
    ether.hisip[1] = 203;
    ether.hisip[2] = 98;
    ether.hisip[3] = 29;    
  }
  else{
    Serial.println(F("DNS lookup succeeded."));
  }
  
  Serial.print(F("SRV: "));
  ether.printIp(ether.hisip);
  Serial.println();
  
  ether.disableBroadcast();
}

void loopNanode(){
    ether.packetLoop(ether.packetReceive());     
}

static void convertByteArrayToAsciiHex(uint8_t* hash, char * returnString, uint8_t byte_array_length) {
  const char digit2Ascii[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
  for (int i=0; i<byte_array_length; i++) {
    returnString[2*i    ] = digit2Ascii[hash[i]>>4];
    returnString[2*i + 1] = digit2Ascii[hash[i]&0xf];
  }
  returnString[2*(byte_array_length-1) + 2] = '\0';
}
