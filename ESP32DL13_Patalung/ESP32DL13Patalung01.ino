//==============New Blynk IoT===============//
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_FIRMWARE_VERSION        "0.1.0" 
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_WROVER_BOARD
#include "BlynkEdgent.h"
#include <SimpleTimer.h>
//==============New Blynk IoT===============//

//===========MCU Digital Pin===========//
//Relay1_sw1_fan                      26
//Relay2_sw2_ultra                    25
//Relay3_ledblynk                     33
//Relay4_sw4_light                    32
//MAX485 RO=RX                        16 
//MAX485 DI=TX                        17
//MAX485_RE_NEG=DE                    18
//MAX485_RE                           19
//===========MCU Digital Pin===========//

//=========Blynk Virtual Pin===========//
//V1  ปุ่ม เปิด-ปิด พัดลม 220VAC
//V2  ปุ่ม เปิด-ปิด ตัวพ่นหมอกอัลตร้าโซนิค 220VAC
//V3  ปุ่ม เปิด-ปิด ไฟ 220VAC
//V5  Temperature
//V6  Humidity
//V9  Date
//V10 Time
//=========Blynk Virtual Pin===========//

//==================NTP================//
#include <NTPClient.h>
#include <WiFiUdp.h>
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;
//==================NTP================//

//================Modbus===============//
#include <ModbusMaster.h>
#define MAX485_DE             18    //DE
#define MAX485_RE_NEG         19    //RE
#define RX2                   16    //RO
#define TX2                   17    //DI
// instantiate ModbusMaster object
ModbusMaster node1;
//================Modbus===============//

//ปุ่ม เปิด-ปิด พัดลม 220VAC
//V11 Auto&Manual cfan
//V12 Slider temperature
//Manual & Auto Switch cfan
#define Relay1_sw1_fan        26
#define Widget_Btn_sw1_fan    V1     
//Slider for set temp limit
bool switchStatus1 = 0; // 0 = manual,1=auto
int TempLevelLimit1 = 0;
bool manualSwitch1 = 0;
     
//ปุ่ม เปิด-ปิด ตัวพ่นละอองน้ำ
//V13 Auto&Manual Pump
//V14 Slider Humidity
#define Relay2_sw2_pump      25
#define Widget_Btn_sw2_pump  V2          
//Slider for set Humi limit
bool switchStatus2 = 0; // 0 = manual,1=auto
int HumiLevellimit = 0;
bool manualSwitch2 = 0;

#define Relay3_ledblynk       33

//==Modbus Pre & Post Transmission==//
void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}
void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}
//==Modbus Pre & Post Transmission==//

//=====Setup Function=====//
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
  node1.preTransmission(preTransmission);
  node1.postTransmission(postTransmission);

  // Setup Pin Mode
  pinMode(Relay1_sw1_fan,OUTPUT);             // NODEMCU PIN gpio26
  pinMode(Relay2_sw2_pump,OUTPUT);            // NODEMCU PIN gpio25
  pinMode(Relay3_ledblynk,OUTPUT);            // NODEMCU PIN gpio32   
  //pinMode(Relay4_sw4_light,OUTPUT);         // NODEMCU PIN GPIO33         
  
  // Set Defult Relay Status
  digitalWrite(Relay1_sw1_fan,LOW);           // NODEMCU PIN gpio26
  digitalWrite(Relay2_sw2_pump,LOW);          // NODEMCU PIN gpio25
  digitalWrite(Relay3_ledblynk,LOW);          // NODEMCU PIN gpio32
  //digitalWrite(Relay4_sw3_light,LOW);       // NODEMCU PIN gpio33 
  
  BlynkEdgent.begin();
  timer.setInterval(10000L,datetime);
  timer.setInterval(5000L, dl13_temp);
  timer.setInterval(5000L, dl13_humi);
}
//=====Setup Function=====//

//Update switchStatus1 on Temperature
BLYNK_WRITE(V11)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}

// Update Temperature setting
BLYNK_WRITE(V12)
{   
  TempLevelLimit1 = param.asInt(); // Get value as integer
}

//Update manualSwitch
BLYNK_WRITE(V1)
{
  manualSwitch1 = param.asInt();
}

//=======DL13 Modbus Temperature=======//
void dl13_temp()
{
  uint8_t result;
  float temp = (node1.getResponseBuffer(0)/10.0f);
  //float humi = (node1.getResponseBuffer(1)/10.0f);

 
  Serial.println("Get Temperature Data");
  result = node1.readInputRegisters(0x040A, 2); // Read 2 registers starting at 1)
  if (result == node1.ku8MBSuccess)
  {
    Serial.print("Temperature: ");
    Serial.println(node1.getResponseBuffer(0)/10.0f);
  }
    Blynk.virtualWrite(V3, temp);
    delay(1000);

  if(switchStatus1)
  {
    // auto
    if(temp > TempLevelLimit1)
    {
        digitalWrite(Relay1_sw1_fan, HIGH);  
        Blynk.virtualWrite(V1, 1); 
    }  
    else
    {
        digitalWrite(Relay1_sw1_fan, LOW);
        Blynk.virtualWrite(V1, 0); 
    }
  }
  else
  {
    if(manualSwitch1)
    {
        digitalWrite(Relay1_sw1_fan, HIGH);        

    }
    else
    {
        digitalWrite(Relay1_sw1_fan, LOW);
    }
    // manaul
  }
}
//=======DL13 Modbus Temperature=======//


//Update switchStatus2 on Humidity
BLYNK_WRITE(V13)
{   
  switchStatus2 = param.asInt(); // Get value as integer
}

// Update Humidity setting
BLYNK_WRITE(V14)
{   
  HumiLevellimit = param.asInt(); // Get value as integer
}

// Update manualSwitch2
BLYNK_WRITE(V2)
{
  manualSwitch2 = param.asInt();
}

//=========DL13 Modbus Humidity=======//
void dl13_humi()
{
  uint8_t result;
  float humi = (node1.getResponseBuffer(1)/10.0f);

 
  Serial.println("Get Humidity Data");
  result = node1.readInputRegisters(0x040A, 2); // Read 2 registers starting at 1)
  if (result == node1.ku8MBSuccess)
  {
    Serial.print("Humidity: ");
    Serial.println(node1.getResponseBuffer(1)/10.0f);
  }
    Blynk.virtualWrite(V4, humi);
    delay(1000);

  if(switchStatus2)
  {
    // auto
    if(humi < HumiLevellimit)
    {
        digitalWrite(Relay2_sw2_pump, HIGH);  
        Blynk.virtualWrite(V2, 1);
    }  
    else
    {
        digitalWrite(Relay2_sw2_pump, LOW);
        Blynk.virtualWrite(V2, 0);
    }
  }
  else
  {
    if(manualSwitch2)
    {
        digitalWrite(Relay2_sw2_pump, HIGH); 
    }
    else
    {
        digitalWrite(Relay2_sw2_pump, LOW);
    }
    // manaul
  }
}
//=========DL13 Modbus Humidity=======//

//==========Display Date&Time=========//
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
//==========Display Date&Time=========//

//==========Blynk conneted===========//
BLYNK_CONNECTED()
{
 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(Relay3_ledblynk, HIGH);  //ledpin for check blynk connected 
    Serial.println("Blynk connected");
    Blynk.syncAll();
 }
}
//==========Blynk conneted===========//

//==========Loop Function============// 
void loop() {      
  BlynkEdgent.run();   
  if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();
}
//==========Loop Function============// 
