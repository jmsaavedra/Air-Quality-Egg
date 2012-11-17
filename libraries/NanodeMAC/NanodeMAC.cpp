/***********************************************************************
 * NanodeMAC
 * Rufus Cable, June 2011 (threebytesfull)
 *
 * Library version created by Andrew Lindsay for use with Nanode and 
 * EtherShield Library at https://github.com/thiseldo/EtherShield
 *
 * Based on sample code to read the MAC address from the 11AA02E48 on the
 * back of the Nanode V5 board.
 *
 * This code is hacky and basic - it doesn't check for bus errors
 * and will probably fail horribly if it's interrupted. It's best
 * run in setup() - fetch the MAC address once and keep it. After
 * the address is fetched, it puts the chip back in standby mode
 * in which it apparently only consumes 1uA.
 *
 * Feel free to reuse this code - suggestions for improvement are
 * welcome! :)
 *
 * BITS    7   6   5   4   3   2   1   0
 * PORTD = D7  D6  D5  D4  D3  D2  D1  D0
 * PORTB = -   -   D13 D12 D11 D10 D9  D8
 *
 * Nanode has UNI/O SCIO on DIG7
 *
 ***********************************************************************/

#include "NanodeMAC.h"
#include <inttypes.h>

#define D7_ON  (1<<7)
#define D7_OFF (~D7_ON)

#define SCIO_HIGH PORTD |= D7_ON
#define SCIO_LOW  PORTD &= D7_OFF

#define SCIO_OUTPUT DDRD |= D7_ON
#define SCIO_INPUT  DDRD &= D7_OFF

#define SCIO_READ ((PIND & D7_ON) != 0)

#define WAIT_QUARTER_BIT delayMicroseconds(9);
#define WAIT_HALF_BIT delayMicroseconds(20);

#define NOP PORTD &= 0xff

// Fixed Timings
// standby pulse time (600us+)
#define UNIO_TSTBY_US 600
// start header setup time (10us+)
#define UNIO_TSS_US 10
// start header low pulse (5us+)
#define UNIO_THDR_US 6

// SCIO Manipulation macros
#define BIT0 SCIO_HIGH;WAIT_HALF_BIT;SCIO_LOW;WAIT_HALF_BIT;
#define BIT1 SCIO_LOW;WAIT_HALF_BIT;SCIO_HIGH;WAIT_HALF_BIT;


void NanodeMAC::unio_standby() {
  
  SCIO_OUTPUT;
  SCIO_HIGH;
  delayMicroseconds(UNIO_TSTBY_US);
}

void NanodeMAC::unio_start_header() {
  SCIO_LOW;
  delayMicroseconds(UNIO_THDR_US);
  unio_sendByte(B01010101);
}

void NanodeMAC::unio_sendByte(byte data) {
  
  SCIO_OUTPUT;
  for (int i=0; i<8; i++) {
    if (data & 0x80) {
      BIT1;
    } else {
      BIT0;
    }
    data <<= 1;
  }
  // MAK
  BIT1;
  // SAK?
  //bool sak = unio_readBit();
  unio_readBit();
}

void NanodeMAC::unio_readBytes(byte *addr, int length) {
  for (int i=0; i<length; i++) {
    
    byte data = 0;
    for (int b=0; b<8; b++) {
      data = (data << 1) | (unio_readBit() ? 1 : 0);
    }
    SCIO_OUTPUT;
    if (i==length-1) {
      BIT0; // NoMAK
    } else {
      BIT1; // MAK
    }
    //bool sak = unio_readBit();
    unio_readBit();
    addr[i] = data;
  }
}

inline bool NanodeMAC::unio_readBit()
{
  SCIO_INPUT;
  WAIT_QUARTER_BIT;
  bool value1 = SCIO_READ;
  WAIT_HALF_BIT;
  bool value2 = SCIO_READ;
  WAIT_QUARTER_BIT;
  return (value2 && !value1);
}

NanodeMAC::NanodeMAC( byte *mac_address ) {

  // Turn off Interrupts while we read the mac address
  noInterrupts();

  // standby
  unio_standby();
  
  // start header
  unio_start_header();
  
  // address A0
  unio_sendByte(0xA0);
  // 0x3 READ
  unio_sendByte(0x03);
  // word address MSB 0x00
  unio_sendByte(0x00);
  // word address LSB 0xFA
  unio_sendByte(0xFA);
  
  // read 6 bytes into array
  unio_readBytes(mac_address, 6);
  
  // back to standby
  unio_standby();
  
  // Re-enable interrupts
  interrupts();

}
  
 
