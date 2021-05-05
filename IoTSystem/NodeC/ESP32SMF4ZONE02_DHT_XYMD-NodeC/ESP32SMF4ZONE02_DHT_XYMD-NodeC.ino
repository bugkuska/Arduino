//อ้างจาก https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/
//====Define Virtual Pin====//
//==========SW1==========//
//V1 ไฟสถานะปุ่ม SW1
//V2 ปุ่ม เปิด-ปิด SW1
//==========SW1==========//

//==========SW2==========//
//V3 ไฟสถานะปุ่ม SW2
//V4 ปุ่ม เปิด-ปิด SW2
//==========SW2==========//

//==========SW3==========//
//V5 ไฟสถานะปุ่ม SW3
//V6 ปุ่ม เปิด-ปิด SW3
//==========SW3==========//

//==========SW4==========//
//V7 ไฟสถานะปุ่ม SW4
//V8 ปุ่ม เปิด-ปิด SW4
//==========SW4==========//

//==========SWac1============//
//V9 ไฟสถานะปุ่ม SWac1
//V10 ปุ่ม เปิด-ปิด SWac1
//==========SWac1============//

//==========Cooling Fan1====//
//V11 ไฟสถานะปุ่ม พัดลมระบายอากาศ
//V12 ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
//V30 Temerature1
//V31 Humidity1
//V59 Auto&Manual Cooling Fan1
//V60 Slider Cooling Fan1
//==========Cooling Fan1====//

//==========Modbus Temp&Humi====//
//V32 Temperature1
//V33 Humidity1
//==========Modbus Temp&Humi====//

//==========Date&Time=======//
//V70 Current Time
//V71 Current Date
//==========Date&Time=======//

//==========Ultrasonic===//
//V98 สถานะ
//V99 ปุ่ม เปิด-ปิด 
//V100 Ultrasonic
//V101 Auto&Manual Ultrasonic
//V102 Slider Ultrasonic
//==========Ultrasonic====//

//====Define Virtual Pin====//

//======Define MCU Pin======//
//D23   relay connect to SWac1
//D15   relay connect to ledbb
//D13   relay connect to ledfan1
//D12   relay ยังไม่ได้ต่อไปใช้งาน
//D14   relay connect to SW1
//D27   relay connect to SW2
//D26   relay connect to SW3
//D25   relay connect to SW4
//D36   ว่าง  
//D39   ว่าง
//D34   ว่าง
//D35   ว่าง
//D32   Ultrasonic trigPin
//D33   Ultrasonic echoPin
//D16   RX2,RO
//D17   TX2,DI
//D18   DE
//D19   RE
//D21   SDA
//D22   SCL
//D5    DHT11
//======Define MCU Pin======//

//Need Libraries 
#include <FS.h>                         //this needs to be first, or it all crashes and burns...
#include <SPIFFS.h>
#include <WiFi.h>                       //https://github.com/esp8266/Arduino
#include <WiFiClient.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>                //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>                //Ver 5.13.4 https://github.com/bblanchon/ArduinoJson
#include <BlynkSimpleEsp32.h>           //Blynk_Release_v0.6.1 
//Blynk credentials
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
int blynkIsDownCount = 0;
BlynkTimer timer;
char server[] = "blynk-cloud.com";      //ถ้า customize server ให้แก้ชื่อ Server ให้ตรง
int port = 8442;
char blynk_token[34] = "";              //ใส่ Blynk_token ของเราที่ Blynk ส่งมาทาง Email ตอน Create Project ใหม่

//Define pin for Digital Output  
#define SWac1            23              //Relay connect to SWac1 
#define ledbb           15              //Check blynk connected 
#define ledfan1         13              //Cooling Fan1
#define Relay1_SW1      14              //SW1
#define Relay2_SW2      27              //SW2
#define Relay3_SW3      26              //Sw3
#define Relay4_SW4      25              //SW4

#define trigPin1  32                    //Ultrasonic trigPin
#define echoPin1  33                    //Ultrasonic echoPin
long duration1, distance1;

//Bridge widget 
WidgetBridge bridge1(V81);

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
#define DHTPin1 5
DHT dht(DHTPin1, DHTTYPE);

//XY-MD2
uint8_t result;
float temp, hum;

//Modbus Master
#include <ModbusMaster.h>
///////// PIN /////////
#define MAX485_DE      18             //Max485 DE
#define MAX485_RE_NEG  19             //Max485 RE
#define RX2 16                        //Max485 RO
#define TX2 17                        //Max485 DI
ModbusMaster node;
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
//End Modbus-Master

//==========SW1==========//
#define Widget_LED_SW1 V1                //ไฟสถานะปุ่ม SW1
#define Widget_Btn_SW1 V2                //ปุ่ม เปิด-ปิด SW1
WidgetLED LedBlynkSW1(Widget_LED_SW1);
//==========SW1==========//

//==========SW2==========//
#define Widget_LED_SW2 V3                //ไฟสถานะปุ่ม SW2
#define Widget_Btn_SW2 V4                //ปุ่ม เปิด-ปิด SW2
WidgetLED LedBlynkSW2(Widget_LED_SW2);
//==========SW2==========//

//==========SW3==========//
#define Widget_LED_SW3 V5               //ไฟสถานะปุ่ม SW3
#define Widget_Btn_SW3 V6               //ปุ่ม เปิด-ปิด SW3
WidgetLED LedBlynkSW3(Widget_LED_SW3);
//==========SW3==========//

//==========SW4==========//
#define Widget_LED_SW4 V7               //ไฟสถานะปุ่ม SW4
#define Widget_Btn_SW4 V8               //ปุ่ม เปิด-ปิด SW4
WidgetLED LedBlynkSW4(Widget_LED_SW4);
//==========SW4==========//

//==========สวิตซ์ 1 คุม load 220VAC===========//
#define Widget_LED_SWac1 V9              //ไฟสถานะปุ่ม สวิตซ์ 1 คุม load 220VAC
#define Widget_Btn_SWac1 V10             //ปุ่ม เปิด-ปิด สวิตซ์ 1 คุม load 220VAC
WidgetLED LedBlynkSWac1(Widget_LED_SWac1);
//==========สวิตซ์ 1 คุม load 220VAC===========//

//==========Cooling Fan1====//
//V31 Humidity1
//V30 Temperature1
//V59 Auto&Manual Cooling Fan1
//V60 Slider Cooling Fan1
#define Widget_LED_Fan1 V11               //ไฟสถานะปุ่ม พัดลมระบายอากาศ
#define Widget_Btn_Fan1 V12               //ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
WidgetLED LedBlynkFan1(Widget_LED_Fan1);
bool switchStatus1 = 0;                   // 0 = manual,1=auto
int tempsensorLimit1 = 0;
bool manualSwitch1 = 0;
//==========Cooling Fan1====//

//==========BridgeSW1====//
//V100 Ultrasonic
//V101 Auto&Manual Ultrasonic1
//V102 Slider Ultrasonic1
#define Widget_LED_Ultra1 V98               //ไฟสถานะปุ่ม 
#define Widget_Btn_Ultra1 V99               //ปุ่ม เปิด-ปิด
WidgetLED LedBlynkUltra1(Widget_LED_Ultra1);
bool switchStatus2 = 0;                   // 0 = manual,1=auto
int UltrasonicLimit2 = 0;
bool manualSwitch2 = 0;
//==========BridgeSW1====//

//===callback notifying us of the need to save config==//
bool shouldSaveConfig = false;
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
//===callback notifying us of the need to save config==//

//==========Setup Function==========//
void setup() {
  // put your setup code here, to run once:
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);
  node.begin(1, Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
    
  // Setup Pin Mode
  pinMode(SWac1, OUTPUT);              
  pinMode(ledbb,OUTPUT);              
  pinMode(ledfan1,OUTPUT);    
  pinMode(Relay1_SW1,OUTPUT);                       
  pinMode(Relay2_SW2,OUTPUT);      
  pinMode(Relay3_SW3,OUTPUT);     
  pinMode(Relay4_SW4,OUTPUT);            

  //UltraSonic
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  
  // Set Defult Relay Status Relay ที่ใช้เป็น Acitve HIGH ตอนเริ่มต้นต้อง Set ค่าเป็น 0 หรือ LOW
  digitalWrite(SWac1, LOW);            
  digitalWrite(ledbb,LOW);           
  digitalWrite(ledfan1,LOW);            
  digitalWrite(Relay1_SW1,LOW);                       
  digitalWrite(Relay2_SW2,LOW);      
  digitalWrite(Relay3_SW3,LOW);     
  digitalWrite(Relay4_SW4,LOW);   

  //Begin read Humidity and Temperature 
  dht.begin();
  
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
  

  //**************************    AP&Blynk CONNECT   ********************************************//
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_blynk_token);

  wifiManager.setTimeout(120);
  //ใช้ได้ 2 กรณี
  //1. เมื่อ MCU เริ่ม boot จะขึ้นชื่อ AP ที่เราตั้งขึ้น ในโค๊ดบรรทัดที่ 316 ให้เราใช้โทรศัพท์มือถือ หรือ Notebook เกาะ Wi-Fi ชื่อ AP ที่เราตั้ง
  //   ถ้าถาม password ให้ใส่คำว่า "password" ช่วงนี้ให้เราทำการตั้งค่า SSID+Password และใส่ blynk auth token ให้แล้วเสร็จภายใน 120 วินาที ก่อน AP จะหมดเวลา
  //   ไม่เช่นนั้น เมื่อครบเวลา 120 วินาที MCU จะ Reset เริ่มต้นใหม่ ให้เราตั้งค่าอีกครั้งภายใน 120 วินาที
  //2. ช่วงไฟดับ Modem router + MCU จะดับทั้งคู่ และเมื่อมีไฟมา ทั้งคู่ก็เริ่มทำงานเช่นกัน
  //   โดยปกติ Modem router จะ Boot ช้ากว่า  MCU ทำให้ MCU กลับไปเป็น AP รอให้เราตั้งค่าใหม่
  //   ดังนั้น AP จะรอเวลาให้เราตั้งค่า 120 วินาที ถ้าไม่มีการตั้งค่าใดๆ เมื่อครบ 120 วินาที MCU จะ Reset อีกครั้ง
  //   ถ้า Modem router  Boot และใช้งานได้ภายใน 120 วินาที และหลังจากที่ MCU Resset และเริ่มทำงานใหม่
  //   ก็จะสามารถเชื่อมต่อกับ  Modem router ที่ Boot และใช้งานได้แล้ว  ได้  ระบบจะทำงานปกติ

  if (!wifiManager.autoConnect("ESP32_NodeC","password")) {         //ชื่อ AP กับ password สามารถเปลี่ยนได้ตามต้องการ
    Serial.println("failed to connect and hit timeout");            //แสดงข้อความใน Serial Monitor
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();                                                  //แก้ เดิม ESP.reset(); ใน Esp8266
    delay(5000);
  }
  
  Serial.println("Connected.......OK!)");                           //แสดงข้อความใน Serial Monitor
  strcpy(blynk_token, custom_blynk_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["blynk_token"] = blynk_token;
    File configFile = SPIFFS.open("/config.json", "w");

      if (!configFile) {
        Serial.println("failed to open config file for writing"); //แสดงข้อความใน Serial Monitor
      }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  //**************************   จบ AP&Blynk CONNECT  *****************************************//
  
  Serial.println("local ip");                         //แสดงข้อความใน Serial Monitor
  delay(100);
  Serial.println(WiFi.localIP());                     //แสดงข้อความใน Serial Monitor
  
   
  //Blynk.config(blynk_token, server, port);          //ถ้าจะ Customize Server ให้ระบุ Server และ Port ปลายทางให้ถูกต้อง
  Blynk.config(blynk_token);                          //เชื่อมต่อโดยใช้ auth token จาก blynk-cloud.com
  timer.setInterval(5000L,dhtSensorData1);            //อ่านค่าอุณหภูมิภายในระบบควบคุม
  timer.setInterval(5000L,xymd02);                    //อ่านค่าอุณหภูมิสภาพแวดล้อมโดยรอบ
  timer.setInterval(5000L,ultrasonicData1);           //อ่านค่าระยะของระดับน้ำจนถึงตัววัด ultrasonic
  timer.setInterval(50000L, reconnectblynk);          //Reconnect ot blynk
}
//===================End Setup==================//

//=====BUTTON ON/OFF SW1========//
BLYNK_WRITE(Widget_Btn_SW1){
int valueSW1 = param.asInt();
  if(valueSW1 == 1){
    digitalWrite(Relay1_SW1, HIGH);
    Blynk.setProperty(Widget_LED_SW1, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_SW1, "label", "เปิดสวิตซ์1");
    LedBlynkSW1.on();
    }
    else{              
    digitalWrite(Relay1_SW1, LOW);
    Blynk.virtualWrite(Widget_Btn_SW1, 0);
    Blynk.setProperty(Widget_LED_SW1, "label", "ปิดสวิตซ์1");
    LedBlynkSW1.off();                       
    }
}
//=====BUTTON ON/OFF SW1========//

//=====BUTTON ON/OFF SW2========//
BLYNK_WRITE(Widget_Btn_SW2){
int valueSW2 = param.asInt();
  if(valueSW2 == 1){
    digitalWrite(Relay2_SW2, HIGH);
    Blynk.setProperty(Widget_LED_SW2, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_SW2, "label", "เปิดสวิตซ์2");
    LedBlynkSW2.on();
    }
    else{              
    digitalWrite(Relay2_SW2, LOW);
    Blynk.virtualWrite(Widget_Btn_SW2, 0);
    Blynk.setProperty(Widget_LED_SW2, "label", "ปิดสวิตซ์2");
    LedBlynkSW2.off();                       
    }
}
//=====BUTTON ON/OFF SW2========//

//=====BUTTON ON/OFF SW3========//
BLYNK_WRITE(Widget_Btn_SW3){
int valueSW3 = param.asInt();
  if(valueSW3 == 1){
    digitalWrite(Relay3_SW3, HIGH);
    Blynk.setProperty(Widget_LED_SW3, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_SW3, "label", "เปิดสวิตซ์3");
    LedBlynkSW3.on();
    }
    else{              
    digitalWrite(Relay3_SW3, LOW);
    Blynk.virtualWrite(Widget_Btn_SW3, 0);
    Blynk.setProperty(Widget_LED_SW3, "label", "ปิดสวิตซ์3");
    LedBlynkSW3.off();                       
    }
}
//=====BUTTON ON/OFF SW3========//

//=====BUTTON ON/OFF SW4========//
BLYNK_WRITE(Widget_Btn_SW4){
int valueSW4 = param.asInt();
  if(valueSW4 == 1){
    digitalWrite(Relay4_SW4, HIGH);
    Blynk.setProperty(Widget_LED_SW4, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_SW4, "label", "เปิดสวิตซ์4");
    LedBlynkSW4.on();
    }
    else{              
    digitalWrite(Relay4_SW4, LOW);
    Blynk.virtualWrite(Widget_Btn_SW4, 0);
    Blynk.setProperty(Widget_LED_SW4, "label", "ปิดสวิตซ์4");
    LedBlynkSW4.off();                       
    }
}
//=====BUTTON ON/OFF SW4========//

//=====BUTTON ON/OFF SWac1========//
BLYNK_WRITE(Widget_Btn_SWac1){
int valueSWac1 = param.asInt();
  if(valueSWac1 == 1){
    digitalWrite(SWac1, HIGH);
    Blynk.setProperty(Widget_LED_SWac1, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_SWac1, "label", "เปิดสวิตซ์1");
    LedBlynkSWac1.on();
    }
    else{              
    digitalWrite(SWac1, LOW);
    Blynk.virtualWrite(Widget_Btn_SWac1, 0);
    Blynk.setProperty(Widget_LED_SWac1, "label", "ปิดสวิตซ์1");
    LedBlynkSWac1.off();                       
    }
}
//=====BUTTON ON/OFF SWac1========//

//=====Update switchStatus1 on DHT11=====//
BLYNK_WRITE(V59)
{   
  switchStatus1 = param.asInt();            // Get value as integer
}
//=====Update switchStatus1 on DHT11=====

//=====Update DHT11 setting=====//
BLYNK_WRITE(V60)
{   
  tempsensorLimit1 = param.asInt();         // Get value as integer
}
//=====Update DHT11 setting=====//

//=====Update manualSwitch======//
BLYNK_WRITE(V12)
{
  manualSwitch1 = param.asInt();
}
//=====Update manualSwitch======//

//=========DHT11-01=============//
void dhtSensorData1(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();        // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)){ 
 Serial.println("Read from DHT Sensor");
  return;
  }
  Blynk.virtualWrite(V30, t);             //เขียนค่าอุณหภูมิไปที่ blynk ผ่าน Virtual pin V30
  Blynk.virtualWrite(V31, h);             //เขียนค่าความชื้นไปที่ blynk ผ่าน Virtual pin V31

 if(switchStatus1)
  {
    // auto
    if(t > tempsensorLimit1)              //ถ้าอุณหภูมิมากกว่าค่าที่เรากำหนดไว้บนสไลด์เดอร์ เงื่อนไขเป็นจริง
    {
        digitalWrite(ledfan1, HIGH);  
        Blynk.virtualWrite(V12, 1);                
        Blynk.setProperty(Widget_LED_Fan1, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Fan1, "label", "เปิดพัดลม");
        LedBlynkFan1.on(); 
    }  
    else
    {
        digitalWrite(ledfan1, LOW);
        Blynk.virtualWrite(V12, 0);
        Blynk.virtualWrite(Widget_LED_Fan1, 0);
        Blynk.setProperty(Widget_LED_Fan1, "label", "ปิดพัดลม");                       
        LedBlynkFan1.off();  
    }
  }
  else
  {
    if(manualSwitch1)
    {
        digitalWrite(ledfan1, HIGH);        
        Blynk.setProperty(Widget_LED_Fan1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Fan1, "label", "เปิดพัดลม");
        LedBlynkFan1.on(); 
    }
    else
    {
        digitalWrite(ledfan1, LOW);
        Blynk.setProperty(Widget_LED_Fan1, "label", "ปิดพัดลม");                       
        LedBlynkFan1.off(); 
    }
    // manaul
  }
}  
//=========DHT11-01=============//

//=====Update switchStatus2 on Ultrasonic1=====//
BLYNK_WRITE(V101)
{   
  switchStatus2 = param.asInt();                  // Get value as integer
}
//=====Update switchStatus2 on Ultrasonic1=====

//=====Update Ultrasonic1 setting=====//
BLYNK_WRITE(V102)
{   
  UltrasonicLimit2 = param.asInt();               // Get value as integer
}
//=====Update Ultrasonic1 setting=====//

//=====Update manualSwitch======//
BLYNK_WRITE(V99)
{
  manualSwitch2 = param.asInt();
}
//=====Update manualSwitch2======//

//=====Ultrasonic=====//
void ultrasonicData1(){
// defines variables
//long duration1, distance1;
  digitalWrite(trigPin1, LOW);  
  delayMicroseconds(2); 
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trigPin1, LOW);
  duration1 = pulseIn(echoPin1, HIGH);
  distance1 = (duration1/2) / 29.1;

   if (distance1 >= 500 || distance1 <= 0){
    Serial.println("Out of range");
  }
  else {
    Serial.print ( "Sensor1  ");
    Serial.print ( distance1);
    Serial.println("cm");
  }
  delay(2000);
  Blynk.virtualWrite(V100, distance1);

if(switchStatus2)
  {
    // auto
    if(distance1 > UltrasonicLimit2)            //ถ้าระยะที่ ultrasonic วัดค่าได้มีค่ามากกว่าค่าที่เรากำหนดไว้บนสไลด์เดอร์ เงื่อนไขเป็นจริง
    {
        //digitalWrite(ledfan1, HIGH);  
        bridge1.digitalWrite(23,HIGH);          //ส่งคำสั่งไปควบคุม NodeB ขา GPIO 23 ที่ต่อเข้ากับแมคเนติคและปั้มน้ำ
        Blynk.virtualWrite(V99, 1);                
        Blynk.setProperty(Widget_LED_Ultra1, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Ultra1, "label", "เปิดปั้มเติมน้ำ");
        LedBlynkUltra1.on(); 
    }  
    else
    {
       // digitalWrite(ledfan1, LOW);
       bridge1.digitalWrite(23,LOW);            //ส่งคำสั่งไปควบคุม NodeB ขา GPIO 23 ที่ต่อเข้ากับแมคเนติคและปั้มน้ำ
        Blynk.virtualWrite(V99, 0);
        Blynk.virtualWrite(Widget_LED_Ultra1, 0);
        Blynk.setProperty(Widget_LED_Ultra1, "label", "ปิดปั้มน้ำ");                       
        LedBlynkUltra1.off();  
    }
  }
  else
  {
    if(manualSwitch2)
    {
        //digitalWrite(ledfan1, HIGH); 
        bridge1.digitalWrite(23,HIGH);          //ส่งคำสั่งไปควบคุม NodeB ขา GPIO 23 ที่ต่อเข้ากับแมคเนติคและปั้มน้ำ     
        Blynk.setProperty(Widget_LED_Ultra1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Ultra1, "label", "เปิดปั้มเติมน้ำ");
        LedBlynkUltra1.on(); 
    }
    else
    {
        //digitalWrite(ledfan1, LOW);
        bridge1.digitalWrite(23,LOW);           //ส่งคำสั่งไปควบคุม NodeB ขา GPIO 23 ที่ต่อเข้ากับแมคเนติคและปั้มน้ำ
        Blynk.setProperty(Widget_LED_Ultra1, "label", "ปิดปั้มน้ำ");                       
        LedBlynkUltra1.off(); 
    }
    // manaul
  } 
}

//==Modbus Temperature & Humidity==//
void xymd02() {
//uint8_t result;
//float temp, hum;
  // Data Frame --> 01 04 00 01 00 02 20 0B
  result = node.readInputRegisters(0x0001, 2);
  if (result == node.ku8MBSuccess)
  {
    temp = node.getResponseBuffer(0) / 10.0f;
    hum = node.getResponseBuffer(1) / 10.0f;
    Serial.print("Temp: "); Serial.print(temp); Serial.print("\t");
    Serial.print("Hum: "); Serial.print(hum);
    Serial.println();
    node.clearResponseBuffer();
    node.clearTransmitBuffer();
    Serial.printf("Info: sht20[0x01] temperature1 = %.1f\r\n",temp);
    Serial.printf("Info: sht20[0x01] humidity1 = %.1f\r\n",hum);
    delay(1000);
    Blynk.virtualWrite(V32,temp);                    //เขียนค่าอุณหภูมิไปที่ blynk ผ่าน Virtual pin V32                        
    Blynk.virtualWrite(V33,hum);                   //เขียนค่าความชื้นไปที่ blynk ผ่าน Virtual pin V33
  }
}
//==Modbus Temperature & Humidity==//

//==Display Current Date/Time==//
void clockDisplay()
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  // Send time to the App
  Blynk.virtualWrite(V70, currentTime);
  // Send date to the App
  Blynk.virtualWrite(V71, currentDate);      
}
//==Display Current Date/Time==//

//==========Blynk conneted==========//
//blynk conneted
BLYNK_CONNECTED()
{
 Blynk.syncAll();
 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(ledbb,HIGH);
    Serial.println("ledbb on");
    bridge1.setAuthToken("ArN8mv_E8raQFasthcBHgUgGyLt_V0sN");  //เอา auth token ของ ESP32 NodeB มาใส่ เพื่อส่งคำสั่งควบคุมผ่าน bridge widget
 }
}
//==========Blynk conneted==========//

//==========Reconnect to blynk======//
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
  if(blynkIsDownCount > 10){
    ESP.restart();
  }
}

//==========Reconnect to blynk======//

//=======Loop Function=======// 
void loop() {    
//LCD Display Data
  lcd.begin();
  lcd.backlight();    
  lcd.setCursor(0, 0);
  lcd.print("==SmartFarming NodeC==");

//Display Current Date&Time
  String currentTime = String(hour()) + ":" + minute();
  String currentDate = String(day()) + "/" + month() + "/" + year();
  lcd.setCursor(0, 1); 
  lcd.print("DT="); 
  lcd.print(currentTime);
      
  lcd.setCursor(9, 1);
  lcd.print("|"); 
  lcd.print(currentDate);

  //Display ultrasonic 
  lcd.setCursor(0, 2);
  lcd.print("Water Level : "); 
  lcd.print(distance1);
  lcd.print("cm");

  //Display Temp & Humi
  lcd.setCursor(0, 3);
  lcd.print("Temp:"); 
  lcd.print(temp);
  lcd.print("C");

  lcd.setCursor(9, 3);
  lcd.print("Humi:"); 
  lcd.print(hum);
  lcd.print("%");
  
  
  if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();
}
//=======Loop Function=======// 
