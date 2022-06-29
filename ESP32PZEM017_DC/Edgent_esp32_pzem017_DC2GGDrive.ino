//============Change logs===================//
//Monitor battery and send it to google sheet
//============Change logs===================//
//==============New Blynk IoT===============//
#define BLYNK_TEMPLATE_ID ""  //Templete id from blynk.console
#define BLYNK_DEVICE_NAME ""  //Device Name from template in blynk.console
#define BLYNK_AUTH_TOKEN ""   //Auth token from blynk.console
#define BLYNK_FIRMWARE_VERSION        "0.2.0" 
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_WROVER_BOARD
#include "BlynkEdgent.h"
#include <SimpleTimer.h>
SimpleTimer timer;
//==============New Blynk IoT===============//

//==================NTP=====================//
#include <NTPClient.h>
#include <WiFiUdp.h>
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;
//==================NTP=====================//

//============Senddata2GGSheet=================//
#include <WiFi.h>
#include <HTTPClient.h>
const char* host = "script.google.com";
const char* httpsPort = "443";
String GAS_ID = ""; //Google Script id from deploy app
//============Senddata2GGSheet=================//

//==============MCU Digital Pin=============//
#define Relay1_ledblynk           26
//#define Relay2_Pump             25
//Relay3  N/A                     33
//Relay4  N/A                     32
//==============MCU Digital Pin=============//

//============Blynk Virtual Pin============//
//V1    Voltage (Volt)
//V2    Current (AH)
//V3    Power (Watt)
//V4    Energy (Watt/Hour)
//V5    Reset Energy
//V21    CurrentDate
//V22    CurrentTime
//============Blynk Virtual Pin============//

//==================Modbus================//
#include <ModbusMaster.h>
///////// PIN /////////
#define MAX485_DE             18    //DE
#define MAX485_RE_NEG         19    //RE
#define RX2                   16    //RO,RX
#define TX2                   17    //DI,TX
// instantiate ModbusMaster object
ModbusMaster node1;       //Battery
//==================Modbus================//

//================Pzem017=================//
static uint8_t pzemSlaveAddr = 0x01;    //Pzem017 slave id 1
static uint16_t NewshuntAddr = 0x0000; // shunt. Default 0x0000 is 100A, replace to "0x0001" if using 50A shunt, 0x0002 is for 200A, 0x0003 is for 300A
float PZEMVoltage = 0;
float PZEMCurrent = 0;
float PZEMPower = 0;
float PZEMEnergy = 0;
unsigned long startMillisPZEM;
unsigned long currentMillisPZEM;
const unsigned long periodPZEM = 1500;
unsigned long startMillisReadData;
unsigned long currentMillisReadData;
const unsigned long periodReadData = 1000;
int ResetEnergy = 0;
int Reset_Energy = 0;
int a = 1;
unsigned long startMillis1;

//================Pzem017=================//

//=====Modbus Pre & Post Transmission1====//
void preTransmission1()
{
if (millis() - startMillis1 > 5000)
{
digitalWrite(MAX485_DE, 1);
digitalWrite(MAX485_RE_NEG, 1);
delay(1);
}
}
void postTransmission1()
{
if (millis() - startMillis1 > 5000)
{
delay(3);
digitalWrite(MAX485_DE, 0);
digitalWrite(MAX485_RE_NEG, 0);
}
}
//=====Modbus Pre & Post Transmission1====//

//==============Setup Function============//
void setup()
{
  //Modbus pinMode
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N2, RX2, TX2); //RX2=16,RO ,TX2=17, DI
 startMillisPZEM = millis();
 
  node1.begin(pzemSlaveAddr, Serial2);
  node1.preTransmission(preTransmission1);
  node1.postTransmission(postTransmission1);
delay(1000);
startMillisReadData = millis();
// Setup Pin Mode
  pinMode(Relay1_ledblynk,OUTPUT);      // ESP32 PIN gpio26
// ESP32 PIN GPIO25
// ESP32 PIN GPIO32   
// ESP32 PIN GPIO33         
  
  // Set Defult Relay Status
  digitalWrite(Relay1_ledblynk,LOW);           // ESP32 PIN gpio26
// ESP32 PIN GPIO25
// ESP32 PIN GPIO32   
// ESP32 PIN GPIO33    
  
  BlynkEdgent.begin();
  timer.setInterval(30000L,datetime); //อ่านค่าวัน/เวลา ทุกๆ 30 วินาที
  timer.setInterval(10000L, Pzem017); //อ่านค่า Pzem017 ทุกๆ 10 วินาที
  timer.setInterval(60000L,sendData2GGSheet); //ส่งค่าเซ็นเซอร์ขึ้น google sheet ทุกๆ 1 นาที
}
//==============Setup Function============//

//============Read Pzem017================//
void Pzem017(){
{
if ((millis() - startMillis1 >= 10000) && (a == 1))
{
setShunt(pzemSlaveAddr);
changeAddress(0XF8, pzemSlaveAddr);
a = 0;
}
currentMillisPZEM = millis();
if (currentMillisPZEM - startMillisPZEM >= periodPZEM)
{
uint8_t result;
result = node1.readInputRegisters(0x0000, 6);
if (result == node1.ku8MBSuccess)
{
uint32_t tempdouble = 0x00000000;
PZEMVoltage = node1.getResponseBuffer(0x0000) / 100.0;
PZEMCurrent = node1.getResponseBuffer(0x0001) / 100.0;
tempdouble = (node1.getResponseBuffer(0x0003) << 16) + node1.getResponseBuffer(0x0002);
PZEMPower = tempdouble / 10.0;
tempdouble = (node1.getResponseBuffer(0x0005) << 16) + node1.getResponseBuffer(0x0004);
PZEMEnergy = tempdouble;
Serial.print("Vdc : "); Serial.print(PZEMVoltage); Serial.println(" V ");
Serial.print("Idc : "); Serial.print(PZEMCurrent); Serial.println(" A ");
Serial.print("Power : "); Serial.print(PZEMPower); Serial.println(" W ");
Serial.print("Energy : "); Serial.print(PZEMEnergy); Serial.println(" Wh ");
}
Blynk.virtualWrite(V1, PZEMVoltage);  //V1 Display Voltage
Blynk.virtualWrite(V2, PZEMCurrent);  //V2 Display Current
Blynk.virtualWrite(V3, PZEMPower);   //V3 Display Power
Blynk.virtualWrite(V4, PZEMEnergy);    //V4 Energy 
startMillisReadData = millis();
}
}
}
//============Read Pzem017================//

//==========Set Shunt address=============//
void setShunt(uint8_t slaveAddr)
{
static uint8_t SlaveParameter = 0x06;
static uint16_t registerAddress = 0x0000;
uint16_t u16CRC = 0xFFFF;
u16CRC = crc16_update(u16CRC, slaveAddr);
u16CRC = crc16_update(u16CRC, SlaveParameter);
u16CRC = crc16_update(u16CRC, highByte(registerAddress));
u16CRC = crc16_update(u16CRC, lowByte(registerAddress));
u16CRC = crc16_update(u16CRC, highByte(NewshuntAddr));
u16CRC = crc16_update(u16CRC, lowByte(NewshuntAddr));
preTransmission1();
Serial2.write(slaveAddr);
Serial2.write(SlaveParameter);
Serial2.write(highByte(registerAddress));
Serial2.write(lowByte(registerAddress));
Serial2.write(highByte(NewshuntAddr));
Serial2.write(lowByte(NewshuntAddr));
Serial2.write(lowByte(u16CRC));
Serial2.write(highByte(u16CRC));
delay(10);
postTransmission1();
delay(100);
}
//==========Set Shunt address=============//

//==========Change pzem Address===========//
void changeAddress(uint8_t OldslaveAddr, uint8_t NewslaveAddr)
{
static uint8_t SlaveParameter = 0x06;
static uint16_t registerAddress = 0x0002;
uint16_t u16CRC = 0xFFFF;
u16CRC = crc16_update(u16CRC, OldslaveAddr);
u16CRC = crc16_update(u16CRC, SlaveParameter);
u16CRC = crc16_update(u16CRC, highByte(registerAddress));
u16CRC = crc16_update(u16CRC, lowByte(registerAddress));
u16CRC = crc16_update(u16CRC, highByte(NewslaveAddr));
u16CRC = crc16_update(u16CRC, lowByte(NewslaveAddr));
preTransmission1();
Serial2.write(OldslaveAddr);
Serial2.write(SlaveParameter);
Serial2.write(highByte(registerAddress));
Serial2.write(lowByte(registerAddress));
Serial2.write(highByte(NewslaveAddr));
Serial2.write(lowByte(NewslaveAddr));
Serial2.write(lowByte(u16CRC));
Serial2.write(highByte(u16CRC));
delay(10);
postTransmission1();
delay(100);
}

//Reset_Energy

BLYNK_WRITE(V5)
{
if (param.asInt() == 1)
{
Reset_Energy = Reset_Energy + 1;
Serial.println(Reset_Energy);
if (Reset_Energy > 2) {
Serial.println("DCresetEnergy");
uint16_t u16CRC = 0xFFFF;
static uint8_t resetCommand = 0x42;
uint8_t slaveAddr = pzemSlaveAddr;
u16CRC = crc16_update(u16CRC, slaveAddr);
u16CRC = crc16_update(u16CRC, resetCommand);
preTransmission1();
Serial2.write(slaveAddr);
Serial2.write(resetCommand);
Serial2.write(lowByte(u16CRC));
Serial2.write(highByte(u16CRC));
delay(10);
postTransmission1();
delay(100);
}
}
}

//==========Send data to google sheet=========//
void sendData2GGSheet() {

uint32_t tempdouble = 0x00000000;
PZEMVoltage = node1.getResponseBuffer(0x0000) / 100.0;
PZEMCurrent = node1.getResponseBuffer(0x0001) / 100.0;
tempdouble = (node1.getResponseBuffer(0x0003) << 16) + node1.getResponseBuffer(0x0002);
PZEMPower = tempdouble / 10.0;
tempdouble = (node1.getResponseBuffer(0x0005) << 16) + node1.getResponseBuffer(0x0004);
PZEMEnergy = tempdouble;
  
  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + GAS_ID + "/exec?BVoltage=" + PZEMVoltage + "&BCurrent=" + PZEMCurrent + "&BPower=" + PZEMPower + "&BEnergy=" + PZEMEnergy;
  //Serial.print(url);
  Serial.println("Posting Battery data to Google Sheet");
  //---------------------------------------------------------------------
  //starts posting data to google sheet
  http.begin(url.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);
  //---------------------------------------------------------------------
  //getting response from google sheet
  String payload;
  if (httpCode > 0) {
    payload = http.getString();
    Serial.println("Payload: " + payload);
  }
  //---------------------------------------------------------------------
  http.end();
}
//==========Send data to google sheet=========//

//==Display Date&Time==//
void datetime() {

// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(25200);

  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.print("DATE: ");
  Serial.println(dayStamp);
  Blynk.virtualWrite(V21, dayStamp);
  
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);
  Blynk.virtualWrite(V22, timeStamp);  
  delay(1000);
}
//==Display Date&Time==//

//=============Blynk conneted============//
BLYNK_CONNECTED()
{
 if (Blynk.connected())
 {
    digitalWrite(Relay1_ledblynk, HIGH);  //ledpin for check blynk connected 
    Serial.println("Blynk connected");
    Blynk.syncAll();
 }
}
//=============Blynk conneted============//

//==============Loop Function============// 
void loop() {      
  BlynkEdgent.run();   
  timer.run();
}
//==============Loop Function============// 
