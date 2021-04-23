//*****Define MCU Pin*******//
//ว่าง                 D0
//SCL                 D1
//SDA                 D2
//ว่าง                 D3
//Relay1_btn1         D5
//Relay2_btn2         D6
//Relay3_ledblynk     D4
//DHT11               D7
//Soil Moisture       A0
//*****Define MCU Pin*******//

//*****Define Blynk Virtual Pin*****//
//V0  ค่าความชื้นในดิน
//V1  ไฟสถานะปุ่ม 1
//V2  ปุ่ม เปิด-ปิด 1
//V3  ไฟสถานะปุ่ม 2
//V4  ปุ่ม เปิด-ปิด 2
//V9  Temperature
//V10 Humidity
//V11 Auto&Manual btn1
//V12 Slider btn1
//V13 Auto&Manual btn2
//V14 Slider btn2
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

//Slider for set Soil Moisture limit
//ตรวจสอบเงื่อนไขสำหรับเปิดปิดปุ่มที่ 1 ในโหมดการทำงานอัตโนมัติ
bool switchStatus1 = 0; // 0 = manual,1=auto
int SoilsensorLimit1 = 0;
bool manualSwitch1 = 0;

//DHT11
#include <DHT.h>
#define DHTPIN D7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
//Slider for set temperature limit
//ตรวจสอบเงื่อนไขสำหรับเปิดปิดปุ่มที่ 2 ในโหมดการทำงานอัตโนมัติ
bool switchStatus2 = 0; // 0 = manual,1=auto
int templimit = 0;
bool manualSwitch2 = 0;

//BTN1
//V11 Auto&Manual btn1
//V12 Slider btn1
#define Relay1_btn1   D5
#define Widget_LED_btn1 V1          //ไฟสถานะปุ่ม 1
#define Widget_Btn_btn1 V2          //ปุ่ม เปิด-ปิด 1
WidgetLED LedBlynkbtn1(Widget_LED_btn1);

//BTN2
//V13 Auto&Manual btn2
//V14 Slider btn2
#define Relay2_btn2   D6
#define Widget_LED_btn2 V3          //ไฟสถานะปุ่ม 2
#define Widget_Btn_btn2 V4          //ปุ่ม เปิด-ปิด 2
WidgetLED LedBlynkbtn2(Widget_LED_btn2);

//BTN3
#define Relay3_ledblynk   D4

//V9  Humidity
//V10 Temperature

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}                       

//Setup Function
  void setup() {
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
 
  if (!wifiManager.autoConnect("NodeMCU_SS01", "password")) {
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
  pinMode(Relay1_btn1,OUTPUT);                // NODEMCU PIN D5 
  pinMode(Relay2_btn2,OUTPUT);                // NODEMCU PIN D6
  pinMode(Relay3_ledblynk,OUTPUT);            // NODEMCU PIN D4         
  
  // Set Defult Relay Status
  digitalWrite(Relay1_btn1,LOW);              // NODEMCU PIN D5
  digitalWrite(Relay2_btn2,LOW);              // NODEMCU PIN D6
  digitalWrite(Relay3_ledblynk,LOW);          // NODEMCU PIN D4
   
   //Start read DHT11
  dht.begin();  //เริ่มอ่านข้อมูล DHT Sensor
  
 //Connect to Blynk Server
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  Blynk.config(blynk_token);
  timer.setInterval(30000L, reconnecting);  
  timer.setInterval(5000L, getSoilSensorData);
  timer.setInterval(5000L, getDHTSensorData);
}

// Update switchStatus1 on Soil Moisture
BLYNK_WRITE(V11)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}

// Update Soil setting
BLYNK_WRITE(V12)
{   
  SoilsensorLimit1 = param.asInt(); // Get value as integer
}

// Update manualSwitch
BLYNK_WRITE(V2)
{
  manualSwitch1 = param.asInt();
}

void getSoilSensorData()
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
        digitalWrite(Relay1_btn1, LOW);  
        Blynk.virtualWrite(V2, 1);
                
        Blynk.setProperty(Widget_LED_btn1, "color", "#C70039");
        Blynk.setProperty(Widget_LED_btn1, "label", "เปิดปุ่มที่ 1");
        LedBlynkbtn1.on(); 
    }  
    else
    {
        digitalWrite(Relay1_btn1, HIGH);
        Blynk.virtualWrite(V2, 0);
        Blynk.virtualWrite(Widget_LED_btn1, 0);
        Blynk.setProperty(Widget_LED_btn1, "label", "ปิดปุ่มที่ 1");                       
        LedBlynkbtn1.off();  
    }
  }
  else
  {
    if(manualSwitch1)
    {
        digitalWrite(Relay1_btn1, LOW);        
        Blynk.setProperty(Widget_LED_btn1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_btn1, "label", "เปิดปุ่มที่ 1");
        LedBlynkbtn1.on(); 
    }
    else
    {
        digitalWrite(Relay1_btn1, HIGH);
        Blynk.setProperty(Widget_LED_btn1, "label", "ปิดปุ่มที่ 1");                       
        LedBlynkbtn1.off();
    }
    // manaul
  }
}

// Update switchStatus2 on Temperature
BLYNK_WRITE(V13)
{   
  switchStatus2 = param.asInt(); // Get value as integer
}

// Update temperture setting
BLYNK_WRITE(V14)
{   
  templimit = param.asInt(); // Get value as integer
}

// Update manualSwitch2
BLYNK_WRITE(V4)
{
  manualSwitch2 = param.asInt();
}

//DHT11
void getDHTSensorData(){
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)){ 
 Serial.println("Read from DHT Sensor 1");
  return;
  }
  Blynk.virtualWrite(V9, t);
  Blynk.virtualWrite(V10, h);
  
  if(switchStatus2)
  {
    // auto
    if(t > templimit)
    {
        digitalWrite(Relay2_btn2, LOW);  
        Blynk.virtualWrite(V4, 1);
        Blynk.setProperty(Widget_LED_btn2, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_btn2, "label", "เปิดปุ่มที่ 2");
        LedBlynkbtn2.on(); 

    }  
    else
    {
        digitalWrite(Relay2_btn2, HIGH);
        Blynk.virtualWrite(V4, 0);
        Blynk.virtualWrite(Widget_LED_btn2, 0);
        Blynk.setProperty(Widget_LED_btn2, "label", "ปิดปุ่มที่ 2");                       
        LedBlynkbtn2.off();  
    }
  }
  else
  {
    if(manualSwitch2)
    {
        digitalWrite(Relay2_btn2, LOW);
        Blynk.setProperty(Widget_LED_btn2, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_btn2, "label", "เปิดปุ่มที่ 2");
        LedBlynkbtn2.on(); 
    }
    else
    {
        digitalWrite(Relay2_btn2, HIGH);
        Blynk.setProperty(Widget_LED_btn2, "label", "ปิดปุ่มที่ 2");                       
        LedBlynkbtn2.off();
    }
    // manaul
  }
}

//Blynk connected
BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    Blynk.syncAll();

 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(Relay3_ledblynk,LOW);
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
  }
  if (blynkIsDownCount >= 5){
    ESP.reset();
  }
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
