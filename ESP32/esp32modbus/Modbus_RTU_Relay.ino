#define RXD2            16
#define TXD2            17
HardwareSerial rs485(1);
#include "modbusRTU.h"

void setup() {
  Serial.begin(9600);
  rs485.begin(9600, SERIAL_8N1, RXD2, TXD2);
}

void loop() {
  for(int channel=1; channel <= 8 ; channel++)
  { 
    relayControl_modbusRTU(1,channel,1);  //address, channel, on-off
    delay(500);
    Serial.printf("Relay Channel %d = %d\r\n",channel, relayStatus_modbusRTU(1,channel));
    delay(500);

    relayControl_modbusRTU(1,channel,0);  //address, channel, on-off
    delay(500);
    Serial.printf("Relay Channel %d = %d\r\n",channel, relayStatus_modbusRTU(1,channel));
    delay(500);
  }
}
