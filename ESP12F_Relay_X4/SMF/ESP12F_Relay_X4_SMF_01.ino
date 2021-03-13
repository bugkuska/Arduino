//*****Define MCU Digital Pin*******//
//Soil Moisture     A0
//Relay1_Valve1     16
//Relay2_Valve2     14
//Relay3_Pump       12
//Relay4_Ledblynk   13
//DHT11             15
//ว่าง                5
//ว่าง                4
//ว่าง                0
//ว่าง                2
//*****Define MCU Digital Pin*******//

//*****Define Blynk Virtual Pin*****//
//V0  ค่าความชื้นในดิน
//V1  ไฟสถานะปุ่ม 1
//V2  ปุ่ม เปิด-ปิด Valve1
//V3  ไฟสถานะปุ่ม 2
//V4  ปุ่ม เปิด-ปิด Valve2
//V5  ไฟสถานะปุ่ม pump
//V6  ปุ่ม เปิด-ปิด pump
//V7  อุณหภูมิในอากาศ
//V8  ความชื้นในอากาศ
//V9  เวลา
//V10 วัน/เดือน/ปี
//V11 Auto&Manual valve1
//V12 Slider Valve1
//*****Define Blynk Virtual Pin*****//

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
char blynk_token[34] = "";

//RTC Widget
#include <TimeLib.h>
#include <WidgetRTC.h>
WidgetRTC rtc;

//DHT11
#include <DHT.h>
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//Valve1
//V11 Auto&Manual valve1
//V12 Slider Valve1
//Manual & Auto Switch 1
#define Relay1_Valve1   16
#define Widget_LED_Valve1 V1          //ไฟสถานะปุ่ม 1
#define Widget_Btn_Valve1 V2          //ปุ่ม เปิด-ปิด Valve1
WidgetLED LedBlynkValve1(Widget_LED_Valve1);

//Valve2
#define Relay2_Valve2   14
#define Widget_LED_Valve2 V3          //ไฟสถานะปุ่ม 2
#define Widget_Btn_Valve2 V4          //ปุ่ม เปิด-ปิด Valve2
WidgetLED LedBlynkValve2(Widget_LED_Valve2);

//Slider for set Light limit
bool switchStatus1 = 0; // 0 = manual,1=auto
int SoilsensorLimit1 = 0;
bool manualSwitch1 = 0;

//Pump
#define Relay3_Pump   12
#define Widget_LED_Pump V5          //ไฟสถานะปุ่ม pump
#define Widget_Btn_Pump V6          //ปุ่ม เปิด-ปิด pump
WidgetLED LedBlynkPump(Widget_LED_Pump);

#define Relay4_Ledblynk    13

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void setup()
{
  Serial.begin(115200);
  Serial.println();

  // Setup Pin Mode
  pinMode(Relay1_Valve1,OUTPUT);              // NODEMCU PIN gpio16 
  pinMode(Relay2_Valve2,OUTPUT);              // NODEMCU PIN gpio14
  pinMode(Relay3_Pump,OUTPUT);                // NODEMCU PIN gpio12   
  pinMode(Relay4_Ledblynk,OUTPUT);            // NODEMCU PIN GPIO13         
  
  // Set Defult Relay Status
  digitalWrite(Relay1_Valve1,LOW);            // NODEMCU PIN gpio16
  digitalWrite(Relay2_Valve2,LOW);            // NODEMCU PIN gpio14
  digitalWrite(Relay3_Pump,LOW);              // NODEMCU PIN gpio12
  digitalWrite(Relay4_Ledblynk,LOW);          // NODEMCU PIN gpio13
  
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
          strcpy(blynk_token, json["blynk_token"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
    WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

  //wifiManager.resetSettings();
  //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    wifiManager.addParameter(&custom_blynk_token);

  //reset settings - for testing
  //wifiManager.resetSettings();
    
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(120);
  
  if (!wifiManager.autoConnect("ESP12F_SMF01", "password")) {
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
  strcpy(blynk_token, custom_blynk_token.getValue());
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
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
   //Start read DHT11
  dht.begin();  //เริ่มอ่านข้อมูล DHT Sensor
  
  Blynk.config(blynk_token);
  timer.setInterval(30000L, reconnecting);  
  timer.setInterval(10000L, clockDisplay); 
  timer.setInterval(5000L, dhtSensorData);
  timer.setInterval(5000L, getsoilSensorData);
}

// Update switchStatus1 on Soil Moisture
BLYNK_WRITE(V11)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}

// Update Soil Moisture setting
BLYNK_WRITE(V12)
{   
  SoilsensorLimit1 = param.asInt(); // Get value as integer
}

// Update ManualSwitch
BLYNK_WRITE(V2)
{
  manualSwitch1 = param.asInt();
}


//Soil Moisture Sensor
void getsoilSensorData()
{
  float soil_percentage;
  int sensor_analog;
  sensor_analog = analogRead(A0);
  Serial.print("Law Soil data:");
  Serial.println(sensor_analog);
  soil_percentage = ( 100 - ( (sensor_analog/1024.00) * 100 ) );
  Blynk.virtualWrite(V0,(soil_percentage)); 
  Serial.print("Soil Percentage = ");
  Serial.print(soil_percentage);
  Serial.print("%\n\n");
  delay(1000);
    

if(switchStatus1)
  {
    // auto
    if(soil_percentage < SoilsensorLimit1)
    {
        digitalWrite(Relay1_Valve1, HIGH);  
        Blynk.virtualWrite(V2, 1);                
        Blynk.setProperty(Widget_LED_Valve1, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์ว");
        LedBlynkValve1.on(); 

        digitalWrite(Relay2_Valve2, HIGH);
        Blynk.virtualWrite(V4, 1);                
        Blynk.setProperty(Widget_LED_Valve2, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve2, "label", "เปิดวาล์ว");
        LedBlynkValve2.on(); 
        
        digitalWrite(Relay3_Pump, HIGH);
        Blynk.setProperty(Widget_LED_Pump, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on();
        
    }  
    else
    {
        digitalWrite(Relay1_Valve1, LOW);
        Blynk.virtualWrite(V2, 0);
        Blynk.virtualWrite(Widget_LED_Valve1, 0);
        Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์ว");                       
        LedBlynkValve1.off();  

        digitalWrite(Relay2_Valve2, LOW);
        Blynk.virtualWrite(V4, 0);                
        Blynk.setProperty(Widget_LED_Valve2, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve2, "label", "ปิดวาล์ว");
        LedBlynkValve2.off(); 
        
        digitalWrite(Relay3_Pump, LOW);
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำ");
        LedBlynkPump.off();
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

        digitalWrite(Relay3_Pump, HIGH);
        Blynk.setProperty(Widget_LED_Pump, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on();
    }
    else
    {
        digitalWrite(Relay1_Valve1, LOW);        
        Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์ว");                       
        LedBlynkValve1.off();

        digitalWrite(Relay3_Pump, LOW);
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำ");
        LedBlynkPump.off();
    }
    // manaul
  }
}

//****BUTTON ON/OFF Valve2****
 BLYNK_WRITE(Widget_Btn_Valve2){
      int valueValve2 = param.asInt();
      if(valueValve2 == 1){
        digitalWrite(Relay2_Valve2,HIGH);
        Blynk.setProperty(Widget_LED_Valve2, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve2, "label", "เปิดวาล์ว2");
        LedBlynkValve2.on();
        
      }
       else{              
        digitalWrite(Relay2_Valve2,LOW);
        Blynk.setProperty(Widget_LED_Valve2, "label", "ปิดวาล์ว2");
        LedBlynkValve2.off();          
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
  Blynk.virtualWrite(V7, t);
  Blynk.virtualWrite(V8, h);
}

//Display Current Date/Time
void clockDisplay()
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  Blynk.virtualWrite(V9, currentTime);
  Blynk.virtualWrite(V10, currentDate);
}

BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    Blynk.syncAll();

 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(Relay4_Ledblynk,HIGH);
 }
}

void reconnecting()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    Blynk.syncAll();
    digitalWrite(Relay4_Ledblynk,HIGH);
  }
}

void loop()
{
  Serial.println();
  if (blynkIsDownCount >=3){
    Serial.println("Reset.....");
    ESP.restart();
  }
  delay(1000);
 
  if (Blynk.connected())
  {
    Blynk.run();
    Blynk.syncAll();
    digitalWrite(Relay4_Ledblynk,HIGH);
  }
   timer.run();  
}
//loop end
