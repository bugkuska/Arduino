#include <SoftwareSerial.h>
#include <ModbusMaster.h>

SoftwareSerial mySerial(2, 15); // RO=RX, DI=TX

#define MAX485_RE      4  //RE
#define MAX485_RE_NEG  5  //DE

// instantiate ModbusMaster object
ModbusMaster node2;

void preTransmission2()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_RE, 1);
}

void postTransmission2()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_RE, 0);
}

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
  
  // Modbus slave ID 
  node2.begin(2, mySerial);
  
  // Callbacks allow us to configure the RS485 transceiver correctly
  node2.preTransmission(preTransmission2);
  node2.postTransmission(postTransmission2);
}

void loop()
{
  uint8_t result;
  //uint16_t data[2]; // prepare variable of storage data from sensor
   
  Serial.println("Get SoilMoisture Data");
  result = node2.readHoldingRegisters(0x0000, 3); // Read 2 registers starting at 1)
  if (result == node2.ku8MBSuccess)
  {
    Serial.print("SoilMoisture: ");
    Serial.println(node2.getResponseBuffer(2)/10.0f);
  }
  delay(1000);
}
