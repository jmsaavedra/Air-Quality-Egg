/*
* NanodeMAC.h
* Nanode MAC chip reading library header
*/

#ifndef _NANODEMAC_LIB_H
#define _NANODEMAC_LIB_H

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

class NanodeMAC {
  private:
	void unio_standby();
	void unio_start_header();
	void unio_sendByte(byte data);
	void unio_readBytes(byte *addr, int length);
	inline bool unio_readBit();
  public:
	NanodeMAC( byte *macaddr );
};

#endif //_NANODEMAC_LIB_H

