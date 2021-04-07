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
//V59 Auto&Manual Cooling Fan1
//V60 Slider Temperature for Cooling Fan1
//V63 Slider Humidity for Cooling Fan1
/////////////////Cooling Fan1/////////////////

/////////////////////Lux Sensor/////////////////////
//V13 ปุ่มสถานะเปิดปิดหลังคา
//V14 ปุ่มเปิดปิดหลังคา
//V32 lux
//V61 Auto&Manual lux
//V62 Slider lux
/////////////////////Lux Sensor/////////////////////

//////////////////Date&Time///////////////////
//V40 Current Time
//V41 Current Date
//////////////////Date&Time///////////////////

////////////Define MCU Pin////////////////////
//D25   relay connect to pump
//D26   relay connect to ledbb
//D27   relay connect to ledfan1
//D14   relay connect to lux sensor
//D12   relay connect to valve1
//D13   relay connect to valve2
//D15   relay connect to valve3
//D23   relay connect to valve4
//D36   soil moisture sensor1   
//D39   soil moisture sensor2
//D34   soil moisture sensor3
//D35   soil moisture sensor4
//D32   ว่าง
//D33   ว่าง
//D16   Max485 DI (RX)
//D17   Max485 RO (TX) 
//D18   M485_DE
//D19   M485_RE_NEG
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

//Modbus
#include <ModbusMaster.h>
///////// PIN /////////
#define M485_DE      18//4
#define M485_RE_NEG  19//0
#define RX2 16
#define TX2 17
ModbusMaster node1;
void preTransmission()
{
  digitalWrite(M485_RE_NEG, 1);
  digitalWrite(M485_DE, 1);
}

void postTransmission()
{
  digitalWrite(M485_RE_NEG, 0);
  digitalWrite(M485_DE, 0);
}
//End Modbus

//Define pin for Digital Output  
#define pump            23              // Relay connect to pump 
#define ledbb           15              //Check blynk connected 
#define ledfan1         13              //Cooling Fan1
#define luxsensor       12              //luxsensor Sensor
#define Relay1_Valve1   14              //valve1
#define Relay2_Valve2   27              //valve2
#define Relay3_Valve3   26              //valve3
#define Relay4_Valve4   25              //valve4 

//Define pin for analog input
#define INPUT_1_A1 36                   //Soil Moisture Sensor1
#define INPUT_2_A2 39                   //Soil Moisture Sensor2
#define INPUT_3_A3 34                   //Soil Moisture Sensor3
#define INPUT_4_A4 35                   //Soil Moisture Sensor4

//RTC Widget
#include <TimeLib.h>
#include <WidgetRTC.h>
WidgetRTC rtc;

//LCD 4 แถว
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

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
//V59 Auto&Manual Cooling fan1
//V60 Slider Temperature to start cooling fan1
#define Widget_LED_Fan1 V11        //ไฟสถานะปุ่ม พัดลมระบายอากาศ
#define Widget_Btn_Fan1 V12        //ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
WidgetLED LedBlynkFan1(Widget_LED_Fan1);

bool switchStatus5 = 0; // 0 = manual,1=auto
int tempsensorLimit5 = 0;
bool manualSwitch5 = 0;
int humisensorLimit5 = 0;
//////////////////////Cooling Fan1/////////////////////////////////

//////////////////////Lux sensor/////////////////////////////////
//V32 lux
//V61 Auto&Manual Lux Sensor
//V62 Slider Lux Sensor
#define Widget_LED_Lux V13        //ไฟสถานะปุ่ม 
#define Widget_Btn_Lux V14        //ปุ่ม เปิด-ปิด 
WidgetLED LedBlynkLux(Widget_LED_Lux);

bool switchStatus6 = 0; // 0 = manual,1=auto
int luxsensorLimit6 = 0;
bool manualSwitch6 = 0;
//////////////////////Cooling Fan1/////////////////////////////////

//LED Status
boolean stateled1 = 0;
boolean prevStateled1 = 0;
boolean stateled2=0;
boolean prevStateled2 = 0;
boolean stateled3=0;
boolean prevStateled3 = 0;
boolean stateled4=0;
boolean prevStateled4 = 0;
boolean stateled5=0;
boolean prevStateled5 = 0;


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
  pinMode(M485_RE_NEG, OUTPUT);
  pinMode(M485_DE, OUTPUT);
  digitalWrite(M485_RE_NEG, 0);
  digitalWrite(M485_DE, 0);
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);
  node1.begin(1, Serial2);
  node1.preTransmission(preTransmission);
  node1.postTransmission(postTransmission);
    
  // Setup Pin Mode
  pinMode(pump, OUTPUT);              
  pinMode(ledbb,OUTPUT);              
  pinMode(ledfan1,OUTPUT);    
  pinMode(luxsensor,OUTPUT);                       
  pinMode(Relay1_Valve1,OUTPUT);      
  pinMode(Relay2_Valve2,OUTPUT);     
  pinMode(Relay3_Valve3,OUTPUT);      
  pinMode(Relay4_Valve4,OUTPUT);      
  
  // Set Defult Relay Status
  digitalWrite(pump, LOW);            
  digitalWrite(ledbb,LOW);           
  digitalWrite(ledfan1,LOW);            
  digitalWrite(luxsensor,LOW);        
  digitalWrite(Relay1_Valve1,LOW);   
  digitalWrite(Relay2_Valve2,LOW);   
  digitalWrite(Relay3_Valve3,LOW);   
  digitalWrite(Relay4_Valve4,LOW);   

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

  if (!wifiManager.autoConnect("ESP328CHRelay","password")) {
    Serial.println("failed to connect and hit timeout");//แสดงข้อความใน Serial Monitor
    delay(100);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();//แก้ เดิม ESP.reset(); ใน Esp8266
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
  
  // Blynk.config(blynk_token);////เริ่มการเชื่อมต่อ Blynk Server แบบปกติ
  //Blynk.config(blynk_token, server, port);
  Blynk.config(blynk_token);
  timer.setInterval(1000L,  Modbus_LTH01);
  timer.setInterval(1000L,  Modbus_LTH02);
  timer.setInterval(1000L, checkphysic_btn_state);
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
    Serial.println(blynkIsDownCount);
    Blynk.connect(5000);
    Blynk.syncAll();
    digitalWrite(ledbb, HIGH); //ledpin for check blynk connected 
  }
  if (blynkIsDownCount >= 10){
    ESP.restart();
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

//////////////Update switchStatus5 on Temperature////////////////
BLYNK_WRITE(V59)
{   
  switchStatus5 = param.asInt(); // Get value as integer
}
//////////////Update switchStatus5 on Temperature////////////////

/////////////////////Update Temperature setting////////////////////////
BLYNK_WRITE(V60)
{   
  tempsensorLimit5 = param.asInt(); // Get value as integer
}
/////////////////////Update Temperature setting////////////////////////

/////////////////////Update Humi setting////////////////////////
BLYNK_WRITE(V63)
{   
  humisensorLimit5 = param.asInt(); // Get value as integer
}
/////////////////////Update Humi setting////////////////////////

//////////////////////Update manualSwitch5///////////////////////
BLYNK_WRITE(V12)
{
  manualSwitch5 = param.asInt();
}
//////////////////////Update manualSwitch5///////////////////////

//Modbus Humi & Temperatue sensor
void Modbus_LTH01()
{
  uint8_t result1;
  float temp1, hum1, lux;

  // Data Frame --> 01 04 00 01 00 02 20 0B
  //result1 = node1.readInputRegisters(0x0001, 2);
    result1 = node1.readHoldingRegisters(0x0000, 3);
  if (result1 == node1.ku8MBSuccess)
  {
    hum1 = node1.getResponseBuffer(0) / 10.0f;
    temp1 = node1.getResponseBuffer(1) / 10.0f;
        
    Serial.print("Temp1: "); Serial.print(temp1); Serial.print("\t");
    Serial.print("Hum1: "); Serial.print(hum1); Serial.print("\t");
    Serial.println();
    
    node1.clearResponseBuffer();
    node1.clearTransmitBuffer();
    delay(3000);

    Blynk.virtualWrite(V30,hum1);
    Blynk.virtualWrite(V31,temp1);

    if(switchStatus5)
  {
    // auto
    if(temp1 > tempsensorLimit5 && hum1 < humisensorLimit5)
    {
        digitalWrite(ledfan1, HIGH);  
        Blynk.virtualWrite(V12, 1);                
        Blynk.setProperty(Widget_LED_Fan1, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Fan1, "label", "เปิดพัดลมระบายอากาศ");
        LedBlynkFan1.on(); 
    }  
    else
    {
        digitalWrite(ledfan1, LOW);
        Blynk.virtualWrite(V12, 0);
        Blynk.virtualWrite(Widget_LED_Fan1, 0);
        Blynk.setProperty(Widget_LED_Fan1, "label", "ปิดพัดลมระบายอากาศ");                       
        LedBlynkFan1.off();  
    }
  }
  else
  {
    if(manualSwitch5)
    {
        digitalWrite(ledfan1, HIGH);        
        Blynk.setProperty(Widget_LED_Fan1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Fan1, "label", "เปิดพัดลมระบายอากาศ");
        LedBlynkFan1.on(); 
    }
    else
    {
        digitalWrite(ledfan1, LOW);
        Blynk.setProperty(Widget_LED_Fan1, "label", "ปิดพัดลมระบายอากาศ");                       
        LedBlynkFan1.off(); 
    }
    // manaul
  }
 }
}
//End Modbus Humi & Temperatue sensor

//////////////Update switchStatus6 on Lux sensor////////////////
BLYNK_WRITE(V61)
{   
  switchStatus6 = param.asInt(); // Get value as integer
}
//////////////Update switchStatus6 on Lux sensor////////////////

/////////////////////Update Lux sensor setting////////////////////////
BLYNK_WRITE(V62)
{   
  luxsensorLimit6 = param.asInt(); // Get value as integer
}
/////////////////////Update Lux sensor setting////////////////////////

//////////////////////Update manualSwitch6///////////////////////
BLYNK_WRITE(V14)
{
  manualSwitch6 = param.asInt();
}
//////////////////////Update manualSwitch6///////////////////////

//Modbus Lux sensor
void Modbus_LTH02()
{
  uint8_t result1;
  float lux;
  // Data Frame --> 01 04 00 01 00 02 20 0B
  //result1 = node1.readInputRegisters(0x0001, 2);
    result1 = node1.readHoldingRegisters(0x0000, 3);
  if (result1 == node1.ku8MBSuccess)
  {
    lux = node1.getResponseBuffer(2);   
    lux = map(lux, 0, 65535, 0, 100);
  
    //lux_percentage = ( 100 - ((lux = node1.getResponseBuffer(2))/65535) * 100 );
     //moisture_percentage4 = ( 100 - ( (sensor_analog4/4095.00) * 100 ) );
    Serial.print("Lux: "); Serial.print(lux);
    Serial.println();
    
    node1.clearResponseBuffer();
    node1.clearTransmitBuffer();
    delay(3000);
    Blynk.virtualWrite(V32,lux);

    if(switchStatus6)
  {
    // auto
    if(lux > luxsensorLimit6)
    {
        digitalWrite(luxsensor, HIGH);  
        Blynk.virtualWrite(V14, 1);                
        Blynk.setProperty(Widget_LED_Lux, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Lux, "label", "เปิดระบบรับแสงแดด");
        LedBlynkLux.on(); 
    }  
    else
    {
        digitalWrite(luxsensor, LOW);
        Blynk.virtualWrite(V14, 0);
        Blynk.virtualWrite(Widget_LED_Lux, 0);
        Blynk.setProperty(Widget_LED_Lux, "label", "ปิดระบบรับแสงแดด");                       
        LedBlynkLux.off();  
    }
  }
  else
  {
    if(manualSwitch6)
    {
        digitalWrite(luxsensor, HIGH);        
        Blynk.setProperty(Widget_LED_Lux, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Lux, "label", "เปิดระบบรับแสงแดด");
        LedBlynkLux.on(); 
    }
    else
    {
        digitalWrite(luxsensor, LOW);
        Blynk.setProperty(Widget_LED_Lux, "label", "ปิดระบบรับแสงแดด");                       
        LedBlynkLux.off(); 
    }
    // manaul
  }
 }
}
//End Modbus Lux sensor

//****BUTTON ON/OFF Pump****//
BLYNK_WRITE(Widget_Btn_Pump){
int valuePump = param.asInt();
  if(valuePump == 1){
    digitalWrite(pump, HIGH);
    Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
    LedBlynkPump.on();
    }
    else{              
    digitalWrite(pump, LOW);
    Blynk.virtualWrite(Widget_Btn_Pump, 0);
    Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำ");
    LedBlynkPump.off();                       
    }
}

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


//**Check Status LED Widget**//
void checkphysic_btn_state()
{
  stateled1=digitalRead(Relay1_Valve1);   //Check ON/OFF สวิตส์1
  if (stateled1 != prevStateled1)
  {
      if (stateled1==1) Blynk.virtualWrite(V2,1);
      if (stateled1==0) Blynk.virtualWrite(V2,0);
  }
  prevStateled1=stateled1;
  
  stateled2=digitalRead(Relay2_Valve2);   //Check ON/OFF สวิตส์2
  if (stateled2 != prevStateled2)
  {
      if (stateled2==1) Blynk.virtualWrite(V4,1); 
      if (stateled2==0) Blynk.virtualWrite(V4,0); 
  }  
  prevStateled2=stateled2;

  stateled3=digitalRead(Relay3_Valve3);   //Check ON/OFF สวิตส์3
  if (stateled3 != prevStateled3)
  {
      if (stateled3==1) Blynk.virtualWrite(V6,1); 
      if (stateled3==0) Blynk.virtualWrite(V6,0); 
  }  
  prevStateled3=stateled3;

stateled4=digitalRead(Relay4_Valve4);     //Check ON/OFF สวิตส์4
  if (stateled4 != prevStateled4)
  {
      if (stateled4==1) Blynk.virtualWrite(V8,1); 
      if (stateled4==0) Blynk.virtualWrite(V8,0); 
  }  
  prevStateled4=stateled4;

  stateled5=digitalRead(pump);     //Check ON/OFF Pump
  if (stateled5 != prevStateled5)
  {
      if (stateled5==1) Blynk.virtualWrite(V10,1); 
      if (stateled5==0) Blynk.virtualWrite(V10,0); 
  }  
  prevStateled5=stateled5;
}

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
  
  if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();//ให้เวลาของ Blynk ทำงาน
}
//**********Loop Function**********// 
