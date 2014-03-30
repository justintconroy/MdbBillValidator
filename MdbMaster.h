/*
  MdbMaster.h - MDB master library for Wiring
  Copyright (c) 2014 Justin T. Conroy. All right reserved.
  Created 16 March 2014 by Justin T. Conroy

  This library is an API for accessing the MDB device. It makes use of a
  modified version of Arduino's HardwareSerial library to communicate
  with devices. This libary provides a convenient interface to common
  commands.
*/

#ifndef MdbMaster_h
#define MdbMaster_h

#include "MdbSerial.h"
#include <Arduino.h>

// Response Codes
#define ACK 0x00 // Acknowledge
#define RET 0xAA // Retransmit (VMC only)
#define NAK 0xFF // Negative Acknowledge

#define MAX_MSG_SIZE 36

class MdbMaster
{
	public:
		MdbMaster();

		virtual void HardReset();

		virtual void SendAck();
		virtual void SendRet();
		virtual void SendNak();

		virtual void SendCommand(unsigned char, unsigned char);
		virtual void SendCommand(unsigned char, unsigned char,
				unsigned char*, unsigned int);

		virtual int GetResponse(unsigned char*, unsigned int*);


};

#endif // MdbMaster_h

