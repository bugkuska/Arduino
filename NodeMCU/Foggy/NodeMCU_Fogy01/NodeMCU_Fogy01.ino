#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <SimpleTimer.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
BlynkTimer timer;
int blynkIsDownCount=0;

//RTC Widget
#include <TimeLib.h>
#include <WidgetRTC.h>
WidgetRTC rtc;

//LCD
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

//DHT11-01
#include <DHT.h>
#define DHTPIN1 D7
#define DHTTYPE1 DHT11
DHT dht1(DHTPIN1, DHTTYPE1);

//Valve1
//V11 Auto&Manual valve1
//V12 Slider Valve1
//Manual & Auto Switch 1
//V0 Valve1
#define Widget_LED_Valve1 V1        //ไฟสถานะปุ่ม 
#define Widget_Btn_Valve1 V2         //ปุ่ม เปิด-ปิด Valve1
WidgetLED LedBlynkValve1(Widget_LED_Valve1);
//Slider for set Light limit
bool switchStatus1 = 0; // 0 = manual,1=auto
int WaterLevelLimit1 = 0;
bool manualSwitch1 = 0;

//Cooling Fan
//V13 Auto&Manual Cfan
//V14 Slider Cfan
 #define Widget_LED_cfan V3        //ไฟสถานะปุ่ม Valve2
#define Widget_Btn_cfan V4         //ปุ่ม เปิด-ปิด Valve2
WidgetLED LedBlynkcfan(Widget_LED_cfan);
//Slider for set temperature limit
bool switchStatus2 = 0; // 0 = manual,1=auto
int HumiLevellimit = 0;
bool manualSwitch2 = 0;

//V7 Temperature1
//V8 Humidity1
//V9 currentTime
//V10 currentDate
  
//Define variable and pin connected to MCU
#define Relay1_Valve1         D0   //Relay1_Valve1
#define Relay2_Cfan           D4   //Cooling Fan start
//D5; //ว่าง
//D6;  //ว่าง

//define your default values here, if there are different values in config.json, they are overwritten.
//char mqtt_server[40];
//char mqtt_port[6] = "8442";
char blynk_token[34] = "";

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup()
{
  // put your setup code here, to run once:
  
  Serial.begin(9600);
  Serial.println();
  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
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

          //strcpy(mqtt_server, json["mqtt_server"]);
          //strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(blynk_token, json["blynk_token"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
      WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
  
  //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    wifiManager.addParameter(&custom_blynk_token);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
//if (WiFi.SSID()!="") wifiManager.setConfigPortalTimeout(60);
 
  if (!wifiManager.autoConnect("foggy", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(100);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
  } 
  if ((WiFi.status()!=WL_CONNECTED) )
  {
      Serial.println("failed to connect");
  } 
  else
  {
    //if you get here you have connected to the WiFi
    Serial.println("connected........:)");
    Serial.print("local ip: ");
    Serial.println(WiFi.localIP());
  }
  
  //read updated parameters
  //strcpy(mqtt_server, custom_mqtt_server.getValue());
  //strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(blynk_token, custom_blynk_token.getValue());
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    //json["mqtt_server"] = mqtt_server;
    //json["mqtt_port"] = mqtt_port;
    json["blynk_token"] = blynk_token;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  
  // Setup Pin Mode
  //D0 N/A
  //D1 SDA
  //D2 SDL
  pinMode(Relay1_Valve1,OUTPUT);         // NODEMCU PIN D3
  pinMode(Relay2_Cfan,OUTPUT);           // NODEMCU PIN D4
  // NODEMCU PIN D5
  // NODEMCU PIN D6
  // NODEMCU PIN D7 DHT11
  
  // Set Defult Relay Status
  //D0 N/A
  //D1 SDA
  //D2 SDL
  digitalWrite(Relay1_Valve1,HIGH);         // NODEMCU PIN D3
  digitalWrite(Relay2_Cfan,HIGH);           // NODEMCU PIN D4
  // NODEMCU PIN D5
  // NODEMCU PIN D6
  // NODEMCU PIN D7 DHT11
  
  //Start read DHT11-01
  dht1.begin();  //เริ่มอ่านข้อมูล DHT Sensor
  
  //Connect to Blynk Server
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);  
  Blynk.config(blynk_token);
  timer.setInterval(5000L, DHT11SensorData); //Send sensor data to display on app blynk
  timer.setInterval(5000L, WaterLevelData);
  timer.setInterval(10000L, ClockDisplay);  
  timer.setInterval(30000L, Reconnecting);  //Function reconnect
}
  

// update switchStatus1 on WaterLevel
BLYNK_WRITE(V11)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}

// update WaterLevel setting
BLYNK_WRITE(V12)
{   
  WaterLevelLimit1 = param.asInt(); // Get value as integer
}

// update manualSwitch
BLYNK_WRITE(V2)
{
  manualSwitch1 = param.asInt();
}

//Water Level Sensor
void WaterLevelData()
{
  int waterlevel_percentage;
  waterlevel_percentage =   analogRead(A0);
  waterlevel_percentage = map(waterlevel_percentage, 0, 1023, 100, 0); //0, 1023, 0, 100);
  Blynk.virtualWrite(V0,(waterlevel_percentage)); 
  Serial.print("Water Level Percentage = ");
  Serial.print(waterlevel_percentage);
  Serial.print("%\n\n");
  delay(1000);
  
   //LCD Display Water Level data    
      lcd.begin();
      lcd.backlight();                                   
      lcd.setCursor(0, 0); 
      lcd.print("WaterLevel="); 
      lcd.print(waterlevel_percentage);
      lcd.print("%");
      
if(switchStatus1)
  {
    // auto
    if(waterlevel_percentage < WaterLevelLimit1)
    {
        digitalWrite(Relay1_Valve1, LOW);  
        Blynk.virtualWrite(V2, 1);
                
        Blynk.setProperty(Widget_LED_Valve1, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์ว");
        LedBlynkValve1.on(); 
    }  
    else
    {
        digitalWrite(Relay1_Valve1, HIGH);
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
        digitalWrite(Relay1_Valve1, LOW);        
        Blynk.setProperty(Widget_LED_Valve1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์ว");
        LedBlynkValve1.on(); 
    }
    else
    {
        digitalWrite(Relay1_Valve1, HIGH);
        Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์ว");                       
        LedBlynkValve1.off();
    }
    // manaul
  }
}

// update switchStatus2 on Humidity
BLYNK_WRITE(V13)
{   
  switchStatus2 = param.asInt(); // Get value as integer
}

// update Humidity setting
BLYNK_WRITE(V14)
{   
  HumiLevellimit = param.asInt(); // Get value as integer
}

// update manualSwitch2
BLYNK_WRITE(V4)
{
  manualSwitch2 = param.asInt();
}

//DHT11-01
void DHT11SensorData(){
 float h1 = dht1.readHumidity();
 float t1 = dht1.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
 if (isnan(h1) || isnan(t1)){ 
 Serial.println("Read from DHT Sensor 1");
  return;
  }
  Blynk.virtualWrite(V7, t1);
  Blynk.virtualWrite(V8, h1);
 
    //LCD Display Temperature and Humidity data                       
      lcd.setCursor(0, 1); 
      lcd.print("T="); 
      lcd.print(t1);
      lcd.print("C");
      
      lcd.setCursor(9, 1);
      lcd.print("H="); 
      lcd.print(h1);
      lcd.print("%");
         
  if(switchStatus2)
  {
    // auto
    if(h1 < HumiLevellimit)
    {
        digitalWrite(Relay2_Cfan, LOW);  
        Blynk.virtualWrite(V4, 1);
        Blynk.setProperty(Widget_LED_cfan, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_cfan, "label", "เปิดระบบพ่นหมอก");
        LedBlynkcfan.on(); 

    }  
    else
    {
        digitalWrite(Relay2_Cfan, HIGH);
        Blynk.virtualWrite(V4, 0);
        Blynk.virtualWrite(Widget_LED_cfan, 0);
        Blynk.setProperty(Widget_LED_cfan, "label", "ปิดระบบพ่นหมอก");                       
        LedBlynkcfan.off();  
    }
  }
  else
  {
    if(manualSwitch2)
    {
        digitalWrite(Relay2_Cfan, LOW);
        Blynk.setProperty(Widget_LED_cfan, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_cfan, "label", "เปิดระบบพ่นหมอก");
        LedBlynkcfan.on(); 
    }
    else
    {
        digitalWrite(Relay2_Cfan, HIGH);
        Blynk.setProperty(Widget_LED_cfan, "label", "ปิดระบบพ่นหมอก");                       
        LedBlynkcfan.off();
    }
    // manaul
  }
}

//Blynk Connected
BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    Blynk.syncAll();

    // Synchronize time on connection
    rtc.begin();

 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
 }
}

void Reconnecting()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    Blynk.syncAll();
    // Synchronize time on connection
    rtc.begin();
  }
  
  if (blynkIsDownCount >= 5){
    ESP.reset();
  }
}

//Display Current Date/Time
void ClockDisplay()
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  Blynk.virtualWrite(V9, currentTime);
  Blynk.virtualWrite(V10, currentDate);
}

void loop()
{
   
  
  if (Blynk.connected())
  {
    Blynk.run();
    Blynk.syncAll();
  }
   timer.run();  
}
