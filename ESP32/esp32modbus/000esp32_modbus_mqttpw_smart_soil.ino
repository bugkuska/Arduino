//อ้างจาก https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/
/*
Hardware
- Esp32+sheild
- Modbus Soil Moisture Sensor
- I2C SHCT3 Temperature
- Build-in Relay 4 ch
- External Relay 3 ch
Software
- Arduino IDE
- Blynk 
- Blynk Energy Require = 6,000
- QR-Code Project https://drive.google.com/drive/folders/1JhWiXRPM2ByscpiX286XVaSsY9tuvpRJ?usp=sharing
- MQTT
- Node-Red
*/
/////////////////////////////////////////Define Virtual Pin////////////////////////////////
//V1 ไฟสถานะปุ่ม Valve1
//V2 ปุ่ม เปิด-ปิด Valve1
//V3 ไฟสถานะปุ่ม Valve2
//V4 ปุ่ม เปิด-ปิด Valve2
//V5 ไฟสถานะปุ่ม Valve3
//V6 ปุ่ม เปิด-ปิด Valve3
//V7 ไฟสถานะปุ่ม Valve4
//V8 ปุ่ม เปิด-ปิด Valve4
//V9 ไฟสถานะปุ่ม pump
//V10 ปุ่ม เปิด-ปิด pump
//V11 Auto&Manual valve1
//V12 Slider Valve1
//V20 Current Time
//V21 Current Date
//V22 ไฟสถานะปุ่ม พัดลมระบายอากาศ
//V23 ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
//V25 Modbus Soil moisture sensor 1
//V30 Humidity
//V31 Temperature
//V40 Time
//V41 Date
//------------------------------------------------------------------------------------------------------------------------//
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <SPIFFS.h>               //SPIFF สำหรับเก็บค่า
#include <WiFi.h>                 //https://github.com/esp8266/Arduino
#include <WiFiClient.h>
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
char server[] = "blynk-cloud.com";                    //ถ้า custom server ให้เปลี่ยน server ตรงนี้ด้วย
int port = 8442;
char blynk_token[34] = "";                            //ใส่ Blynk_token ของเราที่ Blynk ส่งมาทาง Email ตอน Create Project ใหม่
//------------------------------------------------------------------------------------------------------------------------//
#include <Wire.h> 
#include "ETT_ModbusRTU.h"
#include <HardwareSerial.h>
#define SerialDebug  Serial                           // USB Serial(Serial0)
#define SerialRS485_RX_PIN    16
#define SerialRS485_TX_PIN    17
#define SerialRS485  Serial1                          // Serial1(IO17=TXD,IO16=RXD)
#define RS485_DIRECTION_PIN   23                      // ESP32-WROVER :IO23
#define RS485_RXD_SELECT      LOW
#define RS485_TXD_SELECT      HIGH
//------------------------------------------------------------------------------------------------------------------------//
//Modbus Soil Moisture Sensor
uint16_t InputRegister[8];                            // 0x04(Read Input Register)   
uint16_t HoldingRegister[8];                          // 0x03(Holding Register) pointer to message query
uint16_t moisture_int16_value;
float moisture_float_value;
float moisture_percentage1;
int8_t pool_size1;
modbus_t telegramsoil[2];                             // 2-Modbus Frame Service
//------------------------------------------------------------------------------------------------------------------------//
//Modbus Node
Modbus master(0,                                      // node id = 0(master)
              SerialRS485,                            // Serial(2)
              RS485_DIRECTION_PIN);                   // RS485 Modbus
//------------------------------------------------------------------------------------------------------------------------//
//Define pin for clear and config AP&Token
#define AP_Config       14
//------------------------------------------------------------------------------------------------------------------------//
//10A 4channel Relay extend 
#define pump            27                            // Relay connect to pump 
#define ledbb           26                            //Check blynk connected 
#define ledfan           2                            //Cooling Fan
//------------------------------------------------------------------------------------------------------------------------//
//10A Relay on board esp32all
#define Relay1_Valve1  19                             //valve1
#define Relay2_Valve2  18                             //valve2
#define Relay3_Valve3  5                              //valve3
#define Relay4_Valve4  25                             //valve4 
//------------------------------------------------------------------------------------------------------------------------//
//Define pin for analog input
#define INPUT_1_A1 39                                 //Analog input 1
#define INPUT_2_A2 34                                 //Analog input 2
#define INPUT_3_A3 35                                 //Analog input 3
#define INPUT_4_A4 32                                 //Analog input 4
//------------------------------------------------------------------------------------------------------------------------//
 //RTC Widget
#include <TimeLib.h>
#include <WidgetRTC.h>
WidgetRTC rtc;
//------------------------------------------------------------------------------------------------------------------------//
//OLED Display
#include <Wire.h>                                     // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"                              // legacy include: `#include "SSD1306.h"`
SSD1306Wire  display(0x3c, 21, 22);
//------------------------------------------------------------------------------------------------------------------------//
//SHCT3 Humidity and Temperature Sensor
#include <Arduino.h>
#include "SHTC3.h"
SHTC3 s(Wire);
//------------------------------------------------------------------------------------------------------------------------//
//MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
const char* mqtt_server = "";                         //ใส่ IP Address ของ MQTT Server
WiFiClient esp32_modbus04;                            //ถ้าเชื่อมต่อ MQTT Server หลาย Node พร้อมๆ กันให้เปลี่ยนชื่อ
PubSubClient client(mqtt_server, 1883, callback,esp32_modbus04);
long lastMsg1 = 0;
long lastMsg2 = 0;
//------------------------------------------------------------------------------------------------------------------------//
//Valve1
//Slider for set Light limit
bool switchStatus1 = 0;                               // 0 = manual,1=auto
int SoilsensorLimit1 = 0;
bool manualSwitch1 = 0;
#define Widget_LED_Valve1 V1                          //ไฟสถานะปุ่ม Valve1
#define Widget_Btn_Valve1 V2                          //ปุ่ม เปิด-ปิด Valve1
WidgetLED LedBlynkValve1(Widget_LED_Valve1);
//------------------------------------------------------------------------------------------------------------------------//
//Valve2
#define Widget_LED_Valve2 V3                          //ไฟสถานะปุ่ม Valve2
#define Widget_Btn_Valve2 V4                          //ปุ่ม เปิด-ปิด Valve2
WidgetLED LedBlynkValve2(Widget_LED_Valve2);
//------------------------------------------------------------------------------------------------------------------------//
//Valve3
#define Widget_LED_Valve3 V5                          //ไฟสถานะปุ่ม Valve3
#define Widget_Btn_Valve3 V6                          //ปุ่ม เปิด-ปิด Valve3
WidgetLED LedBlynkValve3(Widget_LED_Valve3);
//------------------------------------------------------------------------------------------------------------------------//
//Valve4
#define Widget_LED_Valve4 V7                          //ไฟสถานะปุ่ม Valve4
#define Widget_Btn_Valve4 V8                          //ปุ่ม เปิด-ปิด Valve4
WidgetLED LedBlynkValve4(Widget_LED_Valve4);
//------------------------------------------------------------------------------------------------------------------------//
//Pump
#define Widget_LED_Pump V9                            //ไฟสถานะปุ่ม pump
#define Widget_Btn_Pump V10                           //ปุ่ม เปิด-ปิด pump
WidgetLED LedBlynkPump(Widget_LED_Pump);
//------------------------------------------------------------------------------------------------------------------------//
//Cooling Fan
#define Widget_LED_Fan V22                            //ไฟสถานะปุ่ม พัดลมระบายอากาศ
#define Widget_Btn_Fan V23                            //ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
WidgetLED LedBlynkFan(Widget_LED_Fan);
//------------------------------------------------------------------------------------------------------------------------//
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
//------------------------------------------------------------------------------------------------------------------------//
//Setup 
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
  pinMode(Relay1_Valve1,OUTPUT);      // Pin 19 relay 10A in1 valve1
  pinMode(Relay2_Valve2,OUTPUT);      // Pin 18 relay 10A in2 valve2
  pinMode(Relay3_Valve3,OUTPUT);      // Pin 5  relay 10A in3 valve3
  pinMode(Relay4_Valve4,OUTPUT);      // Pin 25 relay 10A in4 valve4
//------------------------------------------------------------------------------------------------------------------------//  
  // Set Defult Relay Status
  digitalWrite(pump, HIGH);           // Pin 23 relay 10A in1 connect to Manatic + overload
  digitalWrite(ledbb,HIGH);           // Pin 27 relay 10A in2 connect to Green PoltLamp check blynk connected
  digitalWrite(ledfan,HIGH);          // Pin 26 relay 10A in3 connect to Red PoltLamp start cooling fan  
  digitalWrite(Relay1_Valve1,HIGH);   // Pin 19 relay 10A in1 valve1
  digitalWrite(Relay2_Valve2,HIGH);   // Pin 18 relay 10A in2 valve2
  digitalWrite(Relay3_Valve3,HIGH);   // Pin 5  relay 10A in3 valve3
  digitalWrite(Relay4_Valve4,HIGH);   // Pin 25 relay 10A in4 valve4
//------------------------------------------------------------------------------------------------------------------------//
  //Begin read Humidity and Temperature ==> SHCT3
  Wire.begin();
//------------------------------------------------------------------------------------------------------------------------//
  //Begin Sync time
  rtc.begin();
//------------------------------------------------------------------------------------------------------------------------//  
  //rs485
  pinMode(RS485_DIRECTION_PIN, OUTPUT);             // RS485 Direction
  digitalWrite(RS485_DIRECTION_PIN, RS485_RXD_SELECT);
//------------------------------------------------------------------------------------------------------------------------//
  SerialDebug.begin(115200);
  while(!SerialDebug);
//------------------------------------------------------------------------------------------------------------------------//
  SerialRS485.begin(9600, SERIAL_8N1, SerialRS485_RX_PIN, SerialRS485_TX_PIN);
  while(!SerialRS485);
//------------------------------------------------------------------------------------------------------------------------//
  // telegram 0: Read Input Register from Soil Moisture sensor
  telegramsoil[0].u8id = 1;                     // Slave Address
  telegramsoil[0].u8fct = 4;                    // Function 0x04(Read Input Register)  
  telegramsoil[0].u16RegAdd = 0;                // Start Address Read(0x0000)
  telegramsoil[0].u16CoilsNo = 8;               // Number of Register to Read(8 Input Register)
  telegramsoil[0].au16reg = InputRegister;      // Pointer to Buffer Save Input Register
//------------------------------------------------------------------------------------------------------------------------//
  // telegram 1: Read Holding Register from Soil Moisture sensor
  telegramsoil[1].u8id = 1;                     // Slave Address
  telegramsoil[1].u8fct = 3;                    // Function 0x03(Read Holding Register)
  telegramsoil[1].u16RegAdd = 512;              // Start Address Read(0x0200:512)  
  telegramsoil[1].u16CoilsNo = 1;               // Number of Register to Read(1 Holding Register)
  telegramsoil[1].au16reg = HoldingRegister;    // Pointer to Buffer Save Holding Register
//------------------------------------------------------------------------------------------------------------------------//
  master.begin(SerialRS485);                    // Mosbus Interface RS485
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
  if (!wifiManager.autoConnect("esp32_site04","password")) {
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
 //MQTT
  client.setServer(mqtt_server, 1883);
//------------------------------------------------------------------------------------------------------------------------//   
  Blynk.config(blynk_token, server, port);
//------------------------------------------------------------------------------------------------------------------------//
//Timer interval  
  timer.setInterval(10000L,sht3Data);
  timer.setInterval(1000L,soil1);    //Read Modbus soil moisture sensor  
  timer.setInterval(10000L, clockDisplay);
  timer.setInterval(10000L, reconnectblynk);  //Function reconnect blynk 
}
//------------------------------------------------------------------------------------------------------------------------//
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
}
//------------------------------------------------------------------------------------------------------------------------//
//Reconnect mqtt
void reconnectmqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect    
    if (client.connect("esp32_modbus01", "ใส่ MQTT User" , "ใส่ MQTT Password")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      //ESP.restart();
    }
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
//------------------------------------------------------------------------------------------------------------------------//
// update switchStatus1 on Soil sensor value
BLYNK_WRITE(V11)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}

// update Soil Moisture sensor setting
BLYNK_WRITE(V12)
{   
  SoilsensorLimit1 = param.asInt(); // Get value as integer
}

// update manualSwitch
BLYNK_WRITE(V2)
{
  manualSwitch1 = param.asInt();
}
//------------------------------------------------------------------------------------------------------------------------//
void soil1() { 
  pool_size1 = master.poll();
  master.query(telegramsoil[0]);  
  moisture_int16_value = InputRegister[1];                                                  
  moisture_percentage1 = ((float)moisture_int16_value/100.0);
  Blynk.virtualWrite(V25,moisture_percentage1);

  //Publish data to MQTT
    long now = millis();
    if (now - lastMsg1 > 30000) {
    lastMsg1 = now;   
    static char SoilMoisture1[7];
    dtostrf(moisture_percentage1, 6, 2, SoilMoisture1);    
    // Publishes Temperature and Humidity values
    client.publish("esp32_site04/SoilMoisture1", SoilMoisture1);
//------------------------------------------------------------------------------------------------------------------------//
 //Auto&Manual Switch
  if(switchStatus1)
  {
    // auto
    if(moisture_percentage1 < SoilsensorLimit1)
    {
        digitalWrite(Relay1_Valve1, LOW);  
        Blynk.virtualWrite(V2, 1);               
        Blynk.setProperty(Widget_LED_Valve1, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์ว");
        LedBlynkValve1.on(); 
                
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on();
    }  
    else
    {
        digitalWrite(Relay1_Valve1, HIGH);
        Blynk.virtualWrite(V2, 0);
        Blynk.virtualWrite(Widget_LED_Valve1, 0);
        Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์ว");                       
        LedBlynkValve1.off();  
                
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();
    }
  }
  else
  {
    if(manualSwitch1)
    {
        digitalWrite(Relay1_Valve1, LOW);        
        Blynk.setProperty(Widget_LED_Valve1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์ว");
        LedBlynkValve1.on(); 
        
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); 
    }
    else
    {
        digitalWrite(Relay1_Valve1, HIGH);
        Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์ว");                       
        LedBlynkValve1.off();
        
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off(); 
    }
}
}
}
//------------------------------------------------------------------------------------------------------------------------//
//SHCT3  
void sht3Data(){
int Temp;
int Humid;  
s.begin(true);
s.sample();
Temp = s.readTempC();
Humid = s.readHumidity();    
Blynk.virtualWrite(V30, s.readHumidity());
Blynk.virtualWrite(V31, s.readTempC());

//Publish data to MQTT
    long now = millis();
    if (now - lastMsg2 > 30000) {
    lastMsg2 = now;    
    static char temperature[7];
    dtostrf(Temp, 6, 2, temperature);    
    static char humidity[7];
    dtostrf(Humid, 6, 2, humidity);
    // Publishes Temperature and Humidity values
    client.publish("esp32_site04/temperature", temperature);
    client.publish("esp32_site04/humidity", humidity);
  
    //******AUTO Cooling FAN*******
  if (Temp >= 33){       
    digitalWrite(ledfan, LOW);
    Blynk.virtualWrite(Widget_Btn_Fan, 1);       
    Blynk.setProperty(Widget_LED_Fan, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Fan, "label", "พัดลมกำลังทำงาน");
    LedBlynkFan.on();
    }   

  if (Temp < 33){
    digitalWrite(ledfan, HIGH);        
    Blynk.virtualWrite(Widget_Btn_Fan, 0);
    Blynk.setProperty(Widget_LED_Fan, "label", "ปิดพัดลมแล้ว");                        
    LedBlynkFan.off();  
    }       
}
}
//------------------------------------------------------------------------------------------------------------------------//
/*
//****BUTTON ON/OFF Valve1****
BLYNK_WRITE(Widget_Btn_Valve1){
int valueValve1 = param.asInt();
  if(valueValve1 == 1){
    digitalWrite(Relay1_Valve1, LOW);
    Blynk.setProperty(Widget_LED_Valve1, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์ว 1");
    LedBlynkValve1.on();
    digitalWrite(pump, LOW);
    Blynk.virtualWrite(Widget_Btn_Pump,1);
    Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
    LedBlynkPump.on(); 
    }
    else{              
    digitalWrite(Relay1_Valve1, HIGH);
    Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์ว 1");
    LedBlynkValve1.off();     

    digitalWrite(pump, HIGH);      
    Blynk.virtualWrite(Widget_Btn_Pump,0);                
    Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
    LedBlynkPump.off();           
    }
}
  */
//------------------------------------------------------------------------------------------------------------------------//     
//****BUTTON ON/OFF Valve2****
BLYNK_WRITE(Widget_Btn_Valve2){
int valueValve2 = param.asInt();
  if(valueValve2 == 1){
    digitalWrite(Relay2_Valve2, LOW);
    Blynk.setProperty(Widget_LED_Valve2, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Valve2, "label", "เปิดวาล์ว 2");
    LedBlynkValve2.on();

    digitalWrite(pump, LOW);
    Blynk.virtualWrite(Widget_Btn_Pump,1);
    Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
    LedBlynkPump.on(); 
    }
    else{              
    digitalWrite(Relay2_Valve2, HIGH);
    Blynk.setProperty(Widget_LED_Valve2, "label", "ปิดวาล์ว 2");
    LedBlynkValve2.off();     

    digitalWrite(pump, HIGH);      
    Blynk.virtualWrite(Widget_Btn_Pump,0);                
    Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
    LedBlynkPump.off();           
    }
}
//------------------------------------------------------------------------------------------------------------------------//  
//****BUTTON ON/OFF Valve3****
BLYNK_WRITE(Widget_Btn_Valve3){
int valueValve3 = param.asInt();
  if(valueValve3 == 1){       
    digitalWrite(Relay3_Valve3, LOW);
    Blynk.setProperty(Widget_LED_Valve3, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Valve3, "label", "เปิดวาล์ว 3");
    LedBlynkValve3.on();

    digitalWrite(pump, LOW);
    Blynk.virtualWrite(Widget_Btn_Pump,1);
    Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
    LedBlynkPump.on(); 
    }
    else{              
    digitalWrite(Relay3_Valve3, HIGH);
    Blynk.setProperty(Widget_LED_Valve3, "label", "ปิดวาล์ว 3");
    LedBlynkValve3.off();     

    digitalWrite(pump, HIGH);      
    Blynk.virtualWrite(Widget_Btn_Pump,0);                
    Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
    LedBlynkPump.off();           
    }
}
//------------------------------------------------------------------------------------------------------------------------//      
//****BUTTON ON/OFF Valve4****
BLYNK_WRITE(Widget_Btn_Valve4){
int valueValve4 = param.asInt();
  if(valueValve4 == 1){      
    digitalWrite(Relay4_Valve4, LOW);
    Blynk.setProperty(Widget_LED_Valve4, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Valve4, "label", "เปิดวาล์ว 3");
    LedBlynkValve4.on();

    digitalWrite(pump, LOW);
    Blynk.virtualWrite(Widget_Btn_Pump,1);
    Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
    LedBlynkPump.on(); 
    }
    else{              
    digitalWrite(Relay4_Valve4, HIGH);
    Blynk.setProperty(Widget_LED_Valve4, "label", "ปิดวาล์ว 4");
    LedBlynkValve4.off();     

    digitalWrite(pump, HIGH);      
    Blynk.virtualWrite(Widget_Btn_Pump,0);                
    Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
    LedBlynkPump.off();           
    }
}
//------------------------------------------------------------------------------------------------------------------------//
//****BUTTON ON/OFF Cooling FAN****
BLYNK_WRITE(Widget_Btn_Fan){
int valueFan = param.asInt();
  if(valueFan == 1){
    digitalWrite(ledfan, LOW);
    Blynk.setProperty(Widget_LED_Fan, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Fan, "label", "เปิดพัดลมระบายอากาศ");
    LedBlynkFan.on();
    }
    else{              
    digitalWrite(ledfan, HIGH);
    Blynk.virtualWrite(Widget_Btn_Fan, 0);
    Blynk.setProperty(Widget_LED_Fan, "label", "ปิดพัดลมระบายอากาศ");
    LedBlynkFan.off();                       
    }
}
//------------------------------------------------------------------------------------------------------------------------//
//Loop program here....
void loop() 
{
//MQTT
  if (!client.connected()) {
    reconnectmqtt();
  }
  //client.loop();
  if(!client.loop())
    client.connect("esp32_modbus01");
    
  if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();
}
//------------------------------------------------------------------------------------------------------------------------//
