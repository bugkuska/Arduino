#define RXD2            16
#define TXD2            17
HardwareSerial rs485(1);
#include "modbusRTU.h"

void setup() {
  Serial.begin(9600);
  rs485.begin(9600, SERIAL_8N1, RXD2, TXD2);
}

void loop() {
  int id3 = 3;
  float temp3 = sht20ReadTemp_modbusRTU(id3);
  float humi3 = sht20ReadHumi_modbusRTU(id3);
 
  Serial.printf("Info: sht20[0x03] temperature3 = %.1f\r\n",temp3);
  vTaskDelay(1000);
  Serial.printf("Info: sht20[0x03] humidity3 = %.1f\r\n",humi3);
  vTaskDelay(1000); 

  int id4 = 4;
  float temp4 = sht20ReadTemp_modbusRTU(id4);
  float humi4 = sht20ReadHumi_modbusRTU(id4);
 
  Serial.printf("Info: sht20[0x04] temperature4 = %.1f\r\n",temp4);
  vTaskDelay(1000);
  Serial.printf("Info: sht20[0x04] humidity4 = %.1f\r\n",humi4);
  vTaskDelay(1000); 
}
