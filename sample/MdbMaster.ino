#include <MdbSerial.h>

#include <SoftwareSerial.h>

#define BILL_ADDR 0x30

#define txLcdPin 4
#define buttonPin 2

#define NUMSTATUSES 2

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

// MDB state machine states.
enum MdbState
{
  hardReset,
  waitHardReset,
  softReset,
  waitSoftReset,
  initState,
  waitJustReset,
  ackJustReset,
  getStatus,
  waitStatusResponse,
  ackStatusResponse,
  sendSecurity,
  waitSecurityResponse,
  getLevel1Id,
  waitLevel1Id,
  enableBillTypes,
  poll,
  readPollResponse,
  ackPollResponse,
  acceptBill,

  displayResponse
};

char buttonMode = 0;

SoftwareSerial LCD = SoftwareSerial(0, txLcdPin);
// since the LCD does not send data back to the Arduino, we should only define the txLcdPin
const int LCDdelay=10;  // conservative, 2 actually works

// wbp: goto with row & column
void lcdPosition(int row, int col) {
  LCD.write(0xFE);   //command flag
  LCD.write((col + row*64 + 128));    //position 
  delay(LCDdelay);
}
void clearLCD(){
  LCD.write(0xFE);   //command flag
  LCD.write(0x01);   //clear command.
  delay(LCDdelay);
}
void backlightOn() {  //turns on the backlight
  LCD.write(0x7C);   //command flag for backlight stuff
  LCD.write(157);    //light level.
  delay(LCDdelay);
}
void backlightOff(){  //turns off the backlight
  LCD.write(0x7C);   //command flag for backlight stuff
  LCD.write(128);     //light level for off.
  delay(LCDdelay);
}
void serCommand(){   //a general function to call the command flag for issuing all other commands   
  LCD.write(0xFE);
}
void lcdPrintBits(unsigned char regValue, int x, int y){
  for(int i = 0; i < 8; i++)
  {
    lcdPosition(x,y+i);
    if ((regValue >> (7 - i)) & 0x01)
      LCD.print("1");
    else
      LCD.print("0");
  }
}

void MdbSendCommand(unsigned char command, unsigned char *cmdBytes, unsigned int numCmdBytes)
{
  unsigned char sum = 0;

  // Write Address
  MdbPort.write(BILL_ADDR | command, 1);
  sum += BILL_ADDR | command;

  // Avoid buffer overflows on cmdBytes.
  if (numCmdBytes > 33)
  {
    numCmdBytes = 33;
  }

  // Send each additional byte for the command.
  for (int i = 0; i < numCmdBytes; i++)
  {
    MdbPort.write(cmdBytes[i], 0);
    sum += cmdBytes[i];
  }

  // Send checksum.
  MdbPort.write(sum, 0);
}

void MdbSendAck()
{
  MdbPort.write(BILL_ADDR, 0);
}

// response should be a char array with size 36.
void MdbGetResponse(unsigned char *response, unsigned int numBytes)
{
  int index = 0;
  int lastMode = 0;

  // Wait for enough bytes to be ready.
  //while (MdbPort.available() < numBytes);

  // Largest message size is 36 bytes, so only ever read that much.
  // Last byte in a block has mode 1.
  while ((MdbPort.available() > 0)
          && (index < 37)
          && (lastMode == 0))
  {
    response[index] = MdbPort.read() & 0xFF;
    lastMode = (response[index] >> 8) & 0x01;

    index++;
  }
}

MdbState state;
String debugMessage = "";
String lastDebugMessage = "";

void setup()
{
  pinMode(txLcdPin, OUTPUT);
  LCD.begin(9600);
  clearLCD();
  lcdPosition(0,0);
  backlightOn();

  // Hard Reset
  pinMode(1, OUTPUT);
  digitalWrite(1, LOW);
  delay(200);
  digitalWrite(1, HIGH);
  delay(200);


  LCD.print("MDB Test Slave");

  MdbPort.begin();

  while (!MdbPort);

  state = waitSoftReset;
}


unsigned char extraCmdBytes[33];
unsigned char responseBytes[36];

unsigned long currentTime = 0;
unsigned long debugTime = 0;

void loop()
{

  switch(state)
  {
    case hardReset:
      digitalWrite(1, LOW);
      currentTime = millis();

      debugMessage = "Hard Reset";

      state = waitHardReset;
      break;
    case waitHardReset:
      if (millis() - currentTime > 200)
      {
        digitalWrite(2, HIGH);
        state = waitSoftReset;
      }
      break;
    case softReset:
      MdbSendCommand(RESET, extraCmdBytes, 0);
      currentTime = millis();

      debugMessage = "Soft Reset";

      state = waitSoftReset;
      break;

    case waitSoftReset:
      if (millis() - currentTime > (10*1000))
      {
        //state = softReset;
        state = initState;
      }

      break;

    case initState: // Initial state. Poll device.
      MdbSendCommand(POLL, extraCmdBytes, 0);
      currentTime = millis();

      // display current state for debug info
      debugMessage = "initState";

      state = waitJustReset;
      break;

    case waitJustReset:
      if (MdbPort.available() > 0)
      {
        MdbGetResponse(responseBytes, 1);
        debugMessage += " got response";
        state = ackJustReset;
      }
      else if ((millis() - currentTime) > 150)
      {
        // Try again.
        state = initState;
      }

      // display current state for debug info
      debugMessage = "waitJustReset";

      break;

    case ackJustReset:
      MdbSendAck();
      delay(10);

      state = getStatus;
      break;

    case getStatus:
      MdbSendCommand(STATUS, extraCmdBytes, 0);
      currentTime = millis();

      // display current state for debug info
      debugMessage = "getStatus";

      state = waitStatusResponse;
      break;

    case waitStatusResponse:
      if (MdbPort.available() > 0)
      {
        MdbGetResponse(responseBytes, 1);
        debugMessage += " got response";

        delay(10);
        state = ackStatusResponse;
      }
      else if ((millis() - currentTime) > 150)
      {
        // Try again.
        state = getStatus;
      }

      // display current state for debug info
      debugMessage = "waitStatusResponse";
      break;

    case ackStatusResponse:
      MdbSendAck();
      delay(10);
      state = sendSecurity;
      break;

    case sendSecurity:
      // Set all bills to "low" security level.
      extraCmdBytes[0] = 0;
      extraCmdBytes[1] = 0;
      MdbSendCommand(SECURITY, extraCmdBytes, 2);
      delay(10);
      state = waitSecurityResponse;
      break;

    case waitSecurityResponse:
      if (MdbPort.available() > 0)
      {
        MdbGetResponse(responseBytes, 1);
        debugMessage += " got response";
        state = getLevel1Id;
      }
      else if ((millis() - currentTime) > 150)
      {
        // Try again.
        state = sendSecurity;
      }

      // display current state for debug info
      debugMessage = "waitSecurityResponse";
      break;

    case getLevel1Id:
      // Set all bills to "low" security level.
      extraCmdBytes[0] = 0;
      MdbSendCommand(EXP_CMD, extraCmdBytes, 1);
      delay(10);
      state = waitLevel1Id;
      break;

    case waitLevel1Id:
      if (MdbPort.available() > 0)
      {
        MdbGetResponse(responseBytes, 1);
        debugMessage += " got response";
        delay(100);
        state = enableBillTypes;
      }
      else if ((millis() - currentTime) > 150)
      {
        // Try again.
        state = getLevel1Id;
      }

      // display current state for debug info
      debugMessage = "waitLevel1IdResponse";
      break;

    case enableBillTypes:
      // Enable all denominations of bills.
      extraCmdBytes[0] = 0xFF;
      extraCmdBytes[1] = 0xFF;

      // Enable escrow for all bills.
      extraCmdBytes[2] = 0xFF;
      extraCmdBytes[3] = 0xFF;

      MdbSendCommand(BILLTYPE, extraCmdBytes, 4);
      delay(10);
      state = poll;
      break;

    case poll:
      // Poll repeatedly.
      MdbSendCommand(POLL, extraCmdBytes, 0);
      state = readPollResponse;

      debugMessage = "Poll";
      delay(10);
      break;

    case readPollResponse:
    if (MdbPort.available() > 0)
      {
        MdbGetResponse(responseBytes, 1);
        state = ackPollResponse;
      }
      else if ((millis() - currentTime) > 150)
      {
        // Try again.
        state = poll;
      }

      break;

    case ackPollResponse:
      MdbSendAck();
      delay(10);

      //state = displayResponse;
      if (responseBytes[0] == 0x94)
      {
        state = acceptBill;
      }
      else
      {
        // poll repeatedly
        state = poll;
      }
      break;

    case acceptBill:
      extraCmdBytes[0] = 0x01;
      MdbSendCommand(ESCROW, extraCmdBytes, 1);

      debugMessage = "Accept Bill";

      state = poll;
      break;


    case displayResponse:
      // Display each hex byte from the last response on the screen.

      debugMessage = "";
      for (int j = 0; j < 11; j++)
      {
        String statusChar((unsigned int)responseBytes[j], HEX);
        debugMessage += statusChar;

        if (j < 10)
        {
          debugMessage += " ";
        }
      }

      state = poll;
      break;

    default:
      // do something here.
      debugMessage = "Default State";
  }

  if ((millis() - debugTime) > 1500)
  {
    clearLCD();
    lcdPosition(0,0);
    LCD.print(debugMessage);
    lastDebugMessage = debugMessage;
    debugTime = millis();
  }
}
