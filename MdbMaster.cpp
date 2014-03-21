#include "MdbMaster.h"

MdbMaster::MdbMaster()
{
	MdbPort.begin();
	while (!MdbPort);

	_validatorFeatureLevel = 0;
}

void MdbMaster::SendValidatorCommand(unsigned char command)
{
	unsigned char dataBytes[1];

	MdbMaster::SendValidatorCommand(command, dataBytes, 0);

}

void MdbMaster::SendValidatorCommand(unsigned char command,
		unsigned char *dataBytes, unsigned int dataByteCount)
{
	unsigned char sum = 0;

	// Send the command along with the Bill Validator Address.
	MdbPort.write(BILL_ADDR | command, 1);
	sum += BILL_ADDR | command;

	// Limit the number of data bytes that can be sent for a command
	// to 34, since the total length of a message, including the
	// address/command byte and the checksum byte, is limited to 36 bytes.
	if (dataByteCount > 34)
	{
		dataByteCount = 34;
	}

	// Send data bytes.
	for (unsigned int i = 0; i < dataByteCount; i++)
	{
		MdbPort.write(dataBytes[i], 0);
		sum += dataBytes[i];
	}

	// Send checksum.
	MdbPort.write(sum, 0);
}

// Try to read the response after sending a command to the validator.
// Wait for ~5ms after sending a command before returning with error
// code -1 (request retransmit). If there is some other, more fatal error,
// returns -2. If the message is recieved with no problem, 0 is the return
// value and the returned bytes will be put into the array pointed at by
// the response parameter. The number of bytes received will be stored in
// the variable referenced by the numBytes parameter. Other special
// conditions include ACK and NAK, which return 1 and 2 respectively.
//
// Return Codes:
//    0: Message returned normally in response parameter reference
//    1: ACK
//    2: NAK
//    -1: Request retransmit after response timeout
//    -2: Unrecoverable error, device should probably be reset after this.
int MdbMaster::GetValidatorResponse(unsigned char *response,
		unsigned int *numBytes)
{
	return 0;
}

int MdbMaster::GetValidatorFeatureLevel()
{
	// Not implemented yet.
	return 0;
}

// Send a reset command to the bill validator. There is no response to
// this command.
void MdbMaster::SoftResetValidator()
{
	SendValidatorCommand(RESET);
}

// Send an acknowledgement.
void MdbMaster::SendValidatorAck()
{
	MdbPort.write(ACK, 0);
}

// Send a retransmit request.
void MdbMaster::SendValidatorRet()
{
	MdbPort.write(RET, 0);
}

// Send a negative acknowledgement.
void MdbMaster::SendValidatorNak()
{
	MdbPort.write(NAK, 0);
}

// A hard reset will trigger all connected peripherals to reset.
void MdbMaster::HardReset()
{
	pinMode(1, OUTPUT);
	digitalWrite(1, LOW);
	delay(150);
	digitalWrite(1, HIGH);
	delay(150);
}

int MdbMaster::GetValidatorStatus()
{
	SendValidatorCommand(SETUP);
	return 0;
}

