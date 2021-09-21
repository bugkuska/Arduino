//==============New Blynk IoT===============//
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_FIRMWARE_VERSION        "0.2.0" //time interval 10 secs
//==============New Blynk IoT===============//

//============Blynk & WiFiManager===========//
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
//============Blynk & WiFiManager===========//

//===================Modbus=================//
#include <SoftwareSerial.h>
#include <ModbusMaster.h>
SoftwareSerial mySerial(2, 15); // RO=RX, DI=TX
#define MAX485_RE      4  //RE
#define MAX485_RE_NEG  5  //DE
// instantiate ModbusMaster object
ModbusMaster node1;
//===================Modbus=================//

//==Pool size for Modbus Write command==//
int8_t pool_size1;
//==Pool size for Modbus Write command==//

//==Define 4CH Relay==//
#define Relay4_ledblynk       13
//==Define 4CH Relay==//

//==========Define MCU Digital Pin==========//
//N/A       16
//N/A       14
//N/A       12
//Relay4_ledblynk       13
//MAX485 DI=TX      15
//MAX485_RE_NEG=DE  5
//MAX485_RE         4
//MAX485 RO=RX      2   
//ว่าง                0
//==========Define MCU Digital Pin==========//

//=========Define Blynk Virtual Pin=========//
//V0  ปุ่ม เปิด-ปิด ปั้มน้ำ 1
//V1  ปุ่ม เปิด-ปิด วาล์ว1
//V2  ปุ่ม เปิด-ปิด วาล์ว2
//V3  ปุ่ม เปิด-ปิด วาล์ว3
//V4  ปุ่ม เปิด-ปิด วาล์ว4
//=========Define Blynk Virtual Pin=========//

//=====BTN1=====//
#define Widget_Btn_Pump V0          //ปุ่ม เปิด-ปิด ปั้มน้ำ
#define Widget_Btn_Valve1 V1          //ปุ่ม วาล์ว1
#define Widget_Btn_Valve2 V2          //ปุ่ม วาล์ว2
#define Widget_Btn_Valve3 V3          //ปุ่ม วาล์ว3
#define Widget_Btn_Valve4 V4          //ปุ่ม วาล์ว4

//==Modbus Pre & Post Transmission1==//
void preTransmission1()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_RE, 1);
}
void postTransmission1()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_RE, 0);
}
//==Modbus Pre & Post Transmission1==//

//=====Setup Function=====//
void setup()
{
  //Modbus pinMode
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_RE, OUTPUT);
  // Init in receive mode
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_RE, 0);
  Serial.println("start init serial 0");

  //New
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
 
  if (!wifiManager.autoConnect("xESP12FModbusx", "password")) {
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
  //End New
  
//=====Beging Modbus Nodex=====// 
  while (!Serial) {
    Serial.println("loop for init serial 0"); // wait for serial port to connect. Needed for Native USB only
  }
  Serial.println("start init software serial");
  mySerial.begin(9600);
  while (!mySerial) {
    Serial.println("loop for init software serial"); // wait for serial port to connect. Needed for Native USB only
  }  
  //Modbus slave ID 1
  node1.begin(1, mySerial);
  // Callbacks allow us to configure the RS485 transceiver correctly
  node1.preTransmission(preTransmission1);
  node1.postTransmission(postTransmission1);
  Serial.println();

//=====Setup Pin Mode=====//
  pinMode(Relay4_ledblynk,OUTPUT);                // NODEMCU PIN gpio16          
//=====Setup Pin Mode=====//
  
//=====Set Defult Relay Status=====//
  digitalWrite(Relay4_ledblynk,LOW);              // NODEMCU PIN gpio16
//=====Set Defult Relay Status=====// 
  
//=====Begin config Blynk & Set time interval=====//
  Blynk.config(blynk_token);
  timer.setInterval(30000L, reconnecting); 
}
//=====Setup Function=====//

//****BUTTON ON/OFF SW_Pump****//
 BLYNK_WRITE(Widget_Btn_Pump){
      int valueSW_Pump = param.asInt();
      if(valueSW_Pump == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node1.writeSingleRegister(0x01,0x0100);
      }
       else{                    
       pool_size1 = node1.writeSingleRegister(0x01,0x0200);
     }
} 
//****BUTTON ON/OFF SW_Pump****//

//****BUTTON ON/OFF Valve1****//
 BLYNK_WRITE(Widget_Btn_Valve1){
      int valueValve1 = param.asInt();
      if(valueValve1 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node1.writeSingleRegister(0x05,0x0100);
      }
       else{                    
       pool_size1 = node1.writeSingleRegister(0x05,0x0200);
     }
} 
//****BUTTON ON/OFF Valve1****//

//****BUTTON ON/OFF Valve2****//
 BLYNK_WRITE(Widget_Btn_Valve2){
      int valueValve2 = param.asInt();
      if(valueValve2 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node1.writeSingleRegister(0x06,0x0100);
      }
       else{                    
       pool_size1 = node1.writeSingleRegister(0x06,0x0200);
     }
} 
//****BUTTON ON/OFF Valve2****//

//****BUTTON ON/OFF Valve3****//
 BLYNK_WRITE(Widget_Btn_Valve3){
      int valueValve3 = param.asInt();
      if(valueValve3 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node1.writeSingleRegister(0x07,0x0100); 
      }
       else{                    
       pool_size1 = node1.writeSingleRegister(0x07,0x0200);
     }
} 
//****BUTTON ON/OFF Valve3****//

//****BUTTON ON/OFF Valve4****//
 BLYNK_WRITE(Widget_Btn_Valve4){
      int valueValve4 = param.asInt();
      if(valueValve4 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node1.writeSingleRegister(0x08,0x0100); 
      }
       else{                    
       pool_size1 = node1.writeSingleRegister(0x08,0x0200);
     }
} 
//****BUTTON ON/OFF Valve4****//


//=====Blynk connected=====//
BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    Blynk.syncAll();

 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(Relay4_ledblynk,HIGH);
 }
}
//=====Blynk connected=====//

//=====Reconnect to blynk=====//
void reconnecting()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    Blynk.syncAll();
    digitalWrite(Relay4_ledblynk,HIGH);
  }
}
//=====Reconnect to blynk=====//

//=======Loop function=======//
void loop() 
{
   if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();
}
//=======Loop function=======//
