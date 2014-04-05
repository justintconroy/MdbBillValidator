#include "MdbMaster.h"

unsigned long _commandSentTime;

MdbMaster::MdbMaster()
{
	MdbPort.begin();
	while (!MdbPort);

	_commandSentTime = 0;
}

void MdbMaster::SendCommand(unsigned char address, unsigned char command)
{
	unsigned char dataBytes[1];

	MdbMaster::SendCommand(address, command, dataBytes, 0);

}

void MdbMaster::SendCommand(unsigned char address, unsigned char command,
		unsigned char *dataBytes, unsigned int dataByteCount)
{
	unsigned char sum = 0;

	// Send the command along with the Bill Validator Address.
	MdbPort.write(address | command, 1);
	sum += address | command;

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

	_commandSentTime = millis();
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
//   -1: NAK: Request retransmit after response timeout or NAK recieved.
//   -2: Unrecoverable error, device should probably be reset after this.
int MdbMaster::GetResponse(unsigned char *response, unsigned int *numBytes)
{
	int index = 0;
	int lastMode = 0;

	// Wait for some bytes to be available. I should probably add some
	// sort of timeout here.
	while (!MdbPort.available())
	{
		if (millis() - _commandSentTime > 5)
		{
			return -1;
		}
	}

	// Loop through bytes received until there are either no bytes
	// available, 36 bytes have been received (max message size),
	// or a byte with the mode bit set, which signifies the end of
	// the message.
	while ((MdbPort.available() > 0)
			&& (index <= MAX_MSG_SIZE)
			&& (lastMode == 0))
	{
		response[index] = MdbPort.read() & 0xFF;
		lastMode = (response[index] >> 8) & 0x01;
		index++;
	}

	*numBytes = index;

	return 0;
}

// Send an acknowledgement.
void MdbMaster::SendAck()
{
	MdbPort.write(ACK, 0);
}

// Send a retransmit request.
void MdbMaster::SendRet()
{
	MdbPort.write(RET, 0);
}

// Send a negative acknowledgement.
void MdbMaster::SendNak()
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

