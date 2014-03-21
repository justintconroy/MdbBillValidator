/*
  MdbMaster.h - MDB master library for Wiring
  Copyright (c) 2014 Justin T. Conroy. All right reserved.
  Modified 16 March 2014 by Justin T. Conroy

  This library is an API for accessing the MDB device. Currently it only
  supports level 1 bill acceptors. It makes use of a modified version of
  Arduino's HardwareSerial library to communicate with devices. This libary
  provides a convenient interface to common commands.

  This library is synchronous, meaning that all of the methods are blocking
  and will wait for a response (or a timeout) before returning a result.
*/

#ifndef MdbMaster_h
#define MdbMaster_h

#include "MdbSerial.h"
#include <Arduino.h>

#define BILL_ADDR 0x30

// Biller commands
#define RESET 0x00
#define SETUP 0x01
#define STATUS 0x01 // Legacy definition of SETUP.
#define SECURITY 0x02
#define POLL 0x03
#define BILLTYPE 0x04
#define ESCROW 0x05
#define STACKER 0x06
#define EXP_CMD 0x07

// Response Codes
#define ACK 0x00 // Acknowledge
#define RET 0xAA // Retransmit (VMC only)
#define NAK 0xFF // Negative Acknowledge

class MdbMaster
{
	private:
		int _validatorFeatureLevel;
		virtual void SendValidatorCommand(unsigned char);
		virtual void SendValidatorCommand(unsigned char, unsigned char*, unsigned int);

		virtual int GetValidatorResponse(unsigned char*, unsigned int*);

	public:
		MdbMaster();

		virtual void HardReset();
		virtual void SoftResetValidator();

		virtual void SendValidatorAck();
		virtual void SendValidatorRet();
		virtual void SendValidatorNak();

		virtual int GetValidatorFeatureLevel();
		virtual int GetValidatorStatus();
		//virtual int SetValidatorSecurity(unsigned int);
		//virtual int PollValidator();
		//virtual int SetValidatorBillTypes(unsigned int);
		//virtual int ReleaseBillInValidatorEscrow();
		//virtual int RejectBillInValidatorEscrow();
		//virtual int GetValidatorStackerCount();

};



#endif // MdbMaster_h

