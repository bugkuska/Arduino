#include <SoftwareSerial.h>
#include <ModbusMaster.h>
SoftwareSerial mySerial(2, 15); // RO=RX, DI=TX
#define MAX485_RE      4  //RE
#define MAX485_RE_NEG  5  //DE

// instantiate ModbusMaster object
ModbusMaster node3;

//==preTrnasmission==//
void preTransmission3()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_RE, 1);
}
//==preTrnasmission==//

//==postTransmission==//
void postTransmission3()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_RE, 0);
}
//==preTrnasmission==//


//==postTransmission==//

//==Setup Function==//
void setup()
{
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_RE, OUTPUT);
  // Init in receive mode
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_RE, 0);
  Serial.println("start init serial 0");
  Serial.begin(9600);
  
  while (!Serial) {
    Serial.println("loop for init serial 0"); // wait for serial port to connect. Needed for Native USB only
  }
  Serial.println("start init software serial");
  mySerial.begin(9600);
  while (!mySerial) {
    Serial.println("loop for init software serial"); // wait for serial port to connect. Needed for Native USB only
  }
  
  // Modbus slave ID 1
  node3.begin(3, mySerial);
  
  // Callbacks allow us to configure the RS485 transceiver correctly
  node3.preTransmission(preTransmission3);
  node3.postTransmission(postTransmission3);
}
//==Setup Function==//

//==Loop Function==//
void loop()
{
  uint8_t result;  
  Serial.println("Get Temp&Humi Data");
  result = node3.readInputRegisters(0x040A, 2); // Read 2 registers starting at 1)
  if (result == node3.ku8MBSuccess)
  {
    Serial.print("Temperature: ");
    Serial.println(node3.getResponseBuffer(0)/10.0f);
    Serial.print("Humidity: ");
    Serial.println(node3.getResponseBuffer(1)/10.0f);
  }
  delay(1000);
}
//==Loop Function==//
