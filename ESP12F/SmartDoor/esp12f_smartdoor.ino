//=================Blynk Virtual Pin=================//
//V1  push-btn open the door
//V2  push-btn close the door
//V3  push-btn stop the door
//=================Blynk Virtual Pin=================//
//==============Libraries for project================//
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         
#include <SimpleTimer.h>
#include <ArduinoJson.h>          
BlynkTimer timer;
int blynkIsDownCount=0;
char blynk_token[34] = "";
//==============Libraries for project================//

//==============Define Virtual Pin===================//
#define relay1_open 16                    //V1  push-btn open the door
#define Widget_Btn_relay1 V1 
#define relay2_close 14                   //V2  push-btn close the door
#define Widget_Btn_relay2 V2 
#define relay3_stop 12                    //V3  push-btn stop the door          
#define Widget_Btn_relay3 V3 
#define relay4_check_blynk 13             //Check Blynk connection         
#define Widget_Btn_relay4 V4 
//==============Define Virtual Pin===================//

//============flag for saving data==================//
bool shouldSaveConfig = false;
//============flag for saving data==================//

//===========callback to save config================//
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
//===========callback to save config================//

//==================Setup Function==================//
void setup()
{
  Serial.begin(115200);
  Serial.println();

  // Setup Pin Mode
  pinMode(relay1_open,OUTPUT);                    // NODEMCU PIN gpio16 
  pinMode(relay2_close,OUTPUT);                   // NODEMCU PIN gpio14
  pinMode(relay3_stop,OUTPUT);                    // NODEMCU PIN gpio12   
  pinMode(relay4_check_blynk,OUTPUT);             // NODEMCU PIN GPIO13         
  
  // Set Defult Relay Status
  digitalWrite(relay1_open,LOW);                  // NODEMCU PIN gpio16
  digitalWrite(relay2_close,LOW);                 // NODEMCU PIN gpio14
  digitalWrite(relay3_stop,LOW);                  // NODEMCU PIN gpio12
  digitalWrite(relay4_check_blynk,LOW);           // NODEMCU PIN gpio13
  
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
  
  if (!wifiManager.autoConnect("smartdoor", "0814111142")) {
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
}
//==================Setup Function==================//

//=================Blynk connected==================//
BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    //Blynk.syncAll();

 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(relay4_check_blynk, HIGH);
 }
}
//=================Blynk connected==================//

//=================Blynk Reconnect==================//
void reconnecting()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    //Blynk.syncAll();
    digitalWrite(relay4_check_blynk, HIGH);
  }
}
//=================Blynk Reconnect==================//

//==================Loop Function===================//
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
}

//==================Loop Function===================//

//=============Push BUTTON Open the door============//
BLYNK_WRITE(Widget_Btn_relay1){
      int valuedoor = param.asInt();
      if(valuedoor == 1){    
      digitalWrite(relay1_open,HIGH);              
      }
       else{                    
       digitalWrite(relay1_open,LOW);
     }
} 
//=============Push BUTTON Open the door============//

//=============Push BUTTON Close the door===========//
BLYNK_WRITE(Widget_Btn_relay2){
      int valuedoor = param.asInt();
      if(valuedoor == 1){    
      digitalWrite(relay2_close,HIGH);              
      }
       else{                    
       digitalWrite(relay2_close,LOW);
     }
} 
//=============Push BUTTON Close the door===========//

//=============Push BUTTON Stop the door============//
BLYNK_WRITE(Widget_Btn_relay3){
      int valuedoor = param.asInt();
      if(valuedoor == 1){    
      digitalWrite(relay3_stop,HIGH);              
      }
       else{                    
       digitalWrite(relay3_stop,LOW);
     }
} 
//=============Push BUTTON Stop the door============//
