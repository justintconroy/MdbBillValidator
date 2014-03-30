#include "MdbBillValidator.h"

MdbMaster master;

bool _setupInfoRetrieved;

int _validatorFeatureLevel;
int _countryCode;
int _billScalingFactor;
int _decimalPlaces;
int _stackerCapacity;
int _billSecurityLevels;
bool _validatorEscrow;
char _billTypeCredit[16];

MdbBillValidator::MdbBillValidator()
{
	master = MdbMaster();

	_setupInfoRetrieved = false;
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

	unsigned char response[MAX_MSG_SIZE];
	unsigned int numBytesReturned;
	master.GetResponse(response, &numBytesReturned);

	if (response[0] == NAK || numBytesReturned == 0)
	{
		// Negative Acknowledgment
		return -1;
	}

	if (numBytesReturned != 27)
	{
		return -2;
	}

	// Read and save the setup information.
	_validatorFeatureLevel = response[0];
	_countryCode = (((unsigned int) response[1]) << 8) || ((unsigned int) response[2]);
	_billScalingFactor = (((unsigned int) response[3]) << 8) || ((unsigned int) response[4]);
	_decimalPlaces = response[5];
	_stackerCapacity = (((unsigned int) response[6]) << 8) || ((unsigned int) response[7]);
	_billSecurityLevels = (((unsigned int) response[8]) << 8) || ((unsigned int) response[9]);
	_validatorEscrow = response[10] == 0xFF;

	// Get the value of each bill type.
	for (int i = 0; i < 16; i++)
	{
		_billTypeCredit[i] = response[11 + i];
	}

	_setupInfoRetrieved = true;

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

