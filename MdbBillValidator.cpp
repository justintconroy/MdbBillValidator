#include "MdbBillValidator.h"

MdbMaster master;

bool _setupInfoRetrieved;

unsigned int _featureLevel;
unsigned int _countryCode;
unsigned int _billScaleFactor;
unsigned int _decimalPlaces;
unsigned int _stackerCapacity;
unsigned int _billSecurityLevels;
bool _escrowEnabled;
unsigned char _billTypeCredit[16];

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

// Send a reset command to the bill validator. There is no response to
// this command.
void MdbBillValidator::SoftReset()
{
	master.SendCommand(BILL_ADDR, RESET);
	_setupInfoRetrieved = false;
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

	// Make sure the number of bytes returned is in the correct range.
	// There should be between 11 and 27 (inclusive bytes) depending on
	// how many bill types are supported.
	if (numBytesReturned < 11 || numBytesReturned > 27)
	{
		return -2;
	}

	// Read and save the setup information.
	_featureLevel = response[0];
	_countryCode = (((unsigned int) response[1]) << 8) | ((unsigned int) response[2]);
	_billScaleFactor = (((unsigned int) response[3]) << 8) | ((unsigned int) response[4]);
	_decimalPlaces = response[5];
	_stackerCapacity = (((unsigned int) response[6]) << 8) | ((unsigned int) response[7]);
	_billSecurityLevels = (((unsigned int) response[8]) << 8) | ((unsigned int) response[9]);
	_escrowEnabled = response[10] == 0xFF;

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

int MdbBillValidator::Poll()
{
	master.SendCommand(BILL_ADDR, POLL);

	unsigned char response[MAX_MSG_SIZE];
	unsigned int numBytesReturned = 0;
	master.GetResponse(response, &numBytesReturned);

	if (response[0] == NAK || numBytesReturned == 0)
	{
		// Negative Acknowledgment
		return -1;
	}

	return 0;
}

// Getter method for retrieving the feature level of the currently
// connected bill acceptor/validator. This method must be called after
// setup has already been called, otherwise the feature level will not
// be known and this method will return -1 to indicate an error.
unsigned int MdbBillValidator::FeatureLevel()
{
	if(_setupInfoRetrieved)
	{
		return _featureLevel;
	}
	else
	{
		return -1;
	}
}

unsigned int MdbBillValidator::CountryCode()
{
	if(_setupInfoRetrieved)
	{
		return _countryCode;
	}
	else
	{
		return -1;
	}
}

unsigned int MdbBillValidator::BillScaleFactor()
{
	if(_setupInfoRetrieved)
	{
		return _billScaleFactor;
	}
	else
	{
		return -1;
	}
}

unsigned int MdbBillValidator::DecimalPlaces()
{
	if(_setupInfoRetrieved)
	{
		return _decimalPlaces;
	}
	else
	{
		return -1;
	}
}

unsigned int MdbBillValidator::StackerCapacity()
{
	if(_setupInfoRetrieved)
	{
		return _stackerCapacity;
	}
	else
	{
		return -1;
	}
}

unsigned int MdbBillValidator::BillSecurityLevels()
{
	if(_setupInfoRetrieved)
	{
		return _billSecurityLevels;
	}
	else
	{
		return -1;
	}
}

bool MdbBillValidator::EscrowEnabled()
{
	if(_setupInfoRetrieved)
	{
		return _escrowEnabled;
	}
	else
	{
		return false;
	}
}

unsigned char* MdbBillValidator::BillTypeCredit()
{
	if(_setupInfoRetrieved)
	{
		return _billTypeCredit;
	}
	else
	{
		return NULL;
	}
}

String MdbBillValidator::ToString()
{
	String result;

	if (_setupInfoRetrieved)
	{
		result = "Feature Level: " + String(_featureLevel)
			+ "\nCountryCode: " + String(_countryCode)
			+ "\nBillScaleFactor: " + String(_billScaleFactor)
			+ "\nDecimalPlaces: " + String(_decimalPlaces)
			+ "\nStackerCapacity: " + String(_stackerCapacity)
			+ "\nBillSecurityLevels: " + String(_billSecurityLevels)
			+ "\nEscrowEnabled: " + String(_escrowEnabled)
			+ "\nBillTypes:";

		// Get each bill type.
		//for (int i = 0; i < 16; i++)
		//{
			//result += " " + _billTypeCredit[i];
		//}
	}
	else
	{
		result = "Configuration of bill validator unknown. Run GetSetup()";
	}

	return result;
}

