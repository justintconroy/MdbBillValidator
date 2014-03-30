#include "MdbBillValidator.h"

MdbMaster master;
int _validatorFeatureLevel;

MdbBillValidator::MdbBillValidator()
{
	master = MdbMaster();

	_validatorFeatureLevel = -1;
}

// Send an acknowledgement.
void MdbBillValidator::SendAck()
{
	master.SendAck();
}

// Send a retransmit request.
void MdbBillValidator::SendRet()
{
	master.SendRet();
}

// Send a negative acknowledgement.
void MdbBillValidator::SendNak()
{
	master.SendNak();
}

// Getter method for retrieving the feature level of the currently
// connected bill acceptor/validator. This method must be called after
// setup has already been called, otherwise the feature level will not
// be known and this method will return -1 to indicate an error.
int MdbBillValidator::GetFeatureLevel()
{
	// Not implemented yet.
	return _validatorFeatureLevel;
}

// Send a reset command to the bill validator. There is no response to
// this command.
void MdbBillValidator::SoftReset()
{
	master.SendCommand(BILL_ADDR, RESET);
}

int MdbBillValidator::GetSetup()
{
	master.SendCommand(BILL_ADDR, SETUP);
	return 0;
}

int MdbBillValidator::SetSecurity(unsigned int securitySettings)
{
	unsigned char securityBits[2];

	securityBits[0] = securitySettings && 0xFF;
	securityBits[1] = (securitySettings >> 8) && 0xFF;

	master.SendCommand(BILL_ADDR, SECURITY, securityBits, 2);

	unsigned char response[MAX_MSG_SIZE];
	unsigned int numBytesReturned;
	master.GetResponse(response, &numBytesReturned);

	if (numBytesReturned == 0)
	{
		// No data returned.
		return -1;
	}

	if (numBytesReturned > 1)
	{
		// The data received doesn't make any sense...
		return -2;
	}

	if (response[0] == NAK)
	{
		// Negative acknowledgement received.
		return -3;
	}

	if (response[0] == ACK)
	{
		return 0;
	}

	// Unidentifiable response. Needs more debugging.
	return -4;
}

