/////////////////////////////////////////Define Virtual Pin////////////////////////////////
//V1 ปุ่ม เปิด-ปิด Valve1
//V2 ปุ่ม เปิด-ปิด Valve1
//V3 ปุ่ม เปิด-ปิด Valve1
//V4 ปุ่ม เปิด-ปิด Valve1
//V5 ปุ่ม เปิด-ปิด Valve1
//V6 ปุ่ม เปิด-ปิด Valve1
//V7 ปุ่ม เปิด-ปิด Valve1
//V8 ปุ่ม เปิด-ปิด Valve1
//V9 ปุ่ม เปิด-ปิด Valve1
//V10 ปุ่ม เปิด-ปิด Valve1
//V11 ปุ่ม เปิด-ปิด Valve1
//V12 ปุ่ม เปิด-ปิด Valve1
//V13 ปุ่ม เปิด-ปิด Valve1
//V14 ปุ่ม เปิด-ปิด Valve1
//V15 ปุ่ม เปิด-ปิด Valve1
//V16 ปุ่ม เปิด-ปิด Valve1
//V21 Modbus Humidity01
//V22 Modbus Temperature01
//V23 Modbus Light 
//V24 Modbus Soil Moisture Sensor01
//------------------------------------------------------------------------------------------------------------------------//
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <SPIFFS.h>
#include <WiFi.h>                 //https://github.com/esp8266/Arduino
#include <WiFiClient.h>
//needed for library
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //Ver 5.13.4   //https://github.com/bblanchon/ArduinoJson
//------------------------------------------------------------------------------------------------------------------------//
//Blynk Libraries
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp32.h>     //  Blynk_Release_v0.6.1 
int blynkIsDownCount = 0;
BlynkTimer timer;
char server[] = "blynk-cloud.com";
int port = 8442;
char blynk_token[34] = "";//ใส่ Blynk_token ของเราที่ Blynk ส่งมาทาง Email ตอน Create Project ใหม่
//------------------------------------------------------------------------------------------------------------------------//
#include <Wire.h> 
#include "ETT_ModbusRTU.h"
#include <HardwareSerial.h>
#define SerialDebug  Serial                         // USB Serial(Serial0)

#define SerialRS485_RX_PIN    16                    // Serial1(IO17=TXD,IO16=RXD)
#define SerialRS485_TX_PIN    17
#define RS485_DIRECTION_PIN   5  
#define SerialRS485  Serial1                                                                      

//Data array for modbus network sharing
uint16_t au16dataRelay[16];      
uint8_t u8state2;                                   // machine state
uint8_t u8query2;                                   // pointer to message query
//unsigned long u32wait2;
unsigned long lastScanBusTime2 = 0;

modbus_t telegram_relay[16];                                                                           // 16-Modbus Commans
unsigned long u32wait2;                                                                           // Scan Rate Modbus RTU
int8_t pool_size;

//Valve1-Valve16
#define Widget_Btn_Valve1   V1              //ปุ่ม เปิด-ปิด Valve1
#define Widget_Btn_Valve2   V2              //ปุ่ม เปิด-ปิด Valve2
#define Widget_Btn_Valve3   V3              //ปุ่ม เปิด-ปิด Valve3
#define Widget_Btn_Valve4   V4              //ปุ่ม เปิด-ปิด Valve4
#define Widget_Btn_Valve5   V5              //ปุ่ม เปิด-ปิด Valve5
#define Widget_Btn_Valve6   V6              //ปุ่ม เปิด-ปิด Valve6
#define Widget_Btn_Valve7   V7              //ปุ่ม เปิด-ปิด Valve7
#define Widget_Btn_Valve8   V8              //ปุ่ม เปิด-ปิด Valve8
#define Widget_Btn_Valve9   V9              //ปุ่ม เปิด-ปิด Valve9
#define Widget_Btn_Valve10  V10             //ปุ่ม เปิด-ปิด Valve10
#define Widget_Btn_Valve11  V11             //ปุ่ม เปิด-ปิด Valve11
#define Widget_Btn_Valve12  V12             //ปุ่ม เปิด-ปิด Valve12
#define Widget_Btn_Valve13  V13             //ปุ่ม เปิด-ปิด Valve13
#define Widget_Btn_Valve14  V14             //ปุ่ม เปิด-ปิด Valve14
#define Widget_Btn_Valve15  V15             //ปุ่ม เปิด-ปิด Valve15
#define Widget_Btn_Valve16  V16             //ปุ่ม เปิด-ปิด Valve16
//

//GZWS-N01 3IN1 Light, Temperature & Humidity 
modbus_t telegram_gzws01[2];                // 2-Modbus Frame Service
uint16_t InputRegister_gzws01[8];   
uint16_t HoldingRegister_gzws01[8]; 
uint8_t u8state;                            // machine state
uint8_t u8query;                            // pointer to message query
unsigned long u32wait;
unsigned long lastScanBusTime = 0;

//Soil Moisture Sensor01
modbus_t telegram_soil01[2];                // 2-Modbus Frame Service
uint16_t InputRegister_soil01[8];   
uint16_t HoldingRegister_soil01[8]; 
uint8_t u8state1;                           // machine state
uint8_t u8query1;                           // pointer to message query
unsigned long u32wait1;
unsigned long lastScanBusTime1 = 0;

//Modbus Node ID
Modbus master(0,                            // node id = 0(master)
              SerialRS485,                  // Serial2
              RS485_DIRECTION_PIN);         // RS485 Modbus

//------------------------------------------------------------------------------------------------------------------------//
//Define pin for clear and config AP&Token
#define AP_Config       14
//------------------------------------------------------------------------------------------------------------------------//
//10A 4channel Relay extend 
#define pump            27                  // Relay connect to pump 
#define ledbb           26                  //Check blynk connected 
#define ledfan           2                  //Cooling Fan
//------------------------------------------------------------------------------------------------------------------------//
//10A Relay on board esp32all
#define Relay1_SW1  19                      //SW1
#define Relay2_SW2  18                      //SW2
#define Relay3_SW3  5                        //SW3
#define Relay4_SW4  25                       //SW4
//------------------------------------------------------------------------------------------------------------------------//
//Define pin for analog input
#define INPUT_1_A1 39                       //Analog input 1
#define INPUT_2_A2 34                       //Analog input 2
#define INPUT_3_A3 35                       //Analog input 3
#define INPUT_4_A4 32                       //Analog input 4
//------------------------------------------------------------------------------------------------------------------------//
 //RTC Widget
#include <TimeLib.h>
#include <WidgetRTC.h>
WidgetRTC rtc;
//------------------------------------------------------------------------------------------------------------------------//
//OLED Display
#include <Wire.h>                           // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"                    // legacy include: `#include "SSD1306.h"`
SSD1306Wire  display(0x3c, 21, 22);
//------------------------------------------------------------------------------------------------------------------------//

//------------------------------------------------------------------------------------------------------------------------//
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
//------------------------------------------------------------------------------------------------------------------------//

//Setup Function                  
void setup() 
{   
 //OLED
  display.init();
  display.display();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.display();
//------------------------------------------------------------------------------------------------------------------------//  
  // Setup Pin Mode
  pinMode(AP_Config, INPUT_PULLUP);//กำหนดโหมดใช้งานให้กับขา AP_Config เป็นขา กดปุ่ม ค้าง เพื่อตั้งค่า AP config 
  pinMode(pump, OUTPUT);              // Pin 23 relay 10A in1 connect to Manatic + overload
  pinMode(ledbb,OUTPUT);              // Pin 27 relay 10A in2 connect to Green PoltLamp check blynk connected
  pinMode(ledfan,OUTPUT);             // Pin 26 relay 10A in3 connect to Red PoltLamp start cooling fan 
  pinMode(Relay1_SW1,OUTPUT);         // Pin 19 relay 10A in1 valve1
  pinMode(Relay2_SW2,OUTPUT);         // Pin 18 relay 10A in2 valve2
  pinMode(Relay3_SW3,OUTPUT);         // Pin 5  relay 10A in3 valve3
  pinMode(Relay4_SW4,OUTPUT);         // Pin 25 relay 10A in4 valve4
//------------------------------------------------------------------------------------------------------------------------//  
  // Set Defult Relay Status
  digitalWrite(pump, HIGH);           // Pin 23 relay 10A in1 connect to Manatic + overload
  digitalWrite(ledbb,HIGH);           // Pin 27 relay 10A in2 connect to Green PoltLamp check blynk connected
  digitalWrite(ledfan,HIGH);          // Pin 26 relay 10A in3 connect to Red PoltLamp start cooling fan  
  digitalWrite(Relay1_SW1,HIGH);      // Pin 19 relay 10A in1 valve1
  digitalWrite(Relay2_SW2,HIGH);      // Pin 18 relay 10A in2 valve2
  digitalWrite(Relay3_SW3,HIGH);      // Pin 5  relay 10A in3 valve3
  digitalWrite(Relay4_SW4,HIGH);      // Pin 25 relay 10A in4 valve4
//------------------------------------------------------------------------------------------------------------------------//
//Begin Sync time
  rtc.begin();
//------------------------------------------------------------------------------------------------------------------------//  

  SerialDebug.begin(115200);
  while(!SerialDebug);

  SerialRS485.begin(9600, SERIAL_8N1, SerialRS485_RX_PIN, SerialRS485_TX_PIN);
  while(!SerialRS485);
    
  //===============================================================================================
  master.begin(SerialRS485);                                                                      // Mosbus Interface
  master.setTimeOut(2000);                                                                        // if there is no answer in 2000 ms, roll over
  u32wait = millis() + 2000;
  u8state = u8query = 0; 
  lastScanBusTime = millis();
  
  master.begin(SerialRS485);                                                                      // Mosbus Interface
  master.setTimeOut(2000);                                                                        // if there is no answer in 2000 ms, roll over
  u32wait1 = millis() + 2000;
  u8state1 = u8query1 = 0; 
  lastScanBusTime1 = millis();

  master.begin(SerialRS485);                                                                     // Mosbus Interface RS485
  master.setTimeOut(2500);                                                                       // if there is no answer in 3500 ms, roll over
  u32wait2 = millis() + 1000;                                                                     // Next Scan Rate = 1 Second
  u8state2 = u8query2 = 0;  
  lastScanBusTime2 = millis();
  //===============================================================================================
  // Mosbus RTU Command
  //=============================================================================================== 
  //===============================================================================================
  // F06 Write Single Register
  //===============================================================================================
  telegram_relay[0].u8id = 2;                     // slave address : 0x01
  telegram_relay[0].u8fct = 6;                    // Function 0x06(Write Single Register)
  telegram_relay[0].u16RegAdd = 1;                // Address = Relay(1)
  telegram_relay[0].au16reg = &au16dataRelay[0];  // pointer to a memory array in the Arduino
 // au16dataRelay[0] = 0x0200;                    // 0x0400 = Latch Relay
  //===============================================================================================

  //===============================================================================================
  // F06 Write Single Register
  //===============================================================================================
  telegram_relay[1].u8id = 2;                     // slave address : 0x01
  telegram_relay[1].u8fct = 6;                    // Function 0x06(Write Single Register)
  telegram_relay[1].u16RegAdd = 2;                // Address = Relay(2)
  telegram_relay[1].au16reg = &au16dataRelay[1];  // pointer to a memory array in the Arduino
 // au16dataRelay[1] = 0x0200;                    // 0x0400 = Latch Relay
  //===============================================================================================

  //===============================================================================================
  // F06 Write Single Register
  //===============================================================================================
  telegram_relay[2].u8id = 2;                     // slave address : 0x01
  telegram_relay[2].u8fct = 6;                    // Function 0x06(Write Single Register)
  telegram_relay[2].u16RegAdd = 3;                // Address = Relay(3)
  telegram_relay[2].au16reg = &au16dataRelay[2];  // pointer to a memory array in the Arduino
 // au16dataRelay[2] = 0x0200;                    // 0x0400 = Latch Relay
  //===============================================================================================

  //===============================================================================================
  // F06 Write Single Register
  //===============================================================================================
  telegram_relay[3].u8id = 2;                     // slave address : 0x01
  telegram_relay[3].u8fct = 6;                    // Function 0x06(Write Single Register)
  telegram_relay[3].u16RegAdd = 4;                // Address = Relay(4)
  telegram_relay[3].au16reg = &au16dataRelay[3];  // pointer to a memory array in the Arduino
 // au16dataRelay[3] = 0x0200;                    // 0x0400 = Latch Relay
  //===============================================================================================

  //===============================================================================================
  // F06 Write Single Register
  //===============================================================================================
  telegram_relay[4].u8id = 2;                     // slave address : 0x01
  telegram_relay[4].u8fct = 6;                    // Function 0x06(Write Single Register)
  telegram_relay[4].u16RegAdd = 5;                // Address = Relay(5)
  telegram_relay[4].au16reg = &au16dataRelay[4];  // pointer to a memory array in the Arduino
 // au16dataRelay[4] = 0x0200;                    // 0x0400 = Latch Relay
  //===============================================================================================

  //===============================================================================================
  // F06 Write Single Register
  //===============================================================================================
  telegram_relay[5].u8id = 2;                     // slave address : 0x01
  telegram_relay[5].u8fct = 6;                    // Function 0x06(Write Single Register)
  telegram_relay[5].u16RegAdd = 6;                // Address = Relay(6)
  telegram_relay[5].au16reg = &au16dataRelay[5];  // pointer to a memory array in the Arduino
 // au16dataRelay[5] = 0x0200;                    // 0x0400 = Latch Relay
  //===============================================================================================

  //===============================================================================================
  // F06 Write Single Register
  //===============================================================================================
  telegram_relay[6].u8id = 2;                     // slave address : 0x01
  telegram_relay[6].u8fct = 6;                    // Function 0x06(Write Single Register)
  telegram_relay[6].u16RegAdd = 7;                // Address = Relay(7)
  telegram_relay[6].au16reg = &au16dataRelay[6];  // pointer to a memory array in the Arduino
  //au16dataRelay[6] = 0x0200;                    // 0x0400 = Latch Relay
  //===============================================================================================

  //===============================================================================================
  // F06 Write Single Register
  //===============================================================================================
  telegram_relay[7].u8id = 2;                     // slave address : 0x01
  telegram_relay[7].u8fct = 6;                    // Function 0x06(Write Single Register)
  telegram_relay[7].u16RegAdd = 8;                // Address = Relay(8)
  telegram_relay[7].au16reg = &au16dataRelay[7];  // pointer to a memory array in the Arduino
 // au16dataRelay[7] = 0x0200;                    // 0x0400 = Latch Relay
  //===============================================================================================

  //===============================================================================================
  // F06 Write Single Register
  //===============================================================================================
  telegram_relay[8].u8id = 2;                   // slave address : 0x01
  telegram_relay[8].u8fct = 6;                  // Function 0x06(Write Single Register)
  telegram_relay[8].u16RegAdd = 9;              // Address = Relay(9)
  telegram_relay[8].au16reg = &au16dataRelay[8];  // pointer to a memory array in the Arduino
  //au16dataRelay[8] = 0x0200;                  // 0x0400 = Latch Relay
  //===============================================================================================

  //===============================================================================================
  // F06 Write Single Register
  //===============================================================================================
  telegram_relay[9].u8id = 2;                     // slave address : 0x01
  telegram_relay[9].u8fct = 6;                    // Function 0x06(Write Single Register)
  telegram_relay[9].u16RegAdd = 10;               // Address = Relay(10)
  telegram_relay[9].au16reg = &au16dataRelay[9];  // pointer to a memory array in the Arduino
  //au16dataRelay[9] = 0x0200;                    // 0x0400 = Latch Relay
  //===============================================================================================

  //===============================================================================================
  // F06 Write Single Register
  //===============================================================================================
  telegram_relay[10].u8id = 2;                        // slave address : 0x01
  telegram_relay[10].u8fct = 6;                       // Function 0x06(Write Single Register)
  telegram_relay[10].u16RegAdd = 11;                  // Address = Relay(11)
  telegram_relay[10].au16reg = &au16dataRelay[10];    // pointer to a memory array in the Arduino
  //au16dataRelay[10] = 0x0200;                       // 0x0400 = Latch Relay
  //===============================================================================================

  //===============================================================================================
  // F06 Write Single Register
  //===============================================================================================
  telegram_relay[11].u8id = 2;                        // slave address : 0x01
  telegram_relay[11].u8fct = 6;                       // Function 0x06(Write Single Register)
  telegram_relay[11].u16RegAdd = 12;                  // Address = Relay(12)
  telegram_relay[11].au16reg = &au16dataRelay[11];    // pointer to a memory array in the Arduino
  //au16dataRelay[11] = 0x0200;                       // 0x0400 = Latch Relay
  //===============================================================================================

  //===============================================================================================
  // F06 Write Single Register
  //===============================================================================================
  telegram_relay[12].u8id = 2;                        // slave address : 0x01
  telegram_relay[12].u8fct = 6;                       // Function 0x06(Write Single Register)
  telegram_relay[12].u16RegAdd = 13;                  // Address = Relay(13)
  telegram_relay[12].au16reg = &au16dataRelay[12];    // pointer to a memory array in the Arduino
  //au16dataRelay[12] = 0x0200;                       // 0x0400 = Latch Relay
  //===============================================================================================

  //===============================================================================================
  // F06 Write Single Register
  //===============================================================================================
  telegram_relay[13].u8id = 2;                        // slave address : 0x01
  telegram_relay[13].u8fct = 6;                       // Function 0x06(Write Single Register)
  telegram_relay[13].u16RegAdd = 14;                  // Address = Relay(14)
  telegram_relay[13].au16reg = &au16dataRelay[13];    // pointer to a memory array in the Arduino
  //au16dataRelay[13] = 0x0200;                       // 0x0400 = Latch Relay
  //===============================================================================================

  //===============================================================================================
  // F06 Write Single Register
  //===============================================================================================
  telegram_relay[14].u8id = 2;                        // slave address : 0x01
  telegram_relay[14].u8fct = 6;                       // Function 0x06(Write Single Register)
  telegram_relay[14].u16RegAdd = 15;                  // Address = Relay(15)
  telegram_relay[14].au16reg = &au16dataRelay[14];    // pointer to a memory array in the Arduino
  //au16dataRelay[14] = 0x0200;                       // 0x0400 = Latch Relay
  //===============================================================================================

  //===============================================================================================
  // F06 Write Single Register
  //===============================================================================================
  telegram_relay[15].u8id = 2;                        // slave address : 0x01
  telegram_relay[15].u8fct = 6;                       // Function 0x06(Write Single Register)
  telegram_relay[15].u16RegAdd = 16;                  // Address = Relay(16)
  telegram_relay[15].au16reg = &au16dataRelay[15];    // pointer to a memory array in the Arduino
  //au16dataRelay[15] = 0x0200;                       // 0x0400 = Latch Relay
//===============================================================================================
  //------------------------------------------------------------------------------------------------------------------------//  
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
//------------------------------------------------------------------------------------------------------------------------//
//AP AUTO CONNECT   
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
//------------------------------------------------------------------------------------------------------------------------//
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
//------------------------------------------------------------------------------------------------------------------------//
  wifiManager.addParameter(&custom_blynk_token);
//------------------------------------------------------------------------------------------------------------------------//
 for (int i = 5; i > -1; i--) {                           // นับเวลาถอยหลัง 5 วินาทีก่อนกดปุ่ม AP Config
    delay(500);
    Serial.print (String(i) + " ");//แสดงข้อความใน Serial Monitor  
  }
//------------------------------------------------------------------------------------------------------------------------//
  if (digitalRead(AP_Config) == LOW) {
    Serial.println("Button Pressed");                     //แสดงข้อความใน Serial Monitor
    // wifiManager.resetSettings();                       //ให้ล้างค่า SSID และ Password ที่เคยบันทึกไว้
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();  //load the flash-saved configs
    esp_wifi_init(&cfg);                                  //initiate and allocate wifi resources (does not matter if connection fails)
    delay(2000);                                          //wait a bit
    if (esp_wifi_restore() != ESP_OK)
    {
      Serial.println("WiFi is not initialized by esp_wifi_init ");
    } else {
      Serial.println("WiFi Configurations Cleared!");
    }
  }
//------------------------------------------------------------------------------------------------------------------------//
 //reset settings - for testing
 // wifiManager.resetSettings();
//------------------------------------------------------------------------------------------------------------------------//  
  wifiManager.setTimeout(120);
  /*  ใช้ได้ 2 กรณี
  1.  เมื่อกดปุ่มเพื่อ Config ค่า AP แล้ว จะขึ้นชื่อ AP ที่เราตั้งขึ้น
      ช่วงนี้ให้เราทำการตั้งค่า SSID+Password หรืออื่นๆทั้งหมด ภายใน 60 วินาที ก่อน AP จะหมดเวลา
      ไม่เช่นนั้น เมื่อครบเวลา 60 วินาที MCU จะ Reset เริ่มต้นใหม่ ให้เราตั้งค่าอีกครั้งภายใน 60 วินาที
  2.  ช่วงไฟดับ Modem router + MCU จะดับทั้งคู่ และเมื่อมีไฟมา ทั้งคู่ก็เริ่มทำงานเช่นกัน
      โดยปกติ Modem router จะ Boot ช้ากว่า  MCU ทำให้ MCU กลับไปเป็น AP รอให้เราตั้งค่าใหม่
      ดังนั้น AP จะรอเวลาให้เราตั้งค่า 60 วินาที ถ้าไม่มีการตั้งค่าใดๆ เมื่อครบ 60 วินาที MCU จะ Reset อีกครั้ง
      ถ้า Modem router  Boot และใช้งานได้ภายใน 60 วินาที และหลังจากที่ MCU Resset และเริ่มทำงานใหม่
      ก็จะสามารถเชื่อมต่อกับ  Modem router ที่ Boot และใช้งานได้แล้ว  ได้  ระบบจะทำงานปกติ
  */
//------------------------------------------------------------------------------------------------------------------------//    
  if (!wifiManager.autoConnect("ESP32_SMF01","password")) {
    Serial.println("failed to connect and hit timeout");//แสดงข้อความใน Serial Monitor
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();//แก้ เดิม ESP.reset(); ใน Esp8266
    delay(5000);
  }
//------------------------------------------------------------------------------------------------------------------------//  
  Serial.println("Connected.......OK!)");//แสดงข้อความใน Serial Monitor
  strcpy(blynk_token, custom_blynk_token.getValue());
//------------------------------------------------------------------------------------------------------------------------//    
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
//------------------------------------------------------------------------------------------------------------------------//   
  Serial.println("local ip"); //แสดงข้อความใน Serial Monitor
  delay(100);
  Serial.println(WiFi.localIP());//แสดงข้อความใน Serial Monitor
//------------------------------------------------------------------------------------------------------------------------//
 //------------------------------------------------------------------------------------------------------------------------//   
  Blynk.config(blynk_token, server, port);
//------------------------------------------------------------------------------------------------------------------------//

  timer.setInterval(100L,hum_temp_light01);           //Read Modbus Light/Temp/Humi
  timer.setInterval(110L,soil_moisture_sensor01);     //Read Modbus Soil Moisture sensor
  timer.setInterval(10000L, reconnectblynk);          //Function reconnect blynk 
  timer.setInterval(5000L, clockDisplay);
}

//Blynk conneted
BLYNK_CONNECTED()
{
 Blynk.syncAll(); 
 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(ledbb,LOW);
    Serial.println("ledbb on");
 }
}
//------------------------------------------------------------------------------------------------------------------------//
//Reconnect to blynk
void reconnectblynk()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    Blynk.syncAll();
    digitalWrite(ledbb, LOW); //ledpin for check blynk connected 
  }
  
  if (blynkIsDownCount >=5){
    ESP.restart();
  }
}
//------------------------------------------------------------------------------------------------------------------------//

//Display Current Date/Time
void clockDisplay()
{
  display.clear();
  display.drawString(0,5,"esp32_site3n3");
  display.drawString(0,25,"Time :" + String(hour()) + ":" + minute() + ":" + second());
  display.drawString(0,40,"Date :" + String(day()) + " /" + month() + " /" + year()); 
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  // Send time to the App
  Blynk.virtualWrite(V40, currentTime);
  // Send date to the App
  Blynk.virtualWrite(V41, currentDate);
}

//3IN1 Humidity, Temperature & Light
void hum_temp_light01() {

//===============================================================================================
// telegram_gzws01 0: Read Input Register for GZWS-N01 Light/Temperature/Humidity
//===============================================================================================
    telegram_gzws01[0].u8id = 1;                          // Slave Address
    telegram_gzws01[0].u8fct = 4;                         // Function 0x04(Read Input Register)  
    telegram_gzws01[0].u16RegAdd = 0;                     // Start Address Read(0x0000)
    telegram_gzws01[0].u16CoilsNo = 8;                    // Number of Register to Read(8 Input Register)
    telegram_gzws01[0].au16reg = InputRegister_gzws01;    // Pointer to Buffer Save Input Register
//===============================================================================================
  
//===============================================================================================
// telegram_gzws01 1: Read Holding Register for GZWS-N01 Light/Temperature/Humidity
//===============================================================================================
    telegram_gzws01[1].u8id = 1;                          // Slave Address
    telegram_gzws01[1].u8fct = 3;                         // Function 0x03(Read Holding Register)
    telegram_gzws01[1].u16RegAdd = 2000;                  // Start Address Read(0x0200:512)  
    telegram_gzws01[1].u16CoilsNo = 1;                    // Number of Register to Read(1 Holding Register)
    telegram_gzws01[1].au16reg = HoldingRegister_gzws01;  // Pointer to Buffer Save Holding Register
//===============================================================================================

uint16_t humi1_int16_value;
float humi1_float_value;
float humi1_percentage1;

uint16_t temp1_int16_value;
float temp1_float_value;
float temp1_percentage1;

uint16_t light1_int16_value;
float light1_float_value;
float light1_percentage1;

switch( u8state ) 
{
case 0:       
  if (millis() > u32wait) u8state++;                // wait state
    break;
    
case 1: 
  master.query(telegram_gzws01[u8query]);           // send query (only once)
  u8state++;
  u8query++;
    if(u8query > 1) u8query = 0;                      // telegram_gzws01[0],telegram_gzws01[1], ----> ,telegram_gzws01[0]
      break;
    
case 2:
  if(master.poll())                                 // check incoming messages
    {
      Serial.print("Light, Temp & Humi Modbus Slave ID = ");
      Serial.println(HoldingRegister_gzws01[0]);
        
// Modbus Input Register
// 0 = Humidity/10
// 1 = Temperature/10
// 2 = Light Law data
  humi1_int16_value = InputRegister_gzws01[0];          // Humidity/10
  temp1_int16_value = InputRegister_gzws01[1];          // Temperature/10
  light1_int16_value = InputRegister_gzws01[2];         // Light Law Data

//Humidity1
  Serial.print("Humidity Value = ");
  Serial.print(humi1_int16_value);                                                      
  Serial.print(" : ");
  Serial.print((float)humi1_int16_value/10.0);                                         
  humi1_percentage1 = ((float)humi1_int16_value/10.0);
  Serial.print("\t");
  Serial.print(humi1_percentage1);
  Blynk.virtualWrite(V21,humi1_percentage1);
  Serial.print("%RH");
  Serial.println();
//Temperature1        
  Serial.print("Temperature Value = ");
  Serial.print(temp1_int16_value);                                                     
  Serial.print(" : ");
  Serial.print((float)temp1_int16_value/10.0);                                          
  temp1_percentage1 = ((float)temp1_int16_value/10.0);
  Serial.print("\t");
  Serial.print(temp1_percentage1);
  Blynk.virtualWrite(V22,temp1_percentage1);
  Serial.print("C");
  Serial.println();
//Light
  Serial.print("Light Value = ");
  Serial.print(light1_int16_value);                                                       
  Serial.print(" : ");
  Serial.print(light1_int16_value);
  Blynk.virtualWrite(V23,light1_int16_value);
  Serial.print("Lux");
  Serial.println();
      
    }  
  if(master.getState() == COM_IDLE) 
    {
      u8state = 0;
      u32wait = millis() + 1000; 
    }
    break;
  }
}

//Modbus Soil Moisture Sensor1
void soil_moisture_sensor01() {
//===============================================================================================
// telegram 0: Read Input Register for Soil Moisture Sensor01
//===============================================================================================
  telegram_soil01[0].u8id = 5;                          // Slave Address
  telegram_soil01[0].u8fct = 4;                         // Function 0x04(Read Input Register)  
  telegram_soil01[0].u16RegAdd = 0;                     // Start Address Read(0x0000)
  telegram_soil01[0].u16CoilsNo = 8;                    // Number of Register to Read(8 Input Register)
  telegram_soil01[0].au16reg = InputRegister_soil01;    // Pointer to Buffer Save Input Register
//===============================================================================================
  
//===============================================================================================
// telegram 1: Read Holding Register for Soil Moisture Sensor01
//===============================================================================================
  telegram_soil01[1].u8id = 5;                          // Slave Address
  telegram_soil01[1].u8fct = 3;                         // Function 0x03(Read Holding Register)
  telegram_soil01[1].u16RegAdd = 512;                   // Start Address Read(0x0200:512)  
  telegram_soil01[1].u16CoilsNo = 1;                    // Number of Register to Read(1 Holding Register)
  telegram_soil01[1].au16reg = HoldingRegister_soil01;  // Pointer to Buffer Save Holding Register
  //===============================================================================================
  
  uint16_t moisture_int16_value;
  float moisture_float_value;
  float moisture_percentage1;

  switch( u8state1 ) 
  {
    case 0: 
      if (millis() > u32wait1) u8state1++;              // wait state
    break;
    
    case 1: 
      master.query(telegram_soil01[u8query1]);          // send query (only once)
      u8state1++;
      u8query1++;
      if(u8query1 > 1) u8query1 = 0;                    // telegram[0],telegram[1], ----> ,telegram[0]
    break;
    
    case 2:
      if(master.poll())                               // check incoming messages
      {
        Serial.print("Soil Moisture Modbus Slave ID = ");
        Serial.println(HoldingRegister_soil01[0]);
 
        // Modbus Input Register
        // 0 = Soil Temperature/100
        // 1 = Soil Moisture/100
        moisture_int16_value = InputRegister_soil01[1];   // Soil Moisture/100
        Serial.print("Soil Moisture Value = ");
        Serial.print(moisture_int16_value);               // Soil Moisture/100
        Serial.print(" : ");
        moisture_percentage1 = ((float)moisture_int16_value/100.0);
        Serial.print(moisture_percentage1);
        Blynk.virtualWrite(V24,moisture_percentage1);
        Serial.print("%RH");
        Serial.println();
      }  
     
    if(master.getState() == COM_IDLE) 
      {
        u8state1 = 0;
        u32wait1 = millis() + 1000; 
      }
    break;
  }
}
//****BUTTON ON/OFF Valve1****
BLYNK_WRITE(Widget_Btn_Valve1){
      int valueValve1 = param.asInt();
      if(valueValve1 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size = master.poll();
        master.query(telegram_relay[0]);  
        au16dataRelay[0] = 0x0100; 
        //Blynk.virtualWrite(V2,0);                
      }
       else{                    
       pool_size = master.poll();
       master.query(telegram_relay[0]);  
       au16dataRelay[0] = 0x0200; 
       //Blynk.virtualWrite(V2,255); 
     }
} 

//****BUTTON ON/OFF Valve2****
BLYNK_WRITE(Widget_Btn_Valve2){
      int valueValve2 = param.asInt();
      if(valueValve2 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size = master.poll();
        master.query(telegram_relay[1]);  
        au16dataRelay[1] = 0x0100;               
      }
       else{                    
       pool_size = master.poll();
       master.query(telegram_relay[1]);  
       au16dataRelay[1] = 0x0200; 
     }
} 

//****BUTTON ON/OFF Valve3****
BLYNK_WRITE(Widget_Btn_Valve3){
      int valueValve3 = param.asInt();
      if(valueValve3 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size = master.poll();
        master.query(telegram_relay[2]);  
        au16dataRelay[2] = 0x0100;               
      }
       else{                    
       pool_size = master.poll();
       master.query(telegram_relay[2]);  
       au16dataRelay[2] = 0x0200; 
     }
} 

//****BUTTON ON/OFF Valve4****
BLYNK_WRITE(Widget_Btn_Valve4){
      int valueValve4 = param.asInt();
      if(valueValve4 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size = master.poll();
        master.query(telegram_relay[3]);  
        au16dataRelay[3] = 0x0100;               
      }
       else{                    
       pool_size = master.poll();
       master.query(telegram_relay[3]);  
       au16dataRelay[3] = 0x0200; 
     }
} 

//****BUTTON ON/OFF Valve5****
BLYNK_WRITE(Widget_Btn_Valve5){
      int valueValve5 = param.asInt();
      if(valueValve5 == 1){    
        //Modbus command to ON/OFF Relay            
        pool_size = master.poll();
        master.query(telegram_relay[4]);  
        au16dataRelay[4] = 0x0100;               
      }
       else{                    
       pool_size = master.poll();
       master.query(telegram_relay[4]);  
       au16dataRelay[4] = 0x0200; 
     }
} 

//****BUTTON ON/OFF Valve6****
BLYNK_WRITE(Widget_Btn_Valve6){
      int valueValve6 = param.asInt();
      if(valueValve6 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size = master.poll();
        master.query(telegram_relay[5]);  
        au16dataRelay[5] = 0x0100;               
      }
       else{                    
       pool_size = master.poll();
       master.query(telegram_relay[5]);  
       au16dataRelay[5] = 0x0200; 
     }
} 

//****BUTTON ON/OFF Valve7****
BLYNK_WRITE(Widget_Btn_Valve7){
      int valueValve7 = param.asInt();
      if(valueValve7 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size = master.poll();
        master.query(telegram_relay[6]);  
        au16dataRelay[6] = 0x0100;               
      }
       else{                    
       pool_size = master.poll();
       master.query(telegram_relay[6]);  
       au16dataRelay[6] = 0x0200; 
     }
} 

//****BUTTON ON/OFF Valve8****
BLYNK_WRITE(Widget_Btn_Valve8){
      int valueValve8 = param.asInt();
      if(valueValve8 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size = master.poll();
        master.query(telegram_relay[7]);  
        au16dataRelay[7] = 0x0100;               
      }
       else{                    
       pool_size = master.poll();
       master.query(telegram_relay[7]);  
       au16dataRelay[7] = 0x0200; 
     }
} 

//****BUTTON ON/OFF Valve9****
BLYNK_WRITE(Widget_Btn_Valve9){
      int valueValve9 = param.asInt();
      if(valueValve9 == 1){    
        //Modbus command to ON/OFF Relay          
        pool_size = master.poll();
        master.query(telegram_relay[8]);  
        au16dataRelay[8] = 0x0100;               
      }
       else{                    
       pool_size = master.poll();
       master.query(telegram_relay[8]);  
       au16dataRelay[8] = 0x0200; 
     }
} 

//****BUTTON ON/OFF Valve10****
BLYNK_WRITE(Widget_Btn_Valve10){
      int valueValve10 = param.asInt();
      if(valueValve10 == 1){    
        //Modbus command to ON/OFF Relay          
        pool_size = master.poll();
        master.query(telegram_relay[9]);  
        au16dataRelay[9] = 0x0100;               
      }
       else{                    
       pool_size = master.poll();
       master.query(telegram_relay[9]);  
       au16dataRelay[9] = 0x0200; 
     }
} 


//****BUTTON ON/OFF Valve11****
BLYNK_WRITE(Widget_Btn_Valve11){
      int valueValve11 = param.asInt();
      if(valueValve11 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size = master.poll();
        master.query(telegram_relay[10]);  
        au16dataRelay[10] = 0x0100;               
      }
       else{                    
       pool_size = master.poll();
       master.query(telegram_relay[10]);  
       au16dataRelay[10] = 0x0200; 
     }
} 

//****BUTTON ON/OFF Valve12****
BLYNK_WRITE(Widget_Btn_Valve12){
      int valueValve12 = param.asInt();
      if(valueValve12 == 1){    
        //Modbus command to ON/OFF Relay          
        pool_size = master.poll();
        master.query(telegram_relay[11]);  
        au16dataRelay[11] = 0x0100;               
      }
       else{                    
       pool_size = master.poll();
       master.query(telegram_relay[11]);  
       au16dataRelay[11] = 0x0200; 
     }
} 

//****BUTTON ON/OFF Valve13****
BLYNK_WRITE(Widget_Btn_Valve13){
      int valueValve13 = param.asInt();
      if(valueValve13 == 1){    
        //Modbus command to ON/OFF Relay          
        pool_size = master.poll();
        master.query(telegram_relay[12]);  
        au16dataRelay[12] = 0x0100;               
      }
       else{                    
       pool_size = master.poll();
       master.query(telegram_relay[12]);  
       au16dataRelay[12] = 0x0200; 
     }
} 

//****BUTTON ON/OFF Valve14****
BLYNK_WRITE(Widget_Btn_Valve14){
      int valueValve14 = param.asInt();
      if(valueValve14 == 1){    
        //Modbus command to ON/OFF Relay        
        pool_size = master.poll();
        master.query(telegram_relay[13]);  
        au16dataRelay[13] = 0x0100;               
      }
       else{                    
       pool_size = master.poll();
       master.query(telegram_relay[13]);  
       au16dataRelay[13] = 0x0200; 
     }
} 

//****BUTTON ON/OFF Valve15****
BLYNK_WRITE(Widget_Btn_Valve15){
      int valueValve15 = param.asInt();
      if(valueValve15 == 1){    
        //Modbus command to ON/OFF Relay          
        pool_size = master.poll();
        master.query(telegram_relay[14]);  
        au16dataRelay[14] = 0x0100;               
      }
       else{                    
       pool_size = master.poll();
       master.query(telegram_relay[14]);  
       au16dataRelay[14] = 0x0200; 
     }
} 

//****BUTTON ON/OFF Valve16****
BLYNK_WRITE(Widget_Btn_Valve16){
      int valueValve16 = param.asInt();
      if(valueValve16 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size = master.poll();
        master.query(telegram_relay[15]);  
        au16dataRelay[15] = 0x0100;               
      }
       else{                    
       pool_size = master.poll();
       master.query(telegram_relay[15]);  
       au16dataRelay[15] = 0x0200; 
     }
} 

//Loop Function
void loop() 
{
   if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();
}
