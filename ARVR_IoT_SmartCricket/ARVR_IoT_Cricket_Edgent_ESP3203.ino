//==============New Blynk IoT===============//
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_FIRMWARE_VERSION        "0.3.0" 
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_WROVER_BOARD
#include "BlynkEdgent.h"
#include <SimpleTimer.h>
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

//==MCU Digital Pin Check blynk connected===//
#define ledblynk           15
//==MCU Digital Pin Check blynk connected===//

//============Blynk Virtual Pin============//
//V1    sw1_pevap       ปั้มดูด
//V2    sw2_fevap       พัดลมดูด
//V3    sw3_airflow     พัดลมเป่า
//V4    sw4_heater
//V5    Humi1
//V6    Temp1
//V7    Light1
//V8    Windspeed
//V9    CurrentDate
//V10    CurrentTime
//V11   Auto&Manual evap
//V12   Slider temperature
//V13   Auto&Manual Airflow
//V14   Slider Windspeed
//============Blynk Virtual Pin============//

//===========ปุ่ม เปิด-ปิด sw1_pevap============//
//sw1 ต่อโหลดปั้มดูดน้ำ Cooling pad
//V11 Auto&Manual pevap
//V12 Slider temperature
//Manual & Auto Switch pevap
#define Relay1_sw1_pevap               26
#define Widget_Btn_sw1_pevap           V1     
//Slider for set temp limit
bool switchStatus1 = 0; // 0 = manual,1=auto
int TempLevelLimit1 = 0;
bool manualSwitch1 = 0;
//===========ปุ่ม เปิด-ปิด sw1_pevap============//
     
//=========ปุ่ม เปิด-ปิด sw2_fevap===========//
#define Relay2_sw2_fevap              25
#define Widget_Btn_sw2_fevap          V2          
//=========ปุ่ม เปิด-ปิด sw2_fevap===========//

//=========ปุ่ม เปิด-ปิด sw3_airflow===========//
//V13 Auto&Manual Air Flow พัดลมเป่า
//V14 Slider Windspeed
#define Relay3_sw3_airflow            33
#define Widget_Btn_sw3_airflow        V3          
//Slider for set Windspeed limit
bool switchStatus3 = 0; // 0 = manual,1=auto
int WindLevellimit3 = 0;
bool manualSwitch3 = 0;
//=========ปุ่ม เปิด-ปิด sw3_airflow===========//


//==========ปุ่ม เปิด-ปิด sw4_heater============//
//V15 Auto&Manual Heater
//V16 Slider Temperature
#define Relay4_sw4_heater             32
#define Widget_Btn_sw4_heater         V4           
//Slider for set Temperature limit
bool switchStatus4 = 0; // 0 = manual,1=auto
int TempLevellimit4 = 0;
bool manualSwitch4 = 0;       
//==========ปุ่ม เปิด-ปิด sw4_heater============//

//==================Modbus================//
#include <ModbusMaster.h>
///////// PIN /////////
#define MAX485_DE             18    //DE
#define MAX485_RE_NEG         19    //RE
#define RX2                   16    //RO,RX
#define TX2                   17    //DI,TX
// instantiate ModbusMaster object
ModbusMaster node1;
ModbusMaster node2;
//==================Modbus================//

//====Modbus Pre & Post Transmission1====//
void preTransmission1()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}
void postTransmission1()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}
//====Modbus Pre & Post Transmission1====//

//====Modbus Pre & Post Transmission2====//
void preTransmission2()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}
void postTransmission2()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}
//====Modbus Pre & Post Transmission2====//

//=============Setup Function============//
void setup()
{
  //Modbus pinMode
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2); //RX2=16,RO ,TX2=17, DI
 
  node1.begin(1, Serial2);
  node1.preTransmission(preTransmission1);
  node1.postTransmission(postTransmission1);

  node2.begin(2, Serial2);
  node2.preTransmission(preTransmission2);
  node2.postTransmission(postTransmission2);
  
  // Setup Pin Mode
  pinMode(Relay1_sw1_pevap,OUTPUT);         // ESP32 PIN gpio26
  pinMode(Relay2_sw2_fevap,OUTPUT);         // ESP32 PIN GPIO25
  pinMode(Relay3_sw3_airflow,OUTPUT);       // ESP32 PIN GPIO33   
  pinMode(Relay4_sw4_heater,OUTPUT);        // ESP32 PIN GPIO32 
  pinMode(ledblynk,OUTPUT);                 // ESP32 PIN GPIO15         
  
  // Set Defult Relay Status
  digitalWrite(Relay1_sw1_pevap,LOW);       // ESP32 PIN gpio26
  digitalWrite(Relay2_sw2_fevap,LOW);       // ESP32 PIN GPIO25
  digitalWrite(Relay3_sw3_airflow,LOW);     // ESP32 PIN GPIO33   
  digitalWrite(Relay4_sw4_heater,LOW);      // ESP32 PIN GPIO32   
  digitalWrite(ledblynk,HIGH);              // ESP32 PIN GPIO15  
  
  BlynkEdgent.begin();
  timer.setInterval(10000L,datetime);
  timer.setInterval(5000L, Gzws_temp1);
  timer.setInterval(5000L, Gzws_temp2);
  timer.setInterval(5000L, Gzws_lux);
  timer.setInterval(5000L,WindSensorData);
}
//=============Setup Function============//

//==Update switchStatus1 on Temperature==//
BLYNK_WRITE(V11)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}
//==Update switchStatus1 on Temperature==//
//======Update Temperature setting=======//
BLYNK_WRITE(V12)
{   
  TempLevelLimit1 = param.asInt(); // Get value as integer
}
//======Update Temperature setting=======//
//========Update manualSwitch1===========//
BLYNK_WRITE(V1)
{
  manualSwitch1 = param.asInt();
}
//========Update manualSwitch1===========//

//===========GZWS Temperature1===========//
void Gzws_temp1(){
  uint8_t result1; 
  float humi1 = (node1.getResponseBuffer(0)/10.0f);  
  float temp1 = (node1.getResponseBuffer(1)/10.0f); 
  Serial.println("GZWS Temperature");
  Serial.println("GZWS Humidity");
  result1 = node1.readHoldingRegisters(0x0000, 3); // Read 3 registers starting at 1)
  if (result1 == node1.ku8MBSuccess)
  {
    Serial.print("Humi1: ");
    Serial.println(node1.getResponseBuffer(0)/10.0f);
    Serial.print("Temp1: ");
    Serial.println(node1.getResponseBuffer(1)/10.0f);
  }
  delay(1000);
  Blynk.virtualWrite(V5,humi1);
  Blynk.virtualWrite(V6,temp1);

  if(switchStatus1)
  {
    // auto
    if(temp1 > TempLevelLimit1)
    {
        digitalWrite(Relay1_sw1_pevap, HIGH);  
        Blynk.virtualWrite(V1, 1); 
        digitalWrite(Relay2_sw2_fevap,HIGH);
        Blynk.virtualWrite(V2,1);
    }  
    else
    {
        digitalWrite(Relay1_sw1_pevap, LOW);
        Blynk.virtualWrite(V1, 0);    
        digitalWrite(Relay2_sw2_fevap,LOW);
        Blynk.virtualWrite(V2,0);
    }
  }
  else
  {
    if(manualSwitch1)
    {
        digitalWrite(Relay1_sw1_pevap, HIGH);       
    }
    else
    {
        digitalWrite(Relay1_sw1_pevap, LOW);
    }
    // manaul
  }
}
//===========GZWS Temperature1===========//

//==========Manual Switch fevap==========//
BLYNK_WRITE(Widget_Btn_sw2_fevap){
      int valuebtn3 = param.asInt();
      if(valuebtn3 == 1){
        digitalWrite(Relay2_sw2_fevap,HIGH);        
      }
       else{              
        digitalWrite(Relay2_sw2_fevap,LOW);        
     }
}
//==========Manual Switch fevap==========//

//==Update switchStatus4 on Temperature2==//
BLYNK_WRITE(V15)
{   
  switchStatus4 = param.asInt(); // Get value as integer
}
//==Update switchStatus4 on Temperature2==//
//======Update Temperature2 setting=======//
BLYNK_WRITE(V16)
{   
  TempLevellimit4 = param.asInt(); // Get value as integer
}
//======Update Temperature2 setting=======//
//========Update manualSwitch4===========//
BLYNK_WRITE(V4)
{
  manualSwitch4 = param.asInt();
}
//========Update manualSwitch4===========//

//==============GZWS Temperature2============//

//==========ปุ่ม เปิด-ปิด sw4_heater============//
void Gzws_temp2(){
  uint8_t result2; 
  float temp2 = (node1.getResponseBuffer(1)/10.0f); 
  Serial.println("GZWS Temperature");
  result2 = node1.readHoldingRegisters(0x0000, 3); // Read 3 registers starting at 1)
  if (result2 == node1.ku8MBSuccess)
  {
    Serial.print("Temp2: ");
    Serial.println(node1.getResponseBuffer(1)/10.0f);
  }
  delay(1000);

  if(switchStatus4)
  {
    // auto
    if(temp2 < TempLevellimit4)
    {
        digitalWrite(Relay4_sw4_heater, HIGH);  
        Blynk.virtualWrite(V4, 1); 
    }  
    else
    {
        digitalWrite(Relay4_sw4_heater, LOW);
        Blynk.virtualWrite(V4, 0); 
    }
  }
  else
  {
    if(manualSwitch4)
    {
        digitalWrite(Relay4_sw4_heater, HIGH);        
    }
    else
    {
        digitalWrite(Relay4_sw4_heater, LOW);
    }
    // manaul
  }
}
//==============GZWS Temperature2============//

//==============GZWS Lux Sensor=============//
void Gzws_lux(){
  uint8_t result1; 
  float light1 = (node1.getResponseBuffer(2));
  float light_per1;  
  Serial.println("GZWS Light");
  result1 = node1.readHoldingRegisters(0x0000, 3); // Read 3 registers starting at 1)
  if (result1 == node1.ku8MBSuccess)
  {
    light_per1 = (light1 = node1.getResponseBuffer(2));
    light_per1 = map(light_per1, 0,65535, 0,100);   
    Serial.print("Light1: ");
    Serial.println(light_per1);
  }
  delay(1000);
  Blynk.virtualWrite(V7,light_per1);
}
//==============GZWS Lux Sensor=============//

//====Update switchStatus3 on Windspeed====//
BLYNK_WRITE(V13)
{   
  switchStatus3 = param.asInt(); // Get value as integer
}
//====Update switchStatus3 on Windspeed====//
//=======Update Windspeed setting==========//
BLYNK_WRITE(V14)
{   
  WindLevellimit3 = param.asInt(); // Get value as integer
}
//=======Update Windspeed setting==========//
//==========Update manualSwitch3===========//
BLYNK_WRITE(V3)
{
  manualSwitch3 = param.asInt();
}
//==========Update manualSwitch3===========//

//==============WindSpeed Sensor===========//
void WindSensorData(){
uint8_t result;
  float wind01 = (node2.getResponseBuffer(0)); //Change m/s to km/h= m/s *3.6, Change km/h to m/s = km/h /3.6 
  Serial.println("Get Wind Speed Data");
   result = node2.readHoldingRegisters(0x0000, 1); // Read 2 registers starting at 1)
  if (result == node2.ku8MBSuccess)
  {
    Serial.print("Winspeed: ");
    Serial.println(node2.getResponseBuffer(0));
  }
  delay(1000);
  Blynk.virtualWrite(V8, wind01);

  if(switchStatus3)
  {
    // auto
    if(wind01 < WindLevellimit3)
    {
        digitalWrite(Relay3_sw3_airflow, HIGH);  
        Blynk.virtualWrite(V3, 1); 
    }  
    else
    {
        digitalWrite(Relay3_sw3_airflow, LOW);
        Blynk.virtualWrite(V3, 0); 
    }
  }
  else
  {
    if(manualSwitch3)
    {
        digitalWrite(Relay3_sw3_airflow, HIGH);       
    }
    else
    {
        digitalWrite(Relay3_sw3_airflow, LOW);
    }
    // manaul
  }
}
//==============WindSpeed Sensor===========//

//===========Display Date&Time=============//
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
  Blynk.virtualWrite(V9, dayStamp);
  
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);
  Blynk.virtualWrite(V10, timeStamp);  
  delay(1000);
}
//===========Display Date&Time===========//

//=============Blynk conneted============//
BLYNK_CONNECTED()
{
 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(ledblynk, LOW);  //ledpin for check blynk connected 
    Serial.println("Blynk connected");
    Blynk.syncAll();
 }
}
//=============Blynk conneted============//

//==============Loop Function============// 
void loop() {      
  BlynkEdgent.run();   
  if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();
}
//==============Loop Function============// 
