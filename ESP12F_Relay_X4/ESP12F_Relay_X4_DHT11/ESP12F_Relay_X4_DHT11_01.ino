//*****Define MCU Digital Pin*******//
//Relay1_btn1       16
//Relay2_btn2       14
//Relay3_btn3       12
//Relay4_btn4       13
//DHT11             15
//ว่าง                5
//ว่าง                4
//ว่าง                0
//ว่าง                2
//*****Define MCU Digital Pin*******//

//*****Define Blynk Virtual Pin*****//
//V1  ไฟสถานะปุ่ม 1
//V2  ปุ่ม เปิด-ปิด 1
//V3  ไฟสถานะปุ่ม 2
//V4  ปุ่ม เปิด-ปิด 2
//V5  ไฟสถานะปุ่ม 3
//V6  ปุ่ม เปิด-ปิด 3
//V7  ไฟสถานะปุ่ม 4
//V8  ปุ่ม เปิด-ปิด 4
//V9  Temperature
//V10 Humidity
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
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//BTN1
#define Relay1_btn1   16
#define Widget_LED_btn1 V1          //ไฟสถานะปุ่ม 1
#define Widget_Btn_btn1 V2          //ปุ่ม เปิด-ปิด 1
WidgetLED LedBlynkbtn1(Widget_LED_btn1);

//BTN2
#define Relay2_btn2   14
#define Widget_LED_btn2 V3          //ไฟสถานะปุ่ม 2
#define Widget_Btn_btn2 V4          //ปุ่ม เปิด-ปิด 2
WidgetLED LedBlynkbtn2(Widget_LED_btn2);


//BTN3
#define Relay3_btn3   12
#define Widget_LED_btn3 V5          //ไฟสถานะปุ่ม 3
#define Widget_Btn_btn3 V6          //ปุ่ม เปิด-ปิด 3
WidgetLED LedBlynkbtn3(Widget_LED_btn3);

//BTN4
#define Relay4_btn4   13
#define Widget_LED_btn4 V7          //ไฟสถานะปุ่ม 4
#define Widget_Btn_btn4 V8          //ปุ่ม เปิด-ปิด 4
WidgetLED LedBlynkbtn4(Widget_LED_btn4);

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
 
  if (!wifiManager.autoConnect("ESP12F_DHT11", "password")) {
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
  pinMode(Relay1_btn1,OUTPUT);                // NODEMCU PIN gpio16 
  pinMode(Relay2_btn2,OUTPUT);                // NODEMCU PIN gpio14
  pinMode(Relay3_btn3,OUTPUT);                // NODEMCU PIN gpio12   
  pinMode(Relay4_btn4,OUTPUT);                // NODEMCU PIN GPIO13         
  
  // Set Defult Relay Status
  digitalWrite(Relay1_btn1,LOW);              // NODEMCU PIN gpio16
  digitalWrite(Relay2_btn2,LOW);              // NODEMCU PIN gpio14
  digitalWrite(Relay3_btn3,LOW);              // NODEMCU PIN gpio12
  digitalWrite(Relay4_btn4,LOW);              // NODEMCU PIN gpio13
  
  //Start read DHT11
  dht.begin();  //เริ่มอ่านข้อมูล DHT Sensor
  
  //Blynk.begin(ssid,pass,auth);
  //Connect to Blynk Server
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  Blynk.config(blynk_token);
  timer.setInterval(30000L, reconnecting);  
  timer.setInterval(5000L, dhtSensorData);
}

//****BUTTON ON/OFF btn1****
 BLYNK_WRITE(Widget_Btn_btn1){
      int valuebtn1 = param.asInt();
      if(valuebtn1 == 1){
        digitalWrite(Relay1_btn1,HIGH);
        Blynk.setProperty(Widget_LED_btn1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_btn1, "label", "เปิดปุ่มที่ 1");
        LedBlynkbtn1.on();
        
      }
       else{              
        digitalWrite(Relay1_btn1,LOW);
        Blynk.setProperty(Widget_LED_btn1, "label", "ปิดปุ่มที่ 1");
        LedBlynkbtn1.off();          
     }
}

//****BUTTON ON/OFF btn2****
 BLYNK_WRITE(Widget_Btn_btn2){
      int valuebtn2 = param.asInt();
      if(valuebtn2 == 1){
        digitalWrite(Relay2_btn2,HIGH);
        Blynk.setProperty(Widget_LED_btn2, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_btn2, "label", "เปิดปุ่มที่ 2");
        LedBlynkbtn2.on();
        
      }
       else{              
        digitalWrite(Relay2_btn2,LOW);
        Blynk.setProperty(Widget_LED_btn2, "label", "ปิดปุ่มที่ 2");
        LedBlynkbtn2.off();          
     }
}

//****BUTTON ON/OFF btn3****
 BLYNK_WRITE(Widget_Btn_btn3){
      int valuebtn3 = param.asInt();
      if(valuebtn3 == 1){
        digitalWrite(Relay3_btn3,HIGH);
        Blynk.setProperty(Widget_LED_btn3, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_btn3, "label", "เปิดปุ่มที่ 3");
        LedBlynkbtn3.on();
        
      }
       else{              
        digitalWrite(Relay3_btn3,LOW);
        Blynk.setProperty(Widget_LED_btn3, "label", "ปิดปุ่มที่ 3");
        LedBlynkbtn3.off();          
     }
}

//****BUTTON ON/OFF btn4****
 BLYNK_WRITE(Widget_Btn_btn4){
      int valuebtn4 = param.asInt();
      if(valuebtn4 == 1){
        digitalWrite(Relay4_btn4,HIGH);
        Blynk.setProperty(Widget_LED_btn4, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_btn4, "label", "เปิดปุ่มที่ 4");
        LedBlynkbtn4.on();
        
      }
       else{              
        digitalWrite(Relay4_btn4,LOW);
        Blynk.setProperty(Widget_LED_btn4, "label", "ปิดปุ่มที่ 4");
        LedBlynkbtn4.off();          
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
  Blynk.virtualWrite(V9, t);
  Blynk.virtualWrite(V10, h);
}


BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    Blynk.syncAll();

 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
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
