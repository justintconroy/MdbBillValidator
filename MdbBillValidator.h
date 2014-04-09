/*
  MdbBillValidator.h - MDB bill validator library for Wiring
  Copyright (c) 2014 Justin T. Conroy. All right reserved.
  Created 24 March 2014 by Justin T. Conroy

  This library is an API for accessing the MDB device. It makes use of a
  modified version of Arduino's HardwareSerial library to communicate
  with devices. This libary provides a convenient interface to common
  commands.

  This library is synchronous, meaning that all of the methods are blocking
  and will wait for a response (or a timeout) before returning a result.
*/

#ifndef MdbBillValidator_h
#define MdbBillValidator_h

#include "MdbMaster.h"

// Bill Validator Peripheral Address.
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


class MdbBillValidator
{
	public:
		MdbBillValidator();

		virtual void SendAck();
		virtual void SendRet();
		virtual void SendNak();

		virtual void SoftReset();

		virtual int GetSetup();
		virtual int SetSecurity(unsigned int);

		//virtual int Poll();

		//virtual int SetBillTypes(unsigned int);

		//virtual int ReleaseBillInEscrow();
		//virtual int RejectBillInEscrow();

		//virtual int GetStackerCount();

		// Expansion Commands
		//virtual int SendExpansionCommand();

		virtual unsigned int FeatureLevel();
		virtual unsigned int CountryCode();
		virtual unsigned int BillScaleFactor();
		virtual unsigned int DecimalPlaces();
		virtual unsigned int StackerCapacity();
		virtual unsigned int BillSecurityLevels();
		virtual bool EscrowEnabled();
		virtual unsigned char* BillTypeCredit();

		virtual String ToString();

};

#endif /* MdbBillValidator_h */
