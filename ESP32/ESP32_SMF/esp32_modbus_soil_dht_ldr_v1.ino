//อ้างจาก https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/
////////////Define Virtual Pin/////////////////

////////////////////Valve1////////////////////
//V1 ไฟสถานะปุ่ม Valve1
//V2 ปุ่ม เปิด-ปิด Valve1
//V15 Soil 1
//V51 Auto&Manual valve1
//V52 Slider Valve1
////////////////////Valve1////////////////////

////////////////////Valve2////////////////////
//V3 ไฟสถานะปุ่ม Valve2
//V4 ปุ่ม เปิด-ปิด Valve2
//V16 Soil 2
//V53 Auto&Manual valve2
//V54 Slider Valve2
////////////////////Valve2////////////////////

////////////////////Valve3////////////////////
//V5 ไฟสถานะปุ่ม Valve3
//V6 ปุ่ม เปิด-ปิด Valve3
//V17 Soil 3
//V55 Auto&Manual valve3
//V56 Slider Valve3
////////////////////Valve3////////////////////

////////////////////Valve4////////////////////
//V7 ไฟสถานะปุ่ม Valve4
//V8 ปุ่ม เปิด-ปิด Valve4
//V18 Soil 4
//V57 Auto&Manual valve4
//V58 Slider Valve4
////////////////////Valve4////////////////////

/////////////////////Pump/////////////////////
//V9 ไฟสถานะปุ่ม pump
//V10 ปุ่ม เปิด-ปิด pump
/////////////////////Pump/////////////////////
/////////////////Cooling Fan1/////////////////
//V11 ไฟสถานะปุ่ม พัดลมระบายอากาศ
//V12 ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
//V30 Humidity1
//V31 Temperature1
/////////////////Cooling Fan1/////////////////

/////////////////Cooling Fan2/////////////////
//V13 ไฟสถานะปุ่ม พัดลมระบายอากาศ
//V14 ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
//V32 Humidity2
//V33 Temperature2
/////////////////Cooling Fan2/////////////////

/////////////////////LDR1/////////////////////
//V19 ปุ่มสถานะเปิดปิดไฟ1
//V20 ปุ่มเปิดปิดไฟ1
//V34 LDR1
//V63 Auto&Manual LDR1
//V64 Slider LDR1
/////////////////////LDR1/////////////////////

/////////////////////LDR2/////////////////////
//V21 ปุ่มสถานะเปิดปิดไฟ2
//V22 ปุ่มเปิดปิดไฟ2
//V35 LDR2
//V65 Auto&Manual LDR2
//V66 Slider LDR2
/////////////////////LDR2/////////////////////

//////////////////Date&Time///////////////////
//V40 Current Time
//V41 Current Date
//////////////////Date&Time///////////////////

//////////////////////////////////////////////

////////////Define MCU Pin////////////////////
//D25   relay connect to pump
//D26   relay connect to ledbb
//D27   relay connect to ledfan1
//D14   relay connect to ledfan2
//D12   relay connect to valve1
//D13   relay connect to valve2
//D15   relay connect to valve3
//D23   relay connect to valve4
//D36   soil moisture sensor1   
//D39   soil moisture sensor2
//D34   soil moisture sensor3
//D35   soil moisture sensor4
//D32   LDR1
//D33   LDR2
//D16   Max485 DI (RX)
//D17   Max485 RO (TX) 
//D18   DHT11-1
//D19   DHT11-2
//D21   SDA
//D22   SCL
////////////Define MCU Pin////////////////////

#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <SPIFFS.h>
#include <WiFi.h>                 //https://github.com/esp8266/Arduino
#include <WiFiClient.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //Ver 5.13.4 https://github.com/bblanchon/ArduinoJson
#include <BlynkSimpleEsp32.h>     //  Blynk_Release_v0.6.1 
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
int blynkIsDownCount = 0;
BlynkTimer timer;

//Blynk credentials
char server[] = "blynk-cloud.com";
int port = 8442;
char blynk_token[34] = "";//ใส่ Blynk_token ของเราที่ Blynk ส่งมาทาง Email ตอน Create Project ใหม่

//Define pin for Digital Output  
#define pump            25              // Relay connect to pump 
#define ledbb           26              //Check blynk connected 
#define ledfan1         27              //Cooling Fan1
#define ledfan2         14              //Cooling Fan2
#define Relay1_Valve1   12              //valve1
#define Relay2_Valve2   13              //valve2
#define Relay3_Valve3   15              //valve3
#define Relay4_Valve4   23              //valve4 

//Define pin for analog input
#define INPUT_1_A1 36                   //Soil Moisture Sensor1
#define INPUT_2_A2 39                   //Soil Moisture Sensor2
#define INPUT_3_A3 34                   //Soil Moisture Sensor3
#define INPUT_4_A4 35                   //Soil Moisture Sensor4
#define INPUT_5_A5 32                   //LDR1
#define INPUT_6_A6 33                   //LDR2

//RTC Widget
#include <TimeLib.h>
#include <WidgetRTC.h>
WidgetRTC rtc;

//LCD 4 แถว
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

//Define DHT TYPE
#include "DHT.h"
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22
//DHT11-01
#define DHTPin1 18
DHT dht1(DHTPin1, DHTTYPE);
//DHT11-02
#define DHTPin2 19
DHT dht2(DHTPin2, DHTTYPE);

//Modbus
#define RXD2            16
#define TXD2            17
HardwareSerial rs485(1);
#include "modbusRTU.h"

//////////////////////////Valve1///////////////////////////////////
//V15 Valve1
//V51 Auto&Manual valve1
//V52 Slider Valve1
#define Widget_LED_Valve1 V1              //ไฟสถานะปุ่ม Valve1
#define Widget_Btn_Valve1 V2              //ปุ่ม เปิด-ปิด Valve1
WidgetLED LedBlynkValve1(Widget_LED_Valve1);
//Slider for set Light limit
bool switchStatus1 = 0; // 0 = manual,1=auto
int SoilsensorLimit1 = 0;
bool manualSwitch1 = 0;
//////////////////////////Valve1///////////////////////////////////

//////////////////////////Valve2///////////////////////////////////
//V16 Soil 2
//V53 Auto&Manual valve2
//V54 Slider Valve2
#define Widget_LED_Valve2 V3              //ไฟสถานะปุ่ม Valve2
#define Widget_Btn_Valve2 V4              //ปุ่ม เปิด-ปิด Valve2
WidgetLED LedBlynkValve2(Widget_LED_Valve2);
bool switchStatus2 = 0; // 0 = manual,1=auto
int SoilsensorLimit2 = 0;
bool manualSwitch2 = 0;
//////////////////////////Valve2///////////////////////////////////

//////////////////////////Valve3///////////////////////////////////
//V17 Soil 3
//V55 Auto&Manual valve3
//V56 Slider Valve3
#define Widget_LED_Valve3 V5             //ไฟสถานะปุ่ม Valve3
#define Widget_Btn_Valve3 V6             //ปุ่ม เปิด-ปิด Valve3
WidgetLED LedBlynkValve3(Widget_LED_Valve3);
bool switchStatus3 = 0; // 0 = manual,1=auto
int SoilsensorLimit3 = 0;
bool manualSwitch3 = 0;
//////////////////////////Valve3///////////////////////////////////

//////////////////////////Valve4///////////////////////////////////
//V18 Soil 4
//V57 Auto&Manual valve4
//V58 Slider Valve4
#define Widget_LED_Valve4 V7             //ไฟสถานะปุ่ม Valve4
#define Widget_Btn_Valve4 V8             //ปุ่ม เปิด-ปิด Valve4
WidgetLED LedBlynkValve4(Widget_LED_Valve4);
bool switchStatus4 = 0; // 0 = manual,1=auto
int SoilsensorLimit4 = 0;
bool manualSwitch4 = 0;
//////////////////////////Valve4///////////////////////////////////

//////////////////////////Pump1////////////////////////////////////
//ถ้าใช้ Pump ออโต้ ปุ่มสำหรับควบคุมปั้มก็ไม่จำเป็นต้องมี
#define Widget_LED_Pump V9         //ไฟสถานะปุ่ม pump
#define Widget_Btn_Pump V10         //ปุ่ม เปิด-ปิด pump
WidgetLED LedBlynkPump(Widget_LED_Pump);
//////////////////////////Pump1////////////////////////////////////

//////////////////////Cooling Fan1/////////////////////////////////
//V30 Humidity1
//V31 Temperature1
//Cooling Fan1
#define Widget_LED_Fan1 V11        //ไฟสถานะปุ่ม พัดลมระบายอากาศ
#define Widget_Btn_Fan1 V12        //ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
WidgetLED LedBlynkFan1(Widget_LED_Fan1);
//////////////////////Cooling Fan1/////////////////////////////////

//////////////////////Cooling Fan2/////////////////////////////////
#define Widget_LED_Fan2 V13        //ไฟสถานะปุ่ม พัดลมระบายอากาศ
#define Widget_Btn_Fan2 V14        //ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
WidgetLED LedBlynkFan2(Widget_LED_Fan2);
//////////////////////Cooling Fan2/////////////////////////////////

///////////////////////////LDR1////////////////////////////////////
//V34 LDR1
//V63 Auto&Manual LDR1
//V64 Slider LDR1
#define Widget_LED_LDR1 V19        //ไฟสถานะปุ่มไฟ1
#define Widget_Btn_LDR1 V20        //ปุ่ม เปิด-ปิดไฟ1
WidgetLED LedBlynkLDR1(Widget_LED_LDR1);
bool switchStatus7 = 0; // 0 = manual,1=auto
int LDRsensorLimit7 = 0;
bool manualSwitch7 = 0;
///////////////////////////LDR1////////////////////////////////////

///////////////////////////LDR2////////////////////////////////////
//V35 LDR2
//V65 Auto&Manual LDR2
//V66 Slider LDR2
#define Widget_LED_LDR2 V21        //ไฟสถานะปุ่มไฟ2
#define Widget_Btn_LDR2 V22        //ปุ่ม เปิด-ปิดไฟ2
WidgetLED LedBlynkLDR2(Widget_LED_LDR2);
bool switchStatus8 = 0; // 0 = manual,1=auto
int LDRsensorLimit8 = 0;
bool manualSwitch8 = 0;
///////////////////////////LDR2////////////////////////////////////

///////callback notifying us of the need to save config////////////
bool shouldSaveConfig = false;
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
///////callback notifying us of the need to save config////////////

///////////////////////Setup Function/////////////////////////////
void setup() {
  // put your setup code here, to run once:
 Serial.begin(9600);
 rs485.begin(9600, SERIAL_8N1, RXD2, TXD2);
   
  // Setup Pin Mode
  pinMode(pump, OUTPUT);              
  pinMode(ledbb,OUTPUT);              
  pinMode(ledfan1,OUTPUT);    
  pinMode(ledfan2,OUTPUT);                       
  pinMode(Relay1_Valve1,OUTPUT);      
  pinMode(Relay2_Valve2,OUTPUT);     
  pinMode(Relay3_Valve3,OUTPUT);      
  pinMode(Relay4_Valve4,OUTPUT);      
  
  // Set Defult Relay Status
  digitalWrite(pump, LOW);            
  digitalWrite(ledbb,LOW);           
  digitalWrite(ledfan1,LOW);            
  digitalWrite(ledfan2,LOW);        
  digitalWrite(Relay1_Valve1,LOW);   
  digitalWrite(Relay2_Valve2,LOW);   
  digitalWrite(Relay3_Valve3,LOW);   
  digitalWrite(Relay4_Valve4,LOW);   

  //Begin read Humidity and Temperature ==> SHCT3
  dht1.begin();
  dht2.begin();
  
  //Begin Sync time
  rtc.begin();
  
  //*************************    การ อ่าน  เขียนค่า WiFi + Password ]ลงใน Node MCU ESP32   ************//
  //read configuration from FS json
  Serial.println("mounting FS...");//แสดงข้อความใน Serial Monitor

  if (SPIFFS.begin(true)) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(blynk_token, json["blynk_token"]);

        } else {
          Serial.println("failed to load json config");//แสดงข้อความใน Serial Monitor
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");//แสดงข้อความใน Serial Monitor
  }
  //end read
  Serial.println(blynk_token);

  //*************************   จบการ อ่าน  เขียนค่า WiFi + Password ]ลงใน Node MCU ESP32   **********//
  

  //**************************        AP AUTO CONNECT   ********************************************//
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_blynk_token);

  //wifiManager.resetSettings();
/*
  for (int i = 5; i > -1; i--) {  // นับเวลาถอยหลัง 5 วินาทีก่อนกดปุ่ม AP Config
    //digitalWrite(ledbb, HIGH);
    delay(500);
    //digitalWrite(ledbb, LOW);
    // delay(500);
    Serial.print (String(i) + " ");//แสดงข้อความใน Serial Monitor
   
  }

  if (digitalRead(AP_Config) == HIGH) {
    Serial.println("Button Pressed");//แสดงข้อความใน Serial Monitor
    // wifiManager.resetSettings();//ให้ล้างค่า SSID และ Password ที่เคยบันทึกไว้
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); //load the flash-saved configs
    esp_wifi_init(&cfg); //initiate and allocate wifi resources (does not matter if connection fails)
    delay(2000); //wait a bit
    if (esp_wifi_restore() != ESP_OK)
    {
      Serial.println("WiFi is not initialized by esp_wifi_init ");
    } else {
      Serial.println("WiFi Configurations Cleared!");
    }
    //continue
    //delay(1000);
    //esp_restart(); //just my reset configs routine...
  }
*/

  wifiManager.setTimeout(120);
  //ใช้ได้ 2 กรณี
  //1. เมื่อกดปุ่มเพื่อ Config ค่า AP แล้ว จะขึ้นชื่อ AP ที่เราตั้งขึ้น
  //   ช่วงนี้ให้เราทำการตั้งค่า SSID+Password หรืออื่นๆทั้งหมด ภายใน 60 วินาที ก่อน AP จะหมดเวลา
  //   ไม่เช่นนั้น เมื่อครบเวลา 60 วินาที MCU จะ Reset เริ่มต้นใหม่ ให้เราตั้งค่าอีกครั้งภายใน 60 วินาที
  //2. ช่วงไฟดับ Modem router + MCU จะดับทั้งคู่ และเมื่อมีไฟมา ทั้งคู่ก็เริ่มทำงานเช่นกัน
  //   โดยปกติ Modem router จะ Boot ช้ากว่า  MCU ทำให้ MCU กลับไปเป็น AP รอให้เราตั้งค่าใหม่
  //   ดังนั้น AP จะรอเวลาให้เราตั้งค่า 60 วินาที ถ้าไม่มีการตั้งค่าใดๆ เมื่อครบ 60 วินาที MCU จะ Reset อีกครั้ง
  //   ถ้า Modem router  Boot และใช้งานได้ภายใน 60 วินาที และหลังจากที่ MCU Resset และเริ่มทำงานใหม่
  //   ก็จะสามารถเชื่อมต่อกับ  Modem router ที่ Boot และใช้งานได้แล้ว  ได้  ระบบจะทำงานปกติ

  if (!wifiManager.autoConnect("IoTSmartfarming","password")) {
    Serial.println("failed to connect and hit timeout");//แสดงข้อความใน Serial Monitor
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();//แก้ เดิม ESP.reset(); ใน Esp8266
    delay(5000);
  }

  Serial.println("Connected.......OK!)");//แสดงข้อความใน Serial Monitor
  strcpy(blynk_token, custom_blynk_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["blynk_token"] = blynk_token;
    File configFile = SPIFFS.open("/config.json", "w");

      if (!configFile) {
        Serial.println("failed to open config file for writing");//แสดงข้อความใน Serial Monitor
      }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  //**************************    จบ    AP AUTO CONNECT   *****************************************//
  
  Serial.println("local ip"); //แสดงข้อความใน Serial Monitor
  delay(100);
  Serial.println(WiFi.localIP());//แสดงข้อความใน Serial Monitor

  //MQTT
  // client.setServer(mqtt_server, 1883);
   
  // Blynk.config(blynk_token);////เริ่มการเชื่อมต่อ Blynk Server แบบปกติ
  //Blynk.config(blynk_token, server, port);
  Blynk.config(blynk_token);
  timer.setInterval(5000L,dhtSensorData1);
  timer.setInterval(5000L,dhtSensorData2);
  timer.setInterval(5000L,LDR1);
  timer.setInterval(5000L,LDR2);
  timer.setInterval(3000L, getSoilMoisterData1);
  timer.setInterval(3000L, getSoilMoisterData2);
  timer.setInterval(3000L, getSoilMoisterData3);
  timer.setInterval(3000L, getSoilMoisterData4);  
  timer.setInterval(1000L, clockDisplay);
  timer.setInterval(5000L, reconnectblynk);  //Function reconnect  
}
/////////////////////////////End Setup//////////////////////////////

////////////////////////////Blynk conneted//////////////////////////
BLYNK_CONNECTED()
{
 Blynk.syncAll();
 
 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(ledbb,HIGH);
    Serial.println("ledbb on");
 }
}
////////////////////////////Blynk conneted//////////////////////////

//////////////////////////Reconnect to blynk////////////////////////
void reconnectblynk()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    Blynk.syncAll();
    digitalWrite(ledbb, HIGH); //ledpin for check blynk connected 
  }
}
//////////////////////////Reconnect to blynk////////////////////////

///////////////Update switchStatus1 on Soil value1//////////////////
BLYNK_WRITE(V51)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}
///////////////Update switchStatus1 on Soil value1//////////////////

////////////////////Update Soil1 setting////////////////////////////
BLYNK_WRITE(V52)
{   
  SoilsensorLimit1 = param.asInt(); // Get value as integer
}
////////////////////Update Soil1 setting////////////////////////////

/////////////////////Update manualSwitch///////////////////////////
BLYNK_WRITE(V2)
{
  manualSwitch1 = param.asInt();
}
/////////////////////Update manualSwitch///////////////////////////

///////////////////SoilMoisture Sensor1////////////////////////////
void getSoilMoisterData1(){
  float moisture_percentage1;
  int sensor_analog1;
  sensor_analog1 = analogRead(INPUT_1_A1);
  //Serial.print("Law Soil data:");
  //Serial.println(sensor_analog1);
  moisture_percentage1 = ( 100 - ( (sensor_analog1/4095.00) * 100 ) );
  Blynk.virtualWrite(V15,(moisture_percentage1));
  //Serial.print("Moisture Percentage1 = ");
  //Serial.print(moisture_percentage1);
  //Serial.print("%\n\n");
  //delay(1000);
/*
//LCD Display
      lcd.setCursor(0,2);
      lcd.print("S1=");
      lcd.print(moisture_percentage1);
      lcd.print("%");
      lcd.print("|");
//End LCD Display
 */     
if(switchStatus1)
  {
    // auto
    if(moisture_percentage1 < SoilsensorLimit1)
    {
        digitalWrite(Relay1_Valve1, HIGH);  
        Blynk.virtualWrite(V2, 1);
                
        Blynk.setProperty(Widget_LED_Valve1, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์ว");
        LedBlynkValve1.on(); 
    }  
    else
    {
        digitalWrite(Relay1_Valve1, LOW);
        Blynk.virtualWrite(V2, 0);
        Blynk.virtualWrite(Widget_LED_Valve1, 0);
        Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์ว");                       
        LedBlynkValve1.off();  
    }
  }
  else
  {
    if(manualSwitch1)
    {
        digitalWrite(Relay1_Valve1, HIGH);        
        Blynk.setProperty(Widget_LED_Valve1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์ว");
        LedBlynkValve1.on(); 
    }
    else
    {
        digitalWrite(Relay1_Valve1, LOW);
        Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์ว");                       
        LedBlynkValve1.off();
    }
    // manaul
  }
}
///////////////////SoilMoisture Sensor1////////////////////////////

/////////////////Update switchStatus2 on Soil value2//////////////
BLYNK_WRITE(V53)
{   
  switchStatus2 = param.asInt(); // Get value as integer
}
/////////////////Update switchStatus2 on Soil value2//////////////

//////////////////////Update Soil2 setting////////////////////////
BLYNK_WRITE(V54)
{   
  SoilsensorLimit2 = param.asInt(); // Get value as integer
}
//////////////////////Update Soil2 setting////////////////////////

////////////////////////Update manualSwitch///////////////////////
BLYNK_WRITE(V4)
{
  manualSwitch2 = param.asInt();
}
////////////////////////Update manualSwitch///////////////////////

////////////////////SoilMoisture Sensor2/////////////////////////
void getSoilMoisterData2(){
  float moisture_percentage2;
  int sensor_analog2;
  sensor_analog2 = analogRead(INPUT_2_A2);
  //Serial.print("Law Soil data:");
  //Serial.println(sensor_analog2);
  moisture_percentage2 = ( 100 - ( (sensor_analog2/4095.00) * 100 ) );
  Blynk.virtualWrite(V16,(moisture_percentage2));
  //Serial.print("Moisture Percentage2 = ");
  //Serial.print(moisture_percentage2);
  //Serial.print("%\n\n");
  //delay(1000); 
/*
//LCD Display
      lcd.setCursor(10,2);
      lcd.print("S2=");
      lcd.print(moisture_percentage2);
      lcd.print("%");
//End LCD Display
*/
if(switchStatus2)
  {
    // auto
    if(moisture_percentage2 < SoilsensorLimit2)
    {
        digitalWrite(Relay2_Valve2, HIGH);  
        Blynk.virtualWrite(V4, 1);
                
        Blynk.setProperty(Widget_LED_Valve2, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve2, "label", "เปิดวาล์ว");
        LedBlynkValve2.on(); 
    }  
    else
    {
        digitalWrite(Relay2_Valve2, LOW);
        Blynk.virtualWrite(V4, 0);
        Blynk.virtualWrite(Widget_LED_Valve2, 0);
        Blynk.setProperty(Widget_LED_Valve2, "label", "ปิดวาล์ว");                       
        LedBlynkValve2.off();  
    }
  }
  else
  {
    if(manualSwitch2)
    {
        digitalWrite(Relay2_Valve2, HIGH);        
        Blynk.setProperty(Widget_LED_Valve2, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve2, "label", "เปิดวาล์ว");
        LedBlynkValve2.on(); 
    }
    else
    {
        digitalWrite(Relay2_Valve2, LOW);
        Blynk.setProperty(Widget_LED_Valve2, "label", "ปิดวาล์ว");                       
        LedBlynkValve2.off();
    }
    // manaul
  }
}
////////////////////SoilMoisture Sensor2/////////////////////////

/////////////////Update switchStatus3 on Soil value3/////////////
BLYNK_WRITE(V55)
{   
  switchStatus3 = param.asInt(); // Get value as integer
}
/////////////////Update switchStatus3 on Soil value3/////////////

////////////////////////Update Soil3 setting/////////////////////
BLYNK_WRITE(V56)
{   
  SoilsensorLimit3 = param.asInt(); // Get value as integer
}
////////////////////////Update Soil3 setting/////////////////////

///////////////////////Update manualSwitch///////////////////////
BLYNK_WRITE(V6)
{
  manualSwitch3 = param.asInt();
}
///////////////////////Update manualSwitch///////////////////////

////////////////////////SoilMoisture Sensor3/////////////////////
void getSoilMoisterData3(){
  float moisture_percentage3;
  int sensor_analog3;
  sensor_analog3 = analogRead(INPUT_3_A3);
 // Serial.print("Law Soil data:");
 // Serial.println(sensor_analog3);
  moisture_percentage3 = ( 100 - ( (sensor_analog3/4095.00) * 100 ) );
  Blynk.virtualWrite(V17,(moisture_percentage3));
 // Serial.print("Moisture Percentage3 = ");
  //Serial.print(moisture_percentage3);
 // Serial.print("%\n\n");
 // delay(1000);   
    
/* 
    long now = millis();
    if (now - lastMsg1 > 5000) {
    lastMsg1 = now;
  
    char soilmoisture_percent1[8];
    dtostrf(moisture_percentage1, 1, 2, soilmoisture_percent1);
    Serial.print("Soil Moisture Percentage1: ");
    Serial.println(soilmoisture_percent1);
    client.publish("esp32_kk5/soilmoisture/soil1", soilmoisture_percent1);
  }*/
  
  /*
//LCD Display
      lcd.setCursor(0,3);
      lcd.print("S3=");
      lcd.print(moisture_percentage3);
      lcd.print("%");
      lcd.print("|");
//End LCD Display
*/

if(switchStatus3)
  {
    // auto
    if(moisture_percentage3 < SoilsensorLimit3)
    {
        digitalWrite(Relay3_Valve3, HIGH);  
        Blynk.virtualWrite(V6, 1);
                
        Blynk.setProperty(Widget_LED_Valve3, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve3, "label", "เปิดวาล์ว");
        LedBlynkValve3.on(); 
    }  
    else
    {
        digitalWrite(Relay3_Valve3, LOW);
        Blynk.virtualWrite(V6, 0);
        Blynk.virtualWrite(Widget_LED_Valve3, 0);
        Blynk.setProperty(Widget_LED_Valve3, "label", "ปิดวาล์ว");                       
        LedBlynkValve3.off();  
    }
  }
  else
  {
    if(manualSwitch3)
    {
        digitalWrite(Relay3_Valve3, HIGH);        
        Blynk.setProperty(Widget_LED_Valve3, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve3, "label", "เปิดวาล์ว");
        LedBlynkValve3.on(); 
    }
    else
    {
        digitalWrite(Relay3_Valve3, LOW);
        Blynk.setProperty(Widget_LED_Valve3, "label", "ปิดวาล์ว");                       
        LedBlynkValve3.off();
    }
    // manaul
  }
}
////////////////////////SoilMoisture Sensor3/////////////////////

//////////////Update switchStatus4 on Soil value4////////////////
BLYNK_WRITE(V57)
{   
  switchStatus4 = param.asInt(); // Get value as integer
}
//////////////Update switchStatus4 on Soil value4////////////////

/////////////////////Update Soil4 setting////////////////////////
BLYNK_WRITE(V58)
{   
  SoilsensorLimit4 = param.asInt(); // Get value as integer
}
/////////////////////Update Soil4 setting////////////////////////

//////////////////////Update manualSwitch///////////////////////
BLYNK_WRITE(V8)
{
  manualSwitch4 = param.asInt();
}
//////////////////////Update manualSwitch///////////////////////

///////////////////////SoilMoisture Sensor4/////////////////////
void getSoilMoisterData4(){
  float moisture_percentage4;
  int sensor_analog4;
  sensor_analog4 = analogRead(INPUT_4_A4);
  //Serial.print("Law Soil data:");
  //Serial.println(sensor_analog4);
  moisture_percentage4 = ( 100 - ( (sensor_analog4/4095.00) * 100 ) );
  Blynk.virtualWrite(V18,(moisture_percentage4));
  //Serial.print("Moisture Percentage4 = ");
  //Serial.print(moisture_percentage4);
  //Serial.print("%\n\n");
  //delay(1000);   
    
/* 
    long now = millis();
    if (now - lastMsg1 > 5000) {
    lastMsg1 = now;
  
    char soilmoisture_percent1[8];
    dtostrf(moisture_percentage1, 1, 2, soilmoisture_percent1);
    Serial.print("Soil Moisture Percentage1: ");
    Serial.println(soilmoisture_percent1);
    client.publish("esp32_kk5/soilmoisture/soil1", soilmoisture_percent1);
  }*/
/*  
//LCD Display
      lcd.setCursor(10,3);
      lcd.print("S4=");
      lcd.print(moisture_percentage4);
      lcd.print("%");
//End LCD Display
*/
if(switchStatus4)
  {
    // auto
    if(moisture_percentage4 < SoilsensorLimit4)
    {
        digitalWrite(Relay4_Valve4, HIGH);  
        Blynk.virtualWrite(V8, 1);
                
        Blynk.setProperty(Widget_LED_Valve4, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve4, "label", "เปิดวาล์ว");
        LedBlynkValve4.on(); 
    }  
    else
    {
        digitalWrite(Relay4_Valve4, LOW);
        Blynk.virtualWrite(V8, 0);
        Blynk.virtualWrite(Widget_LED_Valve4, 0);
        Blynk.setProperty(Widget_LED_Valve4, "label", "ปิดวาล์ว");                       
        LedBlynkValve4.off();  
    }
  }
  else
  {
    if(manualSwitch4)
    {
        digitalWrite(Relay4_Valve4, HIGH);        
        Blynk.setProperty(Widget_LED_Valve4, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve4, "label", "เปิดวาล์ว");
        LedBlynkValve4.on(); 
    }
    else
    {
        digitalWrite(Relay4_Valve4, LOW);
        Blynk.setProperty(Widget_LED_Valve4, "label", "ปิดวาล์ว");                       
        LedBlynkValve4.off();
    }
    // manaul
  }
}
///////////////////////SoilMoisture Sensor4/////////////////////

//****BUTTON ON/OFF Cooling FAN1****//
BLYNK_WRITE(Widget_Btn_Fan1){
int valueFan1 = param.asInt();
  if(valueFan1 == 1){
    digitalWrite(ledfan1, HIGH);
    Blynk.setProperty(Widget_LED_Fan1, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Fan1, "label", "เปิดพัดลมระบายอากาศ");
    LedBlynkFan1.on();
    }
    else{              
    digitalWrite(ledfan1, LOW);
    Blynk.virtualWrite(Widget_Btn_Fan1, 0);
    Blynk.setProperty(Widget_LED_Fan1, "label", "ปิดพัดลมระบายอากาศ");
    LedBlynkFan1.off();                       
    }
}
//****BUTTON ON/OFF Cooling FAN1****//

//****BUTTON ON/OFF Cooling FAN2****//
BLYNK_WRITE(Widget_Btn_Fan2){
int valueFan2 = param.asInt();
  if(valueFan2 == 1){
    digitalWrite(ledfan2, HIGH);
    Blynk.setProperty(Widget_LED_Fan2, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Fan2, "label", "เปิดพัดลมระบายอากาศ");
    LedBlynkFan2.on();
    }
    else{              
    digitalWrite(ledfan2, LOW);
    Blynk.virtualWrite(Widget_Btn_Fan2, 0);
    Blynk.setProperty(Widget_LED_Fan2, "label", "ปิดพัดลมระบายอากาศ");
    LedBlynkFan2.off();                       
    }
}
//****BUTTON ON/OFF Cooling FAN2****//

///////////////DHT11-01//////////////
void dhtSensorData1(){
  float h1 = dht1.readHumidity();
  float t1 = dht1.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h1) || isnan(t1)){ 
 Serial.println("Read from DHT Sensor 1");
  return;
  }
  Blynk.virtualWrite(30, t1);
  Blynk.virtualWrite(31, h1);
  //******AUTO Cooling FAN*******
  if (t1 >= 33){       
    digitalWrite(ledfan1, HIGH);
    Blynk.virtualWrite(Widget_Btn_Fan1, 1);       
    Blynk.setProperty(Widget_LED_Fan1, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Fan1, "label", "พัดลมกำลังทำงาน");
    LedBlynkFan1.on();
    }   

  if (t1 < 33){
    digitalWrite(ledfan1, LOW);        
    Blynk.virtualWrite(Widget_Btn_Fan1, 0);
    Blynk.setProperty(Widget_LED_Fan1, "label", "ปิดพัดลมแล้ว");                        
    LedBlynkFan1.off();  
    }  
}
///////////////DHT11-01//////////////

///////////////DHT11-02//////////////
void dhtSensorData2(){
  float h2 = dht2.readHumidity();
  float t2 = dht2.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h2) || isnan(t2)){ 
 Serial.println("Read from DHT Sensor 2");
  return;
  }
  Blynk.virtualWrite(32, t2);
  Blynk.virtualWrite(33, h2);
  //******AUTO Cooling FAN*******
  if (t2 >= 33){       
    digitalWrite(ledfan2, HIGH);
    Blynk.virtualWrite(Widget_Btn_Fan2, 1);       
    Blynk.setProperty(Widget_LED_Fan2, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Fan2, "label", "พัดลมกำลังทำงาน");
    LedBlynkFan2.on();
    }   
  if (t2 < 33){
    digitalWrite(ledfan2, LOW);        
    Blynk.virtualWrite(Widget_Btn_Fan2, 0);
    Blynk.setProperty(Widget_LED_Fan2, "label", "ปิดพัดลมแล้ว");                        
    LedBlynkFan2.off();  
    }  
}
///////////////DHT11-02//////////////

//*****Update switchStatus7 on LDR1*****//
BLYNK_WRITE(V63)
{   
  switchStatus7 = param.asInt(); // Get value as integer
}
//*****Update switchStatus7 on LDR1*****//

//*****Update LDR1 setting*****//
BLYNK_WRITE(V64)
{   
  LDRsensorLimit7 = param.asInt(); // Get value as integer
}
//*****Update LDR1 setting*****//

//*****Update manualSwitch*****//
BLYNK_WRITE(V20)
{
  manualSwitch7 = param.asInt();
}
//*****Update manualSwitch*****//

//***********LDR1**************//
void LDR1(){
  float ldr_percentage1;
  int sensor_analog5;
  sensor_analog5 = analogRead(INPUT_5_A5);
  //Serial.print("Law LDR data1:");
  //Serial.println(sensor_analog5);
  ldr_percentage1 = ( 100 - ( (sensor_analog5/4096.00) * 100 ) );
  Blynk.virtualWrite(V34,(ldr_percentage1)); 
  //Serial.print("LDR Percentage1 = ");
  //Serial.print(ldr_percentage1);
  //Serial.print("%\n\n");
  //delay(1000);

  if(switchStatus7)
  {
    // auto
    if(ldr_percentage1 < LDRsensorLimit7)
    {
       // digitalWrite(ledfan2, HIGH);  
        relayControl_modbusRTU(2,1,1);
        Blynk.virtualWrite(V20, 1);
                
        Blynk.setProperty(Widget_LED_LDR1, "color", "#C70039");
        Blynk.setProperty(Widget_LED_LDR1, "label", "เปิดไฟ1");
        LedBlynkLDR1.on(); 

        //For Test Modbus 16ch relay
        relayControl_modbusRTU(3,1,1);
        relayControl_modbusRTU(3,2,1);
        relayControl_modbusRTU(3,3,1);
        relayControl_modbusRTU(3,4,1);
        relayControl_modbusRTU(3,5,1);
        relayControl_modbusRTU(3,6,1);
        relayControl_modbusRTU(3,7,1);
        relayControl_modbusRTU(3,8,1);
        relayControl_modbusRTU(3,9,1);
        relayControl_modbusRTU(3,10,1);
        relayControl_modbusRTU(3,11,1);
        relayControl_modbusRTU(3,12,1);
        relayControl_modbusRTU(3,13,1);
        relayControl_modbusRTU(3,14,1);
        relayControl_modbusRTU(3,15,1);
        relayControl_modbusRTU(3,16,1);
        //End test modbus relay
    }  
    else
    {
        //digitalWrite(ledfan2, LOW);
        relayControl_modbusRTU(2,1,0);
        Blynk.virtualWrite(V20, 0);
        Blynk.virtualWrite(Widget_LED_LDR1, 0);
        Blynk.setProperty(Widget_LED_LDR1, "label", "ปิดไฟ1");                       
        LedBlynkLDR1.off();      
        
        //For Test Modbus 16ch relay
        relayControl_modbusRTU(3,1,0);
        relayControl_modbusRTU(3,2,0);
        relayControl_modbusRTU(3,3,0);
        relayControl_modbusRTU(3,4,0);
        relayControl_modbusRTU(3,5,0);
        relayControl_modbusRTU(3,6,0);
        relayControl_modbusRTU(3,7,0);
        relayControl_modbusRTU(3,8,0);
        relayControl_modbusRTU(3,9,0);
        relayControl_modbusRTU(3,10,0);
        relayControl_modbusRTU(3,11,0);
        relayControl_modbusRTU(3,12,0);
        relayControl_modbusRTU(3,13,0);
        relayControl_modbusRTU(3,14,0);
        relayControl_modbusRTU(3,15,0);
        relayControl_modbusRTU(3,16,0);
        //End test modbus relay   
    }
  }
  else
  {
    if(manualSwitch7)
    {
        //digitalWrite(ledfan2, HIGH);    
        relayControl_modbusRTU(2,1,1);    
        Blynk.setProperty(Widget_LED_LDR1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_LDR1, "label", "เปิดไฟ1");
        LedBlynkLDR1.on(); 

        //For Test Modbus 16ch relay
        relayControl_modbusRTU(3,1,1);
        relayControl_modbusRTU(3,2,1);
        relayControl_modbusRTU(3,3,1);
        relayControl_modbusRTU(3,4,1);
        relayControl_modbusRTU(3,5,1);
        relayControl_modbusRTU(3,6,1);
        relayControl_modbusRTU(3,7,1);
        relayControl_modbusRTU(3,8,1);
        relayControl_modbusRTU(3,9,1);
        relayControl_modbusRTU(3,10,1);
        relayControl_modbusRTU(3,11,1);
        relayControl_modbusRTU(3,12,1);
        relayControl_modbusRTU(3,13,1);
        relayControl_modbusRTU(3,14,1);
        relayControl_modbusRTU(3,15,1);
        relayControl_modbusRTU(3,16,1);
        //End test modbus relay
        
    }
    else
    {
       // digitalWrite(ledfan2, LOW);
       relayControl_modbusRTU(2,1,0);
        Blynk.setProperty(Widget_LED_LDR1, "label", "ปิดไฟ1");                       
        LedBlynkLDR1.off();
        
        //For Test Modbus 16ch relay
        relayControl_modbusRTU(3,1,0);
        relayControl_modbusRTU(3,2,0);
        relayControl_modbusRTU(3,3,0);
        relayControl_modbusRTU(3,4,0);
        relayControl_modbusRTU(3,5,0);
        relayControl_modbusRTU(3,6,0);
        relayControl_modbusRTU(3,7,0);
        relayControl_modbusRTU(3,8,0);
        relayControl_modbusRTU(3,9,0);
        relayControl_modbusRTU(3,10,0);
        relayControl_modbusRTU(3,11,0);
        relayControl_modbusRTU(3,12,0);
        relayControl_modbusRTU(3,13,0);
        relayControl_modbusRTU(3,14,0);
        relayControl_modbusRTU(3,15,0);
        relayControl_modbusRTU(3,16,0);
        //End test modbus relay  
    }
    // manaul
  }
}
//***********LDR1**************//

//**Update switchStatus8 on LDR2**//
BLYNK_WRITE(V65)
{   
  switchStatus8 = param.asInt(); // Get value as integer
}
//**Update switchStatus8 on LDR2**//

//**Update LDR2 setting**//
BLYNK_WRITE(V66)
{   
  LDRsensorLimit8 = param.asInt(); // Get value as integer
}
//**Update LDR2 setting**//

//**Update manualSwitch**//
BLYNK_WRITE(V22)
{
  manualSwitch8 = param.asInt();
}
//**Update manualSwitch**//

//***********LDR2**************//
void LDR2(){
  float ldr_percentage2;
  int sensor_analog6;
  sensor_analog6 = analogRead(INPUT_6_A6);
  //Serial.print("Law LDR data2:");
  //Serial.println(sensor_analog6);
  ldr_percentage2 = ( 100 - ( (sensor_analog6/4096.00) * 100 ) );
  Blynk.virtualWrite(V35,(ldr_percentage2)); 
  //Serial.print("LDR Percentage2 = ");
  //Serial.print(ldr_percentage2);
  //Serial.print("%\n\n");
  //delay(1000);

  if(switchStatus7)
  {
    // auto
    if(ldr_percentage2 < LDRsensorLimit8)
    {
       // digitalWrite(ledfan2, HIGH); 
        relayControl_modbusRTU(2,2,1); 
        Blynk.virtualWrite(V22, 1);
                
        Blynk.setProperty(Widget_LED_LDR2, "color", "#C70039");
        Blynk.setProperty(Widget_LED_LDR2, "label", "เปิดไฟ2");
        LedBlynkLDR2.on(); 
    }  
    else
    {
        //digitalWrite(ledfan2, LOW);
        relayControl_modbusRTU(2,2,0); 
        Blynk.virtualWrite(V22, 0);
        Blynk.virtualWrite(Widget_LED_LDR2, 0);
        Blynk.setProperty(Widget_LED_LDR2, "label", "ปิดไฟ2");                       
        LedBlynkLDR2.off();  
    }
  }
  else
  {
    if(manualSwitch8)
    {
        //digitalWrite(ledfan2, HIGH);   
        relayControl_modbusRTU(2,2,1);      
        Blynk.setProperty(Widget_LED_LDR2, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_LDR2, "label", "เปิดไฟ2");
        LedBlynkLDR2.on(); 
    }
    else
    {
       // digitalWrite(ledfan2, LOW);
       relayControl_modbusRTU(2,2,0); 
        Blynk.setProperty(Widget_LED_LDR2, "label", "ปิดไฟ2");                       
        LedBlynkLDR2.off();
    }
    // manaul
  }
}
//***********LDR2**************//

//**Display Current Date/Time**//
void clockDisplay()
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  // Send time to the App
  Blynk.virtualWrite(V40, currentTime);
  // Send date to the App
  Blynk.virtualWrite(V41, currentDate);      
}
//**Display Current Date/Time**//

//**********Loop Function**********// 
void loop() {    
//LCD Display Data
  lcd.begin();
  lcd.backlight();    
  lcd.setCursor(0, 0);
  lcd.print("==IoT SmartFarming==");

//Display Current Date&Time
  String currentTime = String(hour()) + ":" + minute();
  String currentDate = String(day()) + "/" + month() + "/" + year();
  lcd.setCursor(0, 1); 
  lcd.print("DT="); 
  lcd.print(currentTime);
      
  lcd.setCursor(9, 1);
  lcd.print("|"); 
  lcd.print(currentDate);

//Display Soil Moisture Sensor1
  float moisture_percentage1;
  int sensor_analog1;
  sensor_analog1 = analogRead(INPUT_1_A1);
  moisture_percentage1 = ( 100 - ( (sensor_analog1/4095.00) * 100 ) );
  lcd.setCursor(0,2);
  lcd.print("S1=");
  lcd.print(moisture_percentage1);
  lcd.print("%");
  lcd.print("|");
//End Display Soil Moisture Sensor1

//Display Soil Moisture Sensor2
  float moisture_percentage2;
  int sensor_analog2;
  sensor_analog2 = analogRead(INPUT_2_A2);
  moisture_percentage2 = ( 100 - ( (sensor_analog2/4095.00) * 100 ) );
  lcd.setCursor(10,2);
  lcd.print("S2=");
  lcd.print(moisture_percentage2);
  lcd.print("%");
//End Display Soil Moisture Sensor2

//Display Soil Moisture Sensor3
  float moisture_percentage3;
  int sensor_analog3;
  sensor_analog3 = analogRead(INPUT_3_A3);
  moisture_percentage3 = ( 100 - ( (sensor_analog3/4095.00) * 100 ) );
  lcd.setCursor(0,3);
  lcd.print("S3=");
  lcd.print(moisture_percentage3);
  lcd.print("%");
  lcd.print("|");
//End Display Soil Moisture Sensor4

//Display Soil Moisture Sensor4
  float moisture_percentage4;
  int sensor_analog4;
  sensor_analog4 = analogRead(INPUT_4_A4);
  moisture_percentage4 = ( 100 - ( (sensor_analog4/4095.00) * 100 ) );
  lcd.setCursor(10,3);
  lcd.print("S4=");
  lcd.print(moisture_percentage4);
  lcd.print("%");
//End //Display Soil Moisture Sensor4
     
/*
  //MQTT
  if (!client.connected()) {
    reconnectmqtt();
  }
  //client.loop();
  if(!client.loop())
    client.connect("esp32_nodekk5");
 */
 
  if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();//ให้เวลาของ Blynk ทำงาน
}
//**********Loop Function**********// 
