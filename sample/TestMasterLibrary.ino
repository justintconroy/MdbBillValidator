#include <MdbBillValidator.h>
#include <SoftwareSerial.h>

MdbBillValidator validator;
SoftwareSerial pcSerial(7, 8);

void setup()
{
  validator = MdbBillValidator();

  pcSerial.begin(9600);

  validator.SoftReset();
  delay(100);

  //validator.Poll();
  //delay(20);
  //validator.SendAck();
  //delay(10);

}

void loop()
{
  int result = validator.GetSetup();
  delay(30);

  if (result == 0)
  {
    String x = validator.ToString() + "\n\n";
    pcSerial.print(x);
  }
  else if (result == -1)
  {
    pcSerial.print("NAK\n\n");
  }
  else if (result == -2)
  {
    pcSerial.print("Wrong Number of bytes received.\n\n");
  }
}
