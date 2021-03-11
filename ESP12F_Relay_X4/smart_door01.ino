//*****Define Digital Pin*****//
//GPIO16  Relay1
//GPIO14  Relay2
//GPIO12  Relay3
//GPIO13  Relay4
////////////////////////////////

//*****Define Virtual Pin*****//
//V1  Relay1
//V2  Relay2
//V3  Relay3
//V4  Relay4
///////////////////////////////

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
char blynk_token[34] = "";        //Token blynk

//Line Notify
#include <TridentTD_LineNotify.h>
#define LINE_TOKEN  ""
uint8_t lastLineMsg = 0;  
//V0 soil Moisture sensor
//V5 DHT11
#define relay1 16
#define Widget_Btn_relay1 V1 

#define relay2 14
#define Widget_Btn_relay2 V2 

#define relay3 12
#define Widget_Btn_relay3 V3 

#define relay4 13
#define Widget_Btn_relay4 V4 

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
  
  if (!wifiManager.autoConnect("ESP12F_Smartdoor01", "password")) {
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

  Blynk.config(blynk_token);
  timer.setInterval(30000L, reconnecting);  //Function reconnect

 //LineNotify
  LINE.setToken(LINE_TOKEN);  // กำหนด Line Token
  
  // Setup Pin Mode
  pinMode(relay1,OUTPUT);         // NODEMCU PIN gpio16 
  pinMode(relay2,OUTPUT);         // NODEMCU PIN gpio14
  pinMode(relay3,OUTPUT);         // NODEMCU PIN gpio12   
  pinMode(relay4,OUTPUT);         // NODEMCU PIN GPIO13         
  
  // Set Defult Relay Status
  digitalWrite(relay1,LOW);         // NODEMCU PIN gpio16
  digitalWrite(relay2,LOW);         // NODEMCU PIN gpio14
  digitalWrite(relay3,LOW);         // NODEMCU PIN gpio12
  digitalWrite(relay4,LOW);           // NODEMCU PIN gpio13
}

BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    //Blynk.syncAll();

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
  }
   timer.run();  
}
//loop end

//****Push BUTTON ON door****
BLYNK_WRITE(Widget_Btn_relay1){
      int valuedoor = param.asInt();
      if(valuedoor == 1){    
      digitalWrite(relay1,HIGH);              
      }
       else{                    
       digitalWrite(relay1,LOW);
     }
} 

//****Push BUTTON OFF door****
BLYNK_WRITE(Widget_Btn_relay2){
      int valuedoor = param.asInt();
      if(valuedoor == 1){    
      digitalWrite(relay2,HIGH);              
      }
       else{                    
       digitalWrite(relay2,LOW);
     }
} 

//****Push BUTTON STOP door****
BLYNK_WRITE(Widget_Btn_relay3){
      int valuedoor = param.asInt();
      if(valuedoor == 1){    
      digitalWrite(relay3,HIGH);              
      }
       else{                    
       digitalWrite(relay3,LOW);
     }
} 

  //****Push BUTTON LOCK door****
BLYNK_WRITE(Widget_Btn_relay4){
      int valuedoor = param.asInt();
      if(valuedoor == 1){    
      digitalWrite(relay4,HIGH);              
      }
       else{                    
       digitalWrite(relay4,LOW);
     }
} 
  
