//อ้างจาก https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/
#include <FS.h>                                   //this needs to be first, or it all crashes and burns...
#include <SPIFFS.h>
#include <WiFi.h>                                 //https://github.com/esp8266/Arduino
#include <WiFiClient.h>
//needed for library
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>                          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>                          //Ver 5.13.4 https://github.com/bblanchon/ArduinoJson

#include <BlynkSimpleEsp32.h>                     //  Blynk_Release_v0.6.1 
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
int blynkIsDownCount = 0;
BlynkTimer timer;

char server[] = "blynk-cloud.com";
int port = 8442;
char blynk_token[34] = "";//ใส่ Blynk_token ของเราที่ Blynk ส่งมาทาง Email ตอน Create Project ใหม่

//10A 4channel Relay extend 
#define pump            23            // Relay connect to pump 
#define ledbb           15            //Check blynk connected 
#define ledfan          25            //Cooling Fan

//10A Relay on board esp32all
#define Relay1_Valve1  27             //valve1
#define Relay2_Valve2  14             //valve2
#define Relay3_Valve3  12             //valve3
#define Relay4_Valve4  13             //valve4 

//Define pin for analog input
#define INPUT_1_A1 36
#define INPUT_2_A2 39
#define INPUT_3_A3 34
#define INPUT_4_A4 33

//RTC Widget
#include <TimeLib.h>
#include <WidgetRTC.h>
WidgetRTC rtc;

//DHT11
#include "DHT.h"
#define DHTTYPE DHT11   // DHT 11
#define DHTPin 16
DHT dht(DHTPin, DHTTYPE);

/////////////////////////////////////////Define Virtual Pin////////////////////////////////
//V2 ไฟสถานะปุ่ม Valve1
//V3 ปุ่ม เปิด-ปิด Valve1
//V6 ไฟสถานะปุ่ม Valve2
//V7 ปุ่ม เปิด-ปิด Valve2
//V10 ไฟสถานะปุ่ม Valve3
//V11 ปุ่ม เปิด-ปิด Valve3
//V14 ไฟสถานะปุ่ม Valve4
//V15 ปุ่ม เปิด-ปิด Valve4
//V18 ไฟสถานะปุ่ม pump
//V19 ปุ่ม เปิด-ปิด pump
//V20 Current Time
//V21 Current Date
//V22 ไฟสถานะปุ่ม พัดลมระบายอากาศ
//V23 ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
//V25 Soil 1
//V26 Soil 2
//V27 Soil 3
//V28 Soil 4
//V30 Humidity
//V31 Temperature
//V40 Time
//V41 Date
//////////////////////////////////////////Define Virtual Pin////////////////////////////////

//Valve1
#define Widget_LED_Valve1 V2              //ไฟสถานะปุ่ม Valve1
#define Widget_Btn_Valve1 V3              //ปุ่ม เปิด-ปิด Valve1
WidgetLED LedBlynkValve1(Widget_LED_Valve1);


//Valve2
#define Widget_LED_Valve2 V6              //ไฟสถานะปุ่ม Valve2
#define Widget_Btn_Valve2 V7              //ปุ่ม เปิด-ปิด Valve2
WidgetLED LedBlynkValve2(Widget_LED_Valve2);


//Valve3
#define Widget_LED_Valve3 V10             //ไฟสถานะปุ่ม Valve3
#define Widget_Btn_Valve3 V11             //ปุ่ม เปิด-ปิด Valve3
WidgetLED LedBlynkValve3(Widget_LED_Valve3);


//Valve4
#define Widget_LED_Valve4 V14             //ไฟสถานะปุ่ม Valve4
#define Widget_Btn_Valve4 V15             //ปุ่ม เปิด-ปิด Valve4
WidgetLED LedBlynkValve4(Widget_LED_Valve4);

//Pump
#define Widget_LED_Pump V18         //ไฟสถานะปุ่ม pump
#define Widget_Btn_Pump V19         //ปุ่ม เปิด-ปิด pump
WidgetLED LedBlynkPump(Widget_LED_Pump);

//Cooling Fan
#define Widget_LED_Fan V22        //ไฟสถานะปุ่ม พัดลมระบายอากาศ
#define Widget_Btn_Fan V23        //ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
WidgetLED LedBlynkFan(Widget_LED_Fan);

bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

//Setup
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);  
  // Setup Pin Mode
  pinMode(pump, OUTPUT);              // Pin 23 relay 10A in1 connect to Manatic + overload
  pinMode(ledbb,OUTPUT);              // Pin 27 relay 10A in2 connect to Green PoltLamp check blynk connected
  pinMode(ledfan,OUTPUT);             // Pin 26 relay 10A in3 connect to Red PoltLamp start cooling fan 
  pinMode(Relay1_Valve1,OUTPUT);      // Pin 19 relay 10A in1 valve1
  pinMode(Relay2_Valve2,OUTPUT);      // Pin 18 relay 10A in2 valve2
  pinMode(Relay3_Valve3,OUTPUT);      // Pin 5  relay 10A in3 valve3
  pinMode(Relay4_Valve4,OUTPUT);      // Pin 25 relay 10A in4 valve4
  
  // Set Defult Relay Status
  digitalWrite(pump, LOW);           // Pin 23 relay 10A in1 connect to Manatic + overload
  digitalWrite(ledbb,LOW);           // Pin 27 relay 10A in2 connect to Green PoltLamp check blynk connected
  digitalWrite(ledfan,LOW);          // Pin 26 relay 10A in3 connect to Red PoltLamp start cooling fan  
  digitalWrite(Relay1_Valve1,LOW);   // Pin 19 relay 10A in1 valve1
  digitalWrite(Relay2_Valve2,LOW);   // Pin 18 relay 10A in2 valve2
  digitalWrite(Relay3_Valve3,LOW);   // Pin 5  relay 10A in3 valve3
  digitalWrite(Relay4_Valve4,LOW);   // Pin 25 relay 10A in4 valve4

  //Begin read Humidity and Temperature ==> SHCT3
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
  

  //**************************        AP AUTO CONNECT   ********************************************//
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_blynk_token);

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

  if (!wifiManager.autoConnect("IoTSmartfarming")) {
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
   
  // Blynk.config(blynk_token);////เริ่มการเชื่อมต่อ Blynk Server แบบปกติ
  Blynk.config(blynk_token, server, port);
  
  timer.setInterval(10000L,dhtSensorData);
  timer.setInterval(10000L, getSoilMoisterData1);
  timer.setInterval(10000L, getSoilMoisterData2);
  timer.setInterval(10000L, getSoilMoisterData3);
  timer.setInterval(10000L, getSoilMoisterData4);  
  timer.setInterval(10000L, clockDisplay);
  timer.setInterval(10000L, reconnectblynk);  //Function reconnect  

}

//blynk conneted
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

//Loop 
void loop() {     
  if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();//ให้เวลาของ Blynk ทำงาน
}

//Reconnect to blynk
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

//Display Current Date/Time
void clockDisplay()
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  // Send time to the App
  Blynk.virtualWrite(V40, currentTime);
  // Send date to the App
  Blynk.virtualWrite(V41, currentDate);
}

//SoilMoisture Sensor1
void getSoilMoisterData1(){
  float moisture_percentage1;
  int sensor_analog1;
  sensor_analog1 = analogRead(INPUT_1_A1);
  //Serial.print("Law Soil data:");
  //Serial.println(sensor_analog1);
  moisture_percentage1 = ( 100 - ( (sensor_analog1/4095.00) * 100 ) );
  Blynk.virtualWrite(V25,(moisture_percentage1));
  //Serial.print("Moisture Percentage1 = ");
  //Serial.print(moisture_percentage1);
  //Serial.print("%\n\n");
  //delay(1000);
}

//SoilMoisture Sensor2
void getSoilMoisterData2(){
  float moisture_percentage2;
  int sensor_analog2;
  sensor_analog2 = analogRead(INPUT_2_A2);
  //Serial.print("Law Soil data:");
  //Serial.println(sensor_analog2);
  moisture_percentage2 = ( 100 - ( (sensor_analog2/4095.00) * 100 ) );
  Blynk.virtualWrite(V26,(moisture_percentage2));
  //Serial.print("Moisture Percentage2 = ");
  //Serial.print(moisture_percentage2);
  //Serial.print("%\n\n");
  //delay(1000);   
}

//SoilMoisture Sensor3
void getSoilMoisterData3(){
  float moisture_percentage3;
  int sensor_analog3;
  sensor_analog3 = analogRead(INPUT_3_A3);
 // Serial.print("Law Soil data:");
 // Serial.println(sensor_analog3);
  moisture_percentage3 = ( 100 - ( (sensor_analog3/4095.00) * 100 ) );
  Blynk.virtualWrite(V27,(moisture_percentage3));
 // Serial.print("Moisture Percentage3 = ");
  //Serial.print(moisture_percentage3);
 // Serial.print("%\n\n");
 // delay(1000);   
}


//SoilMoisture Sensor4
void getSoilMoisterData4(){
  float moisture_percentage4;
  int sensor_analog4;
  sensor_analog4 = analogRead(INPUT_4_A4);
  //Serial.print("Law Soil data:");
  //Serial.println(sensor_analog4);
  moisture_percentage4 = ( 100 - ( (sensor_analog4/4095.00) * 100 ) );
  Blynk.virtualWrite(V28,(moisture_percentage4));
  //Serial.print("Moisture Percentage4 = ");
  //Serial.print(moisture_percentage4);
  //Serial.print("%\n\n");
  //delay(1000);   
}

//****BUTTON ON/OFF Valve1****
BLYNK_WRITE(Widget_Btn_Valve1){
int valueValve1 = param.asInt();
  if(valueValve1 == 1){
    digitalWrite(Relay1_Valve1, HIGH);
    Blynk.setProperty(Widget_LED_Valve1, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์วน้ำ");
    LedBlynkValve1.on();
    digitalWrite(pump, HIGH);
    Blynk.virtualWrite(Widget_Btn_Pump,1);
    Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
    LedBlynkPump.on(); 
    }
    else{              
    digitalWrite(Relay1_Valve1, LOW);
    Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์วน้ำเรียบร้อยแล้ว");
    LedBlynkValve1.off();     

    digitalWrite(pump, LOW);      
    Blynk.virtualWrite(Widget_Btn_Pump,0);                
    Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
    LedBlynkPump.off();           
    }
}
     
//****BUTTON ON/OFF Valve2****
BLYNK_WRITE(Widget_Btn_Valve2){
int valueValve2 = param.asInt();
  if(valueValve2 == 1){
    digitalWrite(Relay2_Valve2, HIGH);
    Blynk.setProperty(Widget_LED_Valve2, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Valve2, "label", "เปิดวาล์ว 2");
    LedBlynkValve2.on();

    digitalWrite(pump, HIGH);
    Blynk.virtualWrite(Widget_Btn_Pump,1);
    Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
    LedBlynkPump.on(); 
    }
    else{              
    digitalWrite(Relay2_Valve2, LOW);
    Blynk.setProperty(Widget_LED_Valve2, "label", "ปิดวาล์ว 2");
    LedBlynkValve2.off();     

    digitalWrite(pump, LOW);      
    Blynk.virtualWrite(Widget_Btn_Pump,0);                
    Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
    LedBlynkPump.off();           
    }
}
    
//****BUTTON ON/OFF Valve3****
BLYNK_WRITE(Widget_Btn_Valve3){
int valueValve3 = param.asInt();
  if(valueValve3 == 1){       
    digitalWrite(Relay3_Valve3, HIGH);
    Blynk.setProperty(Widget_LED_Valve3, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Valve3, "label", "เปิดวาล์ว 3");
    LedBlynkValve3.on();

    digitalWrite(pump, HIGH);
    Blynk.virtualWrite(Widget_Btn_Pump,1);
    Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
    LedBlynkPump.on(); 
    }
    else{              
    digitalWrite(Relay3_Valve3, LOW);
    Blynk.setProperty(Widget_LED_Valve3, "label", "ปิดวาล์ว 3");
    LedBlynkValve3.off();     

    digitalWrite(pump, LOW);      
    Blynk.virtualWrite(Widget_Btn_Pump,0);                
    Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
    LedBlynkPump.off();           
    }
}
      
//****BUTTON ON/OFF Valve4****
BLYNK_WRITE(Widget_Btn_Valve4){
int valueValve4 = param.asInt();
  if(valueValve4 == 1){      
    digitalWrite(Relay4_Valve4, HIGH);
    Blynk.setProperty(Widget_LED_Valve4, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Valve4, "label", "เปิดวาล์ว 3");
    LedBlynkValve4.on();

    digitalWrite(pump, HIGH);
    Blynk.virtualWrite(Widget_Btn_Pump,1);
    Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
    LedBlynkPump.on(); 
    }
    else{              
    digitalWrite(Relay4_Valve4, LOW);
    Blynk.setProperty(Widget_LED_Valve4, "label", "ปิดวาล์ว 4");
    LedBlynkValve4.off();     

    digitalWrite(pump, LOW);      
    Blynk.virtualWrite(Widget_Btn_Pump,0);                
    Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
    LedBlynkPump.off();           
    }
}

//****BUTTON ON/OFF Cooling FAN****
BLYNK_WRITE(Widget_Btn_Fan){
int valueFan = param.asInt();
  if(valueFan == 1){
    digitalWrite(ledfan, HIGH);
    Blynk.setProperty(Widget_LED_Fan, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Fan, "label", "เปิดพัดลมระบายอากาศ");
    LedBlynkFan.on();
    }
    else{              
    digitalWrite(ledfan, LOW);
    Blynk.virtualWrite(Widget_Btn_Fan, 0);
    Blynk.setProperty(Widget_LED_Fan, "label", "ปิดพัดลมระบายอากาศ");
    LedBlynkFan.off();                       
    }
}

//DHT11
void dhtSensorData(){
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)){ 
 Serial.println("Read from DHT Sensor");
  return;
  }    
Blynk.virtualWrite(V30, h);
Blynk.virtualWrite(V31, t);

    //******AUTO Cooling FAN*******
  if (t >= 33){       
    digitalWrite(ledfan, HIGH);
    Blynk.virtualWrite(Widget_Btn_Fan, 1);       
    Blynk.setProperty(Widget_LED_Fan, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Fan, "label", "พัดลมกำลังทำงาน");
    LedBlynkFan.on();
    }   

  if (t < 33){
    digitalWrite(ledfan, LOW);        
    Blynk.virtualWrite(Widget_Btn_Fan, 0);
    Blynk.setProperty(Widget_LED_Fan, "label", "ปิดพัดลมแล้ว");                        
    LedBlynkFan.off();  
    }       
}
