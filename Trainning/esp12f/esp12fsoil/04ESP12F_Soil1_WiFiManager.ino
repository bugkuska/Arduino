//==============New Blynk IoT===============//
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_FIRMWARE_VERSION        "0.2.0" 
//==============New Blynk IoT===============//

//============Blynk & WiFiManager===========//
#include <FS.h>                   
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
bool shouldSaveConfig = false;
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
ModbusMaster node1;
//===================Modbus=================//

//==========Define MCU Digital Pin==========//
//Relay1_Pump           16
//Relay2_Valve1         14
//Relay3_Valve2         12
//Relay4_ledblynk       13
//MAX485 DI=TX          15
//MAX485_RE_NEG=DE      5
//MAX485_RE             4
//MAX485 RO=RX          2   
//==========Define MCU Digital Pin==========//

//=========Define Blynk Virtual Pin=========//
//V1  ปุ่ม เปิด-ปิด Valve1
//V2  ปุ่ม เปิด-ปิด Valve2
//V3  ไฟสถานะปุ่ม Pump
//V4  ปุ่ม เปิด-ปิด Pump
//V5  SoilMoisture1
//=========Define Blynk Virtual Pin=========//

//=============ปุ่ม เปิด-ปิด Valve1=============//
//V11 Auto&Manual Valve1
//V12 Slider SoilMoisture
#define Relay1_Valve1           16
#define Widget_Btn_Valve1       V1     
//Slider for set Soil limit
bool switchStatus1 = 0; // 0 = manual,1=auto
int SoilLevelLimit1 = 0;
bool manualSwitch1 = 0;
//=============ปุ่ม เปิด-ปิด Valve1=============//
     
//=============ปุ่ม เปิด-ปิด Valve2=============//
#define Relay2_Valve2         14
#define Widget_Btn_Valve2     V2          
//=============ปุ่ม เปิด-ปิด Valve2=============//

//=============ปุ่ม เปิด-ปิด Pump===============//
#define Relay3_Pump             12
#define Widget_LED_Pump     V3              //ไฟสถานะปุ่ม Pump
#define Widget_Btn_Pump     V4              //ปุ่ม เปิด-ปิด Pump
WidgetLED LedBlynkPump(Widget_LED_Pump);
//=============ปุ่ม เปิด-ปิด Pump===============//

//====Pilotlamp check blynk connected=======//
#define Relay4_ledblynk       13
//====Pilotlamp check blynk connected=======//

//=====Modbus Pre & Post Transmission1======//
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
//=====Modbus Pre & Post Transmission1======//

//===============Setup Function=============//
void setup()
{
//Modbus pinMode
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_RE, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_RE, 0);
  Serial.println("start init serial 0");
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
          strcpy(blynk_token, json["blynk_token"]);
        } else 
        {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
//End read
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);

//WiFiManager
  WiFiManager wifiManager;
  
//Set config save notify callback
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
 
  if (!wifiManager.autoConnect("xESP12FSoil01x", "password")) {
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
  
//=====Beging Modbus Nodex=====// 
  while (!Serial) {
    Serial.println("loop for init serial 0"); // wait for serial port to connect. Needed for Native USB only
  }
  Serial.println("start init software serial");
  mySerial.begin(9600);
  while (!mySerial) {
    Serial.println("loop for init software serial"); // wait for serial port to connect. Needed for Native USB only
  }  
//Modbus slave ID 
  node1.begin(1, mySerial);
//=====Beging Modbus Nodex=====//
  
//==Callbacks allow us to configure the RS485 transceiver correctly==//
  node1.preTransmission(preTransmission1);
  node1.postTransmission(postTransmission1);
//==Callbacks allow us to configure the RS485 transceiver correctly==//    
 
  Serial.println();

//Setup Pin Mode
  pinMode(Relay1_Valve1,OUTPUT);                // NODEMCU PIN gpio16 
  pinMode(Relay2_Valve2,OUTPUT);                // NODEMCU PIN gpio14
  pinMode(Relay3_Pump,OUTPUT);                  // NODEMCU PIN gpio12   
  pinMode(Relay4_ledblynk,OUTPUT);              // NODEMCU PIN GPIO13          
//Set Defult Relay Status
  digitalWrite(Relay1_Valve1,LOW);              // NODEMCU PIN gpio16
  digitalWrite(Relay2_Valve2,LOW);              // NODEMCU PIN gpio14
  digitalWrite(Relay3_Pump,LOW);                // NODEMCU PIN gpio12
  digitalWrite(Relay4_ledblynk,LOW);            // NODEMCU PIN gpio13  
  
//=====Begin config Blynk & Set time interval=====//
  Blynk.config(blynk_token); 
  timer.setInterval(5000L, soil1);
  timer.setInterval(30000L,reconnecting);
//===Begin config Blynk & Set time interval=======//
}
//===============Setup Function===================//

//=====Update switchStatus1 on SoilMoisture=======//
BLYNK_WRITE(V11)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}
//===========Update SoilMoisture setting=========//
BLYNK_WRITE(V12)
{   
  SoilLevelLimit1 = param.asInt(); // Get value as integer
}
//================Update manualSwitch===========//
BLYNK_WRITE(V1)
{
  manualSwitch1 = param.asInt();
}
//==============SoilMoisture Sensor=============//
void soil1()
{
  uint8_t result;
  float soil1 = (node1.getResponseBuffer(2)/10.0f); 
  Serial.println("get data");
  result = node1.readHoldingRegisters(0x0000, 3); // Read 3 registers starting at 1)
  if (result == node1.ku8MBSuccess)
  {
    Serial.print("Humi: ");
    Serial.println(node1.getResponseBuffer(2)/10.0f);
  }
  delay(1000);
  Blynk.virtualWrite(V5, soil1);
  
  if(switchStatus1)
  {
    // auto
    if(soil1 < SoilLevelLimit1)
    {
        digitalWrite(Relay1_Valve1,HIGH);
        Blynk.virtualWrite(V1,1);

        digitalWrite(Relay3_Pump,HIGH);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); 
    }  
    else
    {
        digitalWrite(Relay1_Valve1,LOW);
        Blynk.virtualWrite(V1,0);

        digitalWrite(Relay3_Pump,LOW);   
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();
    }
  }
  else
  {
    if(manualSwitch1)
    {       
        digitalWrite(Relay1_Valve1, HIGH);  
        Blynk.virtualWrite(V1,1);
        digitalWrite(Relay3_Pump,HIGH);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); 
    }
    else
    {      
        digitalWrite(Relay1_Valve1,LOW);
        Blynk.virtualWrite(V1,0);
        digitalWrite(Relay3_Pump,LOW);   
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();  
    }
    // manaul
  }
}
//==============SoilMoisture Sensor=============//

//=============BUTTON ON/OFF Valve2=============//
 BLYNK_WRITE(Widget_Btn_Valve2){
      int valueValve2 = param.asInt();
      if(valueValve2 == 1){
        digitalWrite(Relay2_Valve2,HIGH);    
      }
       else{              
        digitalWrite(Relay2_Valve2,LOW);       
     }
}
//=============BUTTON ON/OFF Valve2=============//

//==================Blynk Conneted==============//
BLYNK_CONNECTED()
{
 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(Relay4_ledblynk, HIGH);  //ledpin for check blynk connected 
    Serial.println("Blynk connected");
    Blynk.syncAll();
 }
}
//==================Blynk Conneted==============//

//===============Reconnect to blynk=============//
void reconnecting()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    Blynk.syncAll();
    digitalWrite(Relay4_ledblynk, HIGH);
  }
}
//===============Reconnect to blynk=============//

//===================Loop function==============//
void loop()
{
  if (Blynk.connected())
  {
    Blynk.run();
    Blynk.syncAll();
  }
   timer.run();  
}
//===================Loop function==============//
