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

