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

//LCD
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

//RTC Widget
#include <TimeLib.h>
#include <WidgetRTC.h>
WidgetRTC rtc;

//Light
//V11 Auto&Manual Light
//V12 Slider light
//Manual & Auto Switch 1
//V0 Light
#define Widget_LED_Light V1        //ไฟสถานะปุ่ม 
#define Widget_Btn_Light V2         //ปุ่ม เปิด-ปิด Light
WidgetLED LedBlynkLight(Widget_LED_Light);

//Cooling Fan
//V13 Auto&Manual Cfan
//V14 Slider Cfan
#define Widget_LED_cfan V3        //ไฟสถานะปุ่ม Valve2
#define Widget_Btn_cfan V4         //ปุ่ม เปิด-ปิด Valve2
WidgetLED LedBlynkcfan(Widget_LED_cfan);

//V7 Temperature1
//V8 Humidity1

//DHT11-01
#include <DHT.h>
#define DHTPIN1 D7
#define DHTTYPE1 DHT11
DHT dht1(DHTPIN1, DHTTYPE1);

int blynkIsDownCount=0;

//Slider for set Light limit
bool switchStatus1 = 0; // 0 = manual,1=auto
int LightLimit1 = 0;
bool manualSwitch1 = 0;

//Slider for set temperature limit
bool switchStatus2 = 0; // 0 = manual,1=auto
int templimit = 0;
bool manualSwitch2 = 0;

//Define variable and pin connected to MCU
const int ledbb             = D3; //ledbb
const int ledfan            = D4;
const int Relay1_Light     = D5;   //Relay1_Light
const int Relay2_Fan1      = D6;   //C-Fan1
            
//New
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

unsigned long lastMillis = 0;
//end new

void setup()
{
  // put your setup code here, to run once:
  
  Serial.begin(115200);
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

  //wifiManager.resetSettings();
  
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
  wifiManager.setTimeout(60);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
//if (WiFi.SSID()!="") wifiManager.setConfigPortalTimeout(60);
 
  if (!wifiManager.autoConnect("node_gg_assist", "password")) {
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
  pinMode(ledbb,OUTPUT);
  pinMode(ledfan,OUTPUT);
  pinMode(Relay1_Light,OUTPUT);      // NODEMCU PIN D4 connect to relay 10A in1
  pinMode(Relay2_Fan1,OUTPUT);       // NODEMCU PIN D5 connect to relay 10A in2
 
  
  // Set Defult Relay Status
  digitalWrite(ledbb,HIGH);
  digitalWrite(ledfan,HIGH);
  digitalWrite(Relay1_Light,LOW);    // NODEMCU PIN D4 connect to relay 10A in1
  digitalWrite(Relay2_Fan1,LOW);     // NODEMCU PIN D5 connect to relay 10A in2
  
  //Start read DHT11-01
  dht1.begin();  //เริ่มอ่านข้อมูล DHT Sensor
  
  //Connect to Blynk Server
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);  
  Blynk.config(blynk_token);
  timer.setInterval(5000L, dht1SensorData); //Send sensor data to display on app blynk
  timer.setInterval(5000L, getldrSensorData);
  timer.setInterval(10000L, clockDisplay);  
  timer.setInterval(30000L, reconnecting);  //Function reconnect
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
  }
   timer.run();  
}//loop end

BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    Blynk.syncAll();

    // Synchronize time on connection
    rtc.begin();

  if (Blynk.connected())
    {
    Serial.println("Blynk Connected");
    digitalWrite(ledbb,LOW);
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
}

// update switchStatus2 on Temperature
BLYNK_WRITE(V13)
{   
  switchStatus2 = param.asInt(); // Get value as integer
}

// update temperture setting
BLYNK_WRITE(V14)
{   
  templimit = param.asInt(); // Get value as integer
}

// update manualSwitch2
BLYNK_WRITE(V4)
{
  manualSwitch2 = param.asInt();
}

//DHT11-01
void dht1SensorData(){
  float h1 = dht1.readHumidity();
  float t1 = dht1.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h1) || isnan(t1)){ 
 Serial.println("Read from DHT Sensor 1");
  return;
  }

   //LCD Display Temperature and Humidity data
      lcd.begin();
      lcd.backlight();    

      lcd.setCursor(0, 0);
      lcd.print("MBS Openhouse2020");

                                     
      lcd.setCursor(1, 1); 
      lcd.print("TEMP =  "); 
      lcd.print(t1);
      lcd.print("  C ");
      
      lcd.setCursor(1, 2);
      lcd.print("HUMI =  "); 
      lcd.print(h1);
      lcd.print("  % ");
      
  Blynk.virtualWrite(V7, t1);
  Blynk.virtualWrite(V8, h1);
  
  if(switchStatus2)
  {
    // auto
    if(t1 > templimit)
    {
        digitalWrite(Relay2_Fan1, HIGH);  
        Blynk.virtualWrite(V4, 1);
        Blynk.setProperty(Widget_LED_cfan, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_cfan, "label", "เปิดพัดลมระบายอากาศ");
        LedBlynkcfan.on(); 

    }  
    else
    {
        digitalWrite(Relay2_Fan1, LOW);
        Blynk.virtualWrite(V4, 0);
        Blynk.virtualWrite(Widget_LED_cfan, 0);
        Blynk.setProperty(Widget_LED_cfan, "label", "ปิดพัดลมระบายอากาศ");                       
        LedBlynkcfan.off();  
    }
  }
  else
  {
    if(manualSwitch2)
    {
        digitalWrite(Relay2_Fan1, HIGH);
        Blynk.setProperty(Widget_LED_cfan, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_cfan, "label", "เปิดพัดลมระบายอากาศ");
        LedBlynkcfan.on(); 
    }
    else
    {
        digitalWrite(Relay2_Fan1, LOW);
        Blynk.setProperty(Widget_LED_cfan, "label", "ปิดพัดลมระบายอากาศ");                       
        LedBlynkcfan.off();
    }
    // manaul
  }
}

// update switchStatus1 on Light
BLYNK_WRITE(V11)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}

// update LDR setting
BLYNK_WRITE(V12)
{   
  LightLimit1 = param.asInt(); // Get value as integer
}

// update manualSwitch
BLYNK_WRITE(V2)
{
  manualSwitch1 = param.asInt();
}


//LDR Sensor
void getldrSensorData()
{
  float ldr_percentage;
  int sensor_analog;
  sensor_analog = analogRead(A0);
  Serial.print("Law LDR data:");
  Serial.println(sensor_analog);
  ldr_percentage = ( 100 - ( (sensor_analog/1024.00) * 100 ) );
  Blynk.virtualWrite(V0,(ldr_percentage)); 
  Serial.print("LDR Percentage = ");
  Serial.print(ldr_percentage);
  Serial.print("%\n\n");
  delay(1000);
   lcd.setCursor(1, 3);
      lcd.print("Light =  "); 
      lcd.print(ldr_percentage);
      lcd.print("  % ");    
     Serial.println();
     delay(3000); 

if(switchStatus1)
  {
    // auto
    if(ldr_percentage < LightLimit1)
    {
        digitalWrite(Relay1_Light, HIGH);  
        Blynk.virtualWrite(V2, 1);
                
        Blynk.setProperty(Widget_LED_Light, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Light, "label", "เปิดไฟ");
        LedBlynkLight.on(); 
    }  
    else
    {
        digitalWrite(Relay1_Light, LOW);
        Blynk.virtualWrite(V2, 0);
        Blynk.virtualWrite(Widget_LED_Light, 0);
        Blynk.setProperty(Widget_LED_Light, "label", "ปิดไฟ");                       
        LedBlynkLight.off();  
    }
  }
  else
  {
    if(manualSwitch1)
    {
        digitalWrite(Relay1_Light, HIGH);        
        Blynk.setProperty(Widget_LED_Light, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Light, "label", "เปิดไฟ");
        LedBlynkLight.on(); 
    }
    else
    {
        digitalWrite(Relay1_Light, LOW);
        Blynk.setProperty(Widget_LED_Light, "label", "ปิดไฟ");                       
        LedBlynkLight.off();
    }
    // manaul
  }
}

//Display Current Date/Time
void clockDisplay()
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  Blynk.virtualWrite(V9, currentTime);
  Blynk.virtualWrite(V10, currentDate);
}
