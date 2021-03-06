//อ้างจาก https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <SPIFFS.h>//เพิ่ม
#include <WiFi.h>                 //https://github.com/esp8266/Arduino
#include <WiFiClient.h>
//needed for library
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //Ver 5.13.4   //https://github.com/bblanchon/ArduinoJson
#include <BlynkSimpleEsp32.h>     //  Blynk_Release_v0.6.1 
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
int blynkIsDownCount = 0;
BlynkTimer timer;
char server[] = "blynk-cloud.com";
int port = 8442;
char blynk_token[34] = "";//ใส่ Blynk_token ของเราที่ Blynk ส่งมาทาง Email ตอน Create Project ใหม่

//Modbus
#define RXD2            16
#define TXD2            17
HardwareSerial rs485(1);
#include "modbusRTU.h"

//Define pin for clear and config AP&Token
#define AP_Config       14

//10A 4channel Relay extend 
#define pump            23        // Relay connect to pump 
#define ledbb           27        //Check blynk connected 
#define ledfan          26        //Cooling Fan

//OLED Display
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
SSD1306Wire  display(0x3c, 21, 22);

/////////////////////////////////////////Define Virtual Pin////////////////////////////////
//***Cooling Fan***//
/*กำหนดเงื่อนไขการทำงานของพัดลมระบายอากาศ*/
//V20 ไฟสถานะปุ่ม Valve1
//V21 ปุ่ม เปิด-ปิด Valve1
//V30 modbus_tempcfan
//V31 modbus_humicfan
//V63 Auto&Manual Cfan
//V64 Slider Cfan
#define Widget_LED_Fan1 V21        //ไฟสถานะปุ่ม พัดลมระบายอากาศ
#define Widget_Btn_Fan1 V22        //ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
WidgetLED LedBlynkFan1(Widget_LED_Fan1);
bool switchStatuscfan = 0; // 0 = manual,1=auto
int cfansensorLimit = 0;
bool manualSwitchcfan = 0;
//***Cooling Fan***//

//*****Valve1*****//
//V1 ไฟสถานะปุ่ม Valve1
//V2 ปุ่ม เปิด-ปิด Valve1
//V32 modbus_temp1 
//V33 modbus_humi1
//V51 Auto&Manual valve1
//V52 Slider Valve1
#define Widget_LED_Valve1 V1              //ไฟสถานะปุ่ม Valve1
#define Widget_Btn_Valve1 V2              //ปุ่ม เปิด-ปิด Valve1
WidgetLED LedBlynkValve1(Widget_LED_Valve1);
//Slider for set Temp&Humi limit
bool switchStatus1 = 0; // 0 = manual,1=auto
int thsensorLimit1 = 0;
bool manualSwitch1 = 0;
//*****Valve1*****//

//*****Valve2*****//
//V3 ไฟสถานะปุ่ม Valve2
//V4 ปุ่ม เปิด-ปิด Valve2
//V34 modbus_temp2
//V35 modbus_humi2
//V53 Auto&Manual valve2
//V54 Slider Valve2
#define Widget_LED_Valve2 V3              //ไฟสถานะปุ่ม Valve2
#define Widget_Btn_Valve2 V4              //ปุ่ม เปิด-ปิด Valve2
WidgetLED LedBlynkValve2(Widget_LED_Valve2);
//Slider for set Temp&Humi limit
bool switchStatus2 = 0; // 0 = manual,1=auto
int thsensorLimit2 = 0;
bool manualSwitch2 = 0;
//*****Valve2*****//

//*****Valve3*****//
//V5 ไฟสถานะปุ่ม Valve3
//V6 ปุ่ม เปิด-ปิด Valve3
//V36 modbus_temp3 
//V37 modbus_humi3
//V55 Auto&Manual valve3
//V56 Slider Valve3
#define Widget_LED_Valve3 V5             //ไฟสถานะปุ่ม Valve3
#define Widget_Btn_Valve3 V6             //ปุ่ม เปิด-ปิด Valve3
WidgetLED LedBlynkValve3(Widget_LED_Valve3);
//Slider for set Temp&Humi limit
bool switchStatus3 = 0; // 0 = manual,1=auto
int thsensorLimit3 = 0;
bool manualSwitch3 = 0;
//*****Valve3*****//

//*****Valve4*****//
//V7 ไฟสถานะปุ่ม Valve4
//V8 ปุ่ม เปิด-ปิด Valve4
//V38 modbus_temp4 
//V39 modbus_humi4
//V57 Auto&Manual valve4
//V58 Slider Valve4
#define Widget_LED_Valve4 V7             //ไฟสถานะปุ่ม Valve4
#define Widget_Btn_Valve4 V8             //ปุ่ม เปิด-ปิด Valve4
WidgetLED LedBlynkValve4(Widget_LED_Valve4);
//Slider for set Temp&Humi limit
bool switchStatus4 = 0; // 0 = manual,1=auto
int thsensorLimit4 = 0;
bool manualSwitch4 = 0;
//*****Valve4*****//

//*****Valve5*****//
//V9 ไฟสถานะปุ่ม Valve5
//V10 ปุ่ม เปิด-ปิด Valve5
//V40 modbus_temp5 
//V41 modbus_humi5
//V59 Auto&Manual valve5
//V60 Slider Valve5
#define Widget_LED_Valve5 V9             //ไฟสถานะปุ่ม Valve5
#define Widget_Btn_Valve5 V10             //ปุ่ม เปิด-ปิด Valve5
WidgetLED LedBlynkValve5(Widget_LED_Valve5);
//Slider for set Temp&Humi limit
bool switchStatus5 = 0; // 0 = manual,1=auto
int thsensorLimit5 = 0;
bool manualSwitch5 = 0;
//*****Valve5*****//

//*****Valve6*****//
//V11 ไฟสถานะปุ่ม Valve6
//V12 ปุ่ม เปิด-ปิด Valve6
//V42 modbus_temp6 
//V43 modbus_humi6
//V61 Auto&Manual valve6
//V62 Slider Valve6
#define Widget_LED_Valve6 V11             //ไฟสถานะปุ่ม Valve6
#define Widget_Btn_Valve6 V12             //ปุ่ม เปิด-ปิด Valve6
WidgetLED LedBlynkValve6(Widget_LED_Valve6);
//Slider for set Temp&Humi limit
bool switchStatus6 = 0; // 0 = manual,1=auto
int thsensorLimit6 = 0;
bool manualSwitch6 = 0;
//*****Valve6*****//

/*ถ้าใช้ pump auto ไม่จำเป็นต้องมีก็ได้*/
//Vx ไฟสถานะปุ่ม pump
//Vx ปุ่ม เปิด-ปิด pump

//RTC Widget
#include <TimeLib.h>
#include <WidgetRTC.h>
WidgetRTC rtc;
//V80 Time
//V81 Date
//////////////////////////////////////////Define Virtual Pin////////////////////////////////

//Callback notifying us of the need to save config
bool shouldSaveConfig = false;
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

//*****Setup*****//
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  rs485.begin(9600, SERIAL_8N1, RXD2, TXD2);
 
  //OLED
  display.init();
  display.display();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.display();
  
  // Setup Pin Mode
  pinMode(AP_Config, INPUT_PULLUP);//กำหนดโหมดใช้งานให้กับขา AP_Config เป็นขา กดปุ่ม ค้าง เพื่อตั้งค่า AP config 
  pinMode(pump, OUTPUT);              // Pin 23 relay 10A in1 connect to Manatic + overload
  pinMode(ledbb,OUTPUT);              // Pin 27 relay 10A in2 connect to Green PoltLamp check blynk connected
  pinMode(ledfan,OUTPUT);             // Pin 26 relay 10A in3 connect to Red PoltLamp start cooling fan 
  
  // Set Defult Relay Status
  digitalWrite(pump, HIGH);           // Pin 23 relay 10A in1 connect to Manatic + overload
  digitalWrite(ledbb,HIGH);           // Pin 27 relay 10A in2 connect to Green PoltLamp check blynk connected
  digitalWrite(ledfan,HIGH);          // Pin 26 relay 10A in3 connect to Red PoltLamp start cooling fan   

  /*Modbus Relay default status*/
  
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

  for (int i = 5; i > -1; i--) {  // นับเวลาถอยหลัง 5 วินาทีก่อนกดปุ่ม AP Config
    //digitalWrite(ledbb, HIGH);
    delay(500);
    //digitalWrite(ledbb, LOW);
    // delay(500);
    Serial.print (String(i) + " ");//แสดงข้อความใน Serial Monitor
   
  }

  if (digitalRead(AP_Config) == LOW) {
   // Serial.println("Button Pressed");//แสดงข้อความใน Serial Monitor
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

  if (!wifiManager.autoConnect("ESP32_Modbus01","password")) {
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
   //client.setServer(mqtt_server, 1883);
   
  Blynk.config(blynk_token);//เริ่มการเชื่อมต่อ Blynk Server แบบปกติ
  //Blynk.config(blynk_token, server, port); //เชื่อมต่อ Blynk Server แบบ custom host
  timer.setInterval(10000L,wtr10e_cfan);    //Read Modbus wtr10e Sensor1
  timer.setInterval(10000L,wtr10e_node1);    //Read Modbus wtr10e Sensor1
  timer.setInterval(10000L,wtr10e_node2);    //Read Modbus wtr10e Sensor2
  timer.setInterval(10000L,wtr10e_node3);    //Read Modbus wtr10e Sensor3
  timer.setInterval(10000L,wtr10e_node4);    //Read Modbus wtr10e Sensor4
  timer.setInterval(10000L,wtr10e_node5);    //Read Modbus wtr10e Sensor5
  timer.setInterval(10000L,wtr10e_node6);    //Read Modbus wtr10e Sensor6
  timer.setInterval(10000L, clockDisplay);  //Display Date/Time
  timer.setInterval(10000L, reconnectblynk);  //Function reconnect blynk  
}
//*****End Setup*****//

//*****Blynk conneted*****//
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
//*****Blynk conneted*****//  

//**Reconnect to blynk**//
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
//**Reconnect to blynk**//

//**Display Current Date/Time**//
void clockDisplay()
{
  display.clear();
  display.drawString(0,5,"#ESP32_Modbus01#");
  display.drawString(0,25,"Time :" + String(hour()) + "/" + minute() + "/" + second());
  display.drawString(0,40,"Date :" + String(day()) + " /" + month() + " /" + year()); 
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  // Send time to the App
  Blynk.virtualWrite(V80, currentTime);
  // Send date to the App
  Blynk.virtualWrite(V81, currentDate);
}
//**Display Current Date/Time**//

///////////////Update switchStatuscfan on Temp&Humi Cooling Fan/////////////
BLYNK_WRITE(V63)
{   
  switchStatuscfan = param.asInt(); // Get value as integer
}
///////////////Update switchStatuscfan on Temp&Humi Cooling Fan//////////////////

////////////////////Update Temp&Humi setting////////////////////////////
BLYNK_WRITE(V64)
{   
  cfansensorLimit = param.asInt(); // Get value as integer
}
////////////////////Update Temp&Humi setting////////////////////////////

/////////////////////Update manualSwitch///////////////////////////
BLYNK_WRITE(V22)
{
  manualSwitchcfan = param.asInt();
}
/////////////////////Update manualSwitch///////////////////////////

///////////////////WTR10-E Temp&Humi Cooling Fan///////////////////
void wtr10e_cfan()
{
int id = 2;
  float tempcfan = sht20ReadTemp_modbusRTU(id);
  float humicfan = sht20ReadHumi_modbusRTU(id);
  Serial.printf("Info: wtr10e[0x02] temperature cfan = %.1f\r\n",tempcfan);
  vTaskDelay(500);
  Serial.printf("Info: wtr10e[0x02] humidity cfan = %.1f\r\n",humicfan);
  vTaskDelay(500);

  Blynk.virtualWrite(V30, tempcfan);
  Blynk.virtualWrite(V31, humicfan);    
 
  if(switchStatuscfan)
  {
    // auto
    if(tempcfan > cfansensorLimit)
    {
        relayControl_modbusRTU(1,1,1);
        Blynk.virtualWrite(V22, 1);                
        Blynk.setProperty(Widget_LED_Fan1, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Fan1, "label", "เปิดพัดลมระบายอากาศ");
        LedBlynkFan1.on(); 
    }  
    else
    {
         relayControl_modbusRTU(1,1,0);
        Blynk.virtualWrite(V22, 0);
        Blynk.virtualWrite(Widget_LED_Fan1, 0);
        Blynk.setProperty(Widget_LED_Fan1, "label", "ปิดพัดลมระบายอากาศ");                       
        LedBlynkFan1.off();  
    }
  }
  else
  {
    if(manualSwitchcfan)
    {
         relayControl_modbusRTU(1,1,1);       
        Blynk.setProperty(Widget_LED_Fan1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Fan1, "label", "เปิดพัดลมระบายอากาศ");
        LedBlynkFan1.on(); 
    }
    else
    {
         relayControl_modbusRTU(1,1,0);
        Blynk.setProperty(Widget_LED_Fan1, "label", "ปิดพัดลมระบายอากาศ");                       
        LedBlynkFan1.off();
    }
    // manaul
  }
}
///////////////////WTR10-E Temp&Humi Cooling Fan/////////////////////////

///////////////Update switchStatus1 on Temp&Humi value1//////////////////
BLYNK_WRITE(V51)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}
///////////////Update switchStatus1 on Temp&Humi value1//////////////////

////////////////////Update Temp&Humi setting////////////////////////////
BLYNK_WRITE(V52)
{   
  thsensorLimit1 = param.asInt(); // Get value as integer
}
////////////////////Update Temp&Humi setting////////////////////////////

/////////////////////Update manualSwitch///////////////////////////
BLYNK_WRITE(V2)
{
  manualSwitch1 = param.asInt();
}
/////////////////////Update manualSwitch///////////////////////////

///////////////////WTR10-E Temp&Humi Node1/////////////////////////
void wtr10e_node1()
{
int id = 3;
  float temp1 = sht20ReadTemp_modbusRTU(id);
  float humi1 = sht20ReadHumi_modbusRTU(id);
  Serial.printf("Info: wtr10e[0x03] temperature1 = %.1f\r\n",temp1);
  vTaskDelay(500);
  Serial.printf("Info: wtr10e[0x03] humidity1 = %.1f\r\n",humi1);
  vTaskDelay(500);

  Blynk.virtualWrite(V32, temp1);
  Blynk.virtualWrite(V33, humi1);    
 
  if(switchStatus1)
  {
    // auto
    if(humi1 <= thsensorLimit1)
    {
        relayControl_modbusRTU(4,1,1);
        Blynk.virtualWrite(V2, 1);                
        Blynk.setProperty(Widget_LED_Valve1, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์ว");
        LedBlynkValve1.on(); 
    }  
    else
    {
         relayControl_modbusRTU(4,1,0);
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
         relayControl_modbusRTU(4,1,1);       
        Blynk.setProperty(Widget_LED_Valve1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์ว");
        LedBlynkValve1.on(); 
    }
    else
    {
         relayControl_modbusRTU(4,1,0);
        Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์ว");                       
        LedBlynkValve1.off();
    }
    // manaul
  }
}
///////////////////WTR10-E Temp&Humi Node1/////////////////////////

///////////////Update switchStatus2 on Temp&Humi value2//////////////////
BLYNK_WRITE(V53)
{   
  switchStatus2 = param.asInt(); // Get value as integer
}
///////////////Update switchStatus2 on Temp&Humi value2//////////////////

////////////////////Update Temp&Humi setting////////////////////////////
BLYNK_WRITE(V54)
{   
  thsensorLimit2 = param.asInt(); // Get value as integer
}
////////////////////Update Temp&Humi setting////////////////////////////

/////////////////////Update manualSwitch///////////////////////////
BLYNK_WRITE(V4)
{
  manualSwitch2 = param.asInt();
}
/////////////////////Update manualSwitch///////////////////////////

///////////////////WTR10-E Temp&Humi Node2/////////////////////////
void wtr10e_node2()
{
int id = 5;
  float temp2 = sht20ReadTemp_modbusRTU(id);
  float humi2 = sht20ReadHumi_modbusRTU(id);
  Serial.printf("Info: wtr10e[0x05] temperature2 = %.1f\r\n",temp2);
  vTaskDelay(500);
  Serial.printf("Info: wtr10e[0x05] humidity2 = %.1f\r\n",humi2);
  vTaskDelay(500);

  Blynk.virtualWrite(V34, temp2);
  Blynk.virtualWrite(V35, humi2);    
 
  if(switchStatus2)
  {
    // auto
    if(humi2 <= thsensorLimit2)
    {
        relayControl_modbusRTU(6,1,1);
        Blynk.virtualWrite(V4, 1);                
        Blynk.setProperty(Widget_LED_Valve2, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve2, "label", "เปิดวาล์ว");
        LedBlynkValve2.on(); 
    }  
    else
    {
         relayControl_modbusRTU(6,1,0);
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
         relayControl_modbusRTU(6,1,1);       
        Blynk.setProperty(Widget_LED_Valve2, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve2, "label", "เปิดวาล์ว");
        LedBlynkValve2.on(); 
    }
    else
    {
         relayControl_modbusRTU(6,1,0);
        Blynk.setProperty(Widget_LED_Valve2, "label", "ปิดวาล์ว");                       
        LedBlynkValve2.off();
    }
    // manaul
  }
}
///////////////////WTR10-E Temp&Humi Node2/////////////////////////


///////////////Update switchStatus3 on Temp&Humi value3//////////////////
BLYNK_WRITE(V55)
{   
  switchStatus3 = param.asInt(); // Get value as integer
}
///////////////Update switchStatus3 on Temp&Humi value3//////////////////

////////////////////Update Temp&Humi setting////////////////////////////
BLYNK_WRITE(V56)
{   
  thsensorLimit3 = param.asInt(); // Get value as integer
}
////////////////////Update Temp&Humi setting////////////////////////////

/////////////////////Update manualSwitch///////////////////////////
BLYNK_WRITE(V6)
{
  manualSwitch3 = param.asInt();
}
/////////////////////Update manualSwitch///////////////////////////

///////////////////WTR10-E Temp&Humi Node3/////////////////////////
void wtr10e_node3()
{
int id = 7;
  float temp3 = sht20ReadTemp_modbusRTU(id);
  float humi3 = sht20ReadHumi_modbusRTU(id);
  Serial.printf("Info: wtr10e[0x07] temperature3 = %.1f\r\n",temp3);
  vTaskDelay(500);
  Serial.printf("Info: wtr10e[0x03] humidity3 = %.1f\r\n",humi3);
  vTaskDelay(500);
  
  Blynk.virtualWrite(V36, temp3);
  Blynk.virtualWrite(V37, humi3);    
 
  if(switchStatus3)
  {
    // auto
    if(humi3 <= thsensorLimit3)
    {
        relayControl_modbusRTU(8,1,1);
        Blynk.virtualWrite(V6, 1);                
        Blynk.setProperty(Widget_LED_Valve3, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve3, "label", "เปิดวาล์ว");
        LedBlynkValve3.on(); 
    }  
    else
    {
         relayControl_modbusRTU(8,1,0);
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
         relayControl_modbusRTU(8,1,1);       
        Blynk.setProperty(Widget_LED_Valve3, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve3, "label", "เปิดวาล์ว");
        LedBlynkValve3.on(); 
    }
    else
    {
         relayControl_modbusRTU(8,1,0);
        Blynk.setProperty(Widget_LED_Valve3, "label", "ปิดวาล์ว");                       
        LedBlynkValve3.off();
    }
    // manaul
  }
}
///////////////////WTR10-E Temp&Humi Node3/////////////////////////


///////////////Update switchStatus4 on Temp&Humi value4//////////////////
BLYNK_WRITE(V57)
{   
  switchStatus4 = param.asInt(); // Get value as integer
}
///////////////Update switchStatus4 on Temp&Humi value4//////////////////

////////////////////Update Temp&Humi setting////////////////////////////
BLYNK_WRITE(V58)
{   
  thsensorLimit4 = param.asInt(); // Get value as integer
}
////////////////////Update Temp&Humi setting////////////////////////////

/////////////////////Update manualSwitch///////////////////////////
BLYNK_WRITE(V8)
{
  manualSwitch4 = param.asInt();
}
/////////////////////Update manualSwitch///////////////////////////

///////////////////WTR10-E Temp&Humi Node4/////////////////////////
void wtr10e_node4()
{
int id = 9;
  float temp4 = sht20ReadTemp_modbusRTU(id);
  float humi4 = sht20ReadHumi_modbusRTU(id);
  Serial.printf("Info: wtr10e[0x09] temperature4 = %.1f\r\n",temp4);
  vTaskDelay(500);
  Serial.printf("Info: wtr10e[0x09] humidity4 = %.1f\r\n",humi4);
  vTaskDelay(500);

  Blynk.virtualWrite(V38, temp4);
  Blynk.virtualWrite(V39, humi4);    
 
  if(switchStatus4)
  {
    // auto
    if(humi4 <= thsensorLimit4)
    {
        relayControl_modbusRTU(10,1,1);
        Blynk.virtualWrite(V8, 1);                
        Blynk.setProperty(Widget_LED_Valve4, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve4, "label", "เปิดวาล์ว");
        LedBlynkValve4.on(); 
    }  
    else
    {
         relayControl_modbusRTU(10,1,0);
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
         relayControl_modbusRTU(10,1,1);       
        Blynk.setProperty(Widget_LED_Valve4, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve4, "label", "เปิดวาล์ว");
        LedBlynkValve4.on(); 
    }
    else
    {
         relayControl_modbusRTU(10,1,0);
        Blynk.setProperty(Widget_LED_Valve4, "label", "ปิดวาล์ว");                       
        LedBlynkValve4.off();
    }
    // manaul
  }
}
///////////////////WTR10-E Temp&Humi Node4/////////////////////////

///////////////Update switchStatus5 on Temp&Humi value5//////////////////
BLYNK_WRITE(V59)
{   
  switchStatus5 = param.asInt(); // Get value as integer
}
///////////////Update switchStatus5 on Temp&Humi value5//////////////////

////////////////////Update Temp&Humi setting////////////////////////////
BLYNK_WRITE(V60)
{   
  thsensorLimit5 = param.asInt(); // Get value as integer
}
////////////////////Update Temp&Humi setting////////////////////////////

/////////////////////Update manualSwitch///////////////////////////
BLYNK_WRITE(V10)
{
  manualSwitch5 = param.asInt();
}
/////////////////////Update manualSwitch///////////////////////////

///////////////////WTR10-E Temp&Humi Node5/////////////////////////
void wtr10e_node5()
{
int id = 11;
  float temp5 = sht20ReadTemp_modbusRTU(id);
  float humi5 = sht20ReadHumi_modbusRTU(id);
  Serial.printf("Info: wtr10e[0x11] temperature5 = %.1f\r\n",temp5);
  vTaskDelay(500);
  Serial.printf("Info: wtr10e[0x11] humidity5 = %.1f\r\n",humi5);
  vTaskDelay(500);

  Blynk.virtualWrite(V40, temp5);
  Blynk.virtualWrite(V41, humi5);    
 
  if(switchStatus5)
  {
    // auto
    if(humi5 <= thsensorLimit5)
    {
        relayControl_modbusRTU(12,1,1);
        Blynk.virtualWrite(V10, 1);                
        Blynk.setProperty(Widget_LED_Valve5, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve5, "label", "เปิดวาล์ว");
        LedBlynkValve5.on(); 
    }  
    else
    {
         relayControl_modbusRTU(12,1,0);
        Blynk.virtualWrite(V10, 0);
        Blynk.virtualWrite(Widget_LED_Valve5, 0);
        Blynk.setProperty(Widget_LED_Valve5, "label", "ปิดวาล์ว");                       
        LedBlynkValve5.off();  
    }
  }
  else
  {
    if(manualSwitch5)
    {
         relayControl_modbusRTU(12,1,1);       
        Blynk.setProperty(Widget_LED_Valve5, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve5, "label", "เปิดวาล์ว");
        LedBlynkValve5.on(); 
    }
    else
    {
         relayControl_modbusRTU(12,1,0);
        Blynk.setProperty(Widget_LED_Valve5, "label", "ปิดวาล์ว");                       
        LedBlynkValve5.off();
    }
    // manaul
  }
}
///////////////////WTR10-E Temp&Humi Node5/////////////////////////

///////////////Update switchStatus6 on Temp&Humi value6//////////////////
BLYNK_WRITE(V61)
{   
  switchStatus6 = param.asInt(); // Get value as integer
}
///////////////Update switchStatus6 on Temp&Humi value6//////////////////

////////////////////Update Temp&Humi setting////////////////////////////
BLYNK_WRITE(V62)
{   
  thsensorLimit6 = param.asInt(); // Get value as integer
}
////////////////////Update Temp&Humi setting////////////////////////////

/////////////////////Update manualSwitch///////////////////////////
BLYNK_WRITE(V12)
{
  manualSwitch6 = param.asInt();
}
/////////////////////Update manualSwitch///////////////////////////

///////////////////WTR10-E Temp&Humi Node6/////////////////////////
void wtr10e_node6()
{
int id = 13;
  float temp6 = sht20ReadTemp_modbusRTU(id);
  float humi6 = sht20ReadHumi_modbusRTU(id);
  Serial.printf("Info: wtr10e[0x13] temperature6 = %.1f\r\n",temp6);
  vTaskDelay(500);
  Serial.printf("Info: wtr10e[0x13] humidity6 = %.1f\r\n",humi6);
  vTaskDelay(500);

  Blynk.virtualWrite(V42, temp6);
  Blynk.virtualWrite(V43, humi6);    
 
  if(switchStatus6)
  {
    // auto
    if(humi6 <= thsensorLimit6)
    {
        relayControl_modbusRTU(14,1,1);
        Blynk.virtualWrite(V12, 1);                
        Blynk.setProperty(Widget_LED_Valve6, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve6, "label", "เปิดวาล์ว");
        LedBlynkValve6.on(); 
    }  
    else
    {
         relayControl_modbusRTU(14,1,0);
        Blynk.virtualWrite(V12, 0);
        Blynk.virtualWrite(Widget_LED_Valve6, 0);
        Blynk.setProperty(Widget_LED_Valve6, "label", "ปิดวาล์ว");                       
        LedBlynkValve6.off();  
    }
  }
  else
  {
    if(manualSwitch6)
    {
         relayControl_modbusRTU(14,1,1);       
        Blynk.setProperty(Widget_LED_Valve6, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve6, "label", "เปิดวาล์ว");
        LedBlynkValve6.on(); 
    }
    else
    {
         relayControl_modbusRTU(14,1,0);
        Blynk.setProperty(Widget_LED_Valve6, "label", "ปิดวาล์ว");                       
        LedBlynkValve6.off();
    }
    // manaul
  }
}
///////////////////WTR10-E Temp&Humi Node6/////////////////////////

//*****Loop Function*****// 
void loop() {    
  display.display();   
  
  if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();
}
//*****Loop Function*****// 
