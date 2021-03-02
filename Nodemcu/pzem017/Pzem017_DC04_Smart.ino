//อ้างอิง https://solarduino.com/pzem-017-dc-energy-meter-online-monitoring-with-blynk-app
////////// Pin App blynk////////////////
// V0 = สัญญาณไวไฟ
// V1 = Ledblynk
// V2 = Voltage
// V3 = Cerrunt
// V4 = Energy
// V5 = Power
// V6 = Reset Energy
// V7 = Soil Moisture Sensor1
// V8 = ไฟสถานะปุ่มวาล์ว1
// V9 = ปุ่มเปิดปิดวาล์ว1
// V10 = Auto&Manual valve1
// V11 = Slider Valve1
// V12 = ไฟสถานะปุ่มพัดลมระบายอากาศ
// V13 = ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
// V14 = Auto&Manual Cfan
// V15 = Slider Cfan
// V16 = Current Time
// V17 = Current Date  
// V18 = Temperature
// V19 = Humidity
//////////////////////////

//////MCU Digital Pin/////
// D0 = MAX485_DE
// D1 = MAX485_RE
// D2 = MAX485_RO
// D3 = MAX485_DI
// D4 = Connect to Relay1 IN1 >> Check Blynk connected NC1,COM1, NO1
// D5 = Connect to Relay2 IN2 >> Cooling Fan NC2, COM2, NO2
// D6 = Connect ot Relay3 IN3 >> Switch ON/OFF Valve Relay3 NC3, COM3, NO3
// D7 = Connect ot DHT VCC, GND, Out
//////////////////////////

//////MCU Analog Pin/////
// A0 = Soil Moisture Sensor
//////////////////////////

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

//MQTT
#include <PubSubClient.h>
const char* mqtt_server = "";   //ใส่ MQTT Server 
WiFiClient nodepzem016_01;      //ถ้าเชื่อมต่อเข้า MQTT Server หลายโหนดพร้อมกันให้เปลี่ยยนชื่อให้ไม่เหมือนกัน
PubSubClient client(nodepzem016_01);

//Max485
#include <ModbusMaster.h>
ModbusMaster node;
#define MAX485_DE 16          //NodeMCU D0
#define MAX485_RE 5           //NodeMCU D1

//Serail communication
#include <SoftwareSerial.h>
SoftwareSerial PZEMSerial;

//Pzem017
static uint8_t pzemSlaveAddr = 0x01;
static uint16_t NewshuntAddr = 0x0001; // shunt. Default 0x0000 is 100A, replace to "0x0001" if using 50A shunt, 0x0002 is for 200A, 0x0003 is for 300A
float PZEMVoltage = 0;
float PZEMCurrent = 0;
float PZEMPower = 0;
float PZEMEnergy = 0;
unsigned long startMillisPZEM;
unsigned long currentMillisPZEM;
const unsigned long periodPZEM = 1500;
unsigned long startMillisReadData;
unsigned long currentMillisReadData;
const unsigned long periodReadData = 1000;
int ResetEnergy = 0;
int Reset_Energy = 0;
int a = 1;
unsigned long startMillis1;
int status_led = 1;

//RTC Widget
#include <TimeLib.h>
#include <WidgetRTC.h>
WidgetRTC rtc;

//DHT11-01
#include <DHT.h>
#define DHTPIN1 D7
#define DHTTYPE1 DHT11
DHT dht1(DHTPIN1, DHTTYPE1);

//Valve1
//V10 Auto&Manual valve1
//V11 Slider Valve1
//V7 Soil Moisture Sensor1
#define Widget_LED_Valve1 V8          //ไฟสถานะปุ่ม 
#define Widget_Btn_Valve1 V9          //ปุ่ม เปิด-ปิด Valve1
WidgetLED LedBlynkValve1(Widget_LED_Valve1);

//Slider for set Soil limit
bool switchStatus1 = 0;               // 0 = manual,1=auto
int SoilsensorLimit1 = 0;
bool manualSwitch1 = 0;

//Cooling Fan
//V14 Auto&Manual Cfan
//V15 Slider Cfan
#define Widget_LED_cfan V12           //ไฟสถานะปุ่มพัดลมระบายอากาศ
#define Widget_Btn_cfan V13           //ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
WidgetLED LedBlynkcfan(Widget_LED_cfan);

//Slider for set temperature limit
bool switchStatus2  = 0;              // 0 = manual,1=auto
int templimit       = 0;
bool manualSwitch2  = 0;

#define ledblynk      D4
#define ledfan        D5
#define Relay3_Valve1 D6

char blynk_token[34] = "";           //ไม่ต้องกำหนด เราจะกำหนดผ่าน web config

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

//Setup Function
void setup()
{
// put your setup code here, to run once:
Serial.begin(9600);
Serial.println();
startMillis1 = millis();
PZEMSerial.begin(9600, SWSERIAL_8N1, 4, 0);   //NodeMCU D2,D3
startMillisPZEM = millis();

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
      } else 
        {
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
//useful to make it all retry or go to sleep in seconds
wifiManager.setTimeout(60);

//fetches ssid and pass and tries to connect
//if it does not connect it starts an access point with the specified name
//here  "AutoConnectAP"
//and goes into a blocking loop awaiting configuration
//if (WiFi.SSID()!="") wifiManager.setConfigPortalTimeout(60);
 
if (!wifiManager.autoConnect("nodepzem016_01", "password")) {
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
  
//Set pin mode
  pinMode(MAX485_DE, OUTPUT);         //NODEMCU PIN D0
  pinMode(MAX485_RE, OUTPUT);         //NODEMCU PIN D1
  //SWSERIAL_8N2, D2, D3                    
  pinMode(ledblynk,OUTPUT);           // NODEMCU PIN D4
  pinMode(ledfan,OUTPUT);             // NODEMCU PIN D5
  pinMode(Relay3_Valve1,OUTPUT);      // NODEMCU PIN D6

//Set Default status >> Relay Active LOW
  digitalWrite(MAX485_DE, 0);
  digitalWrite(MAX485_RE, 0);
  digitalWrite(ledblynk,HIGH);         // NODEMCU PIN D4
  digitalWrite(ledfan,HIGH);           // NODEMCU PIN D5
  digitalWrite(Relay3_Valve1,HIGH);    // NODEMCU PIN D6
  
node.preTransmission(preTransmission);
node.postTransmission(postTransmission);
node.begin(pzemSlaveAddr, PZEMSerial);
delay(1000);
startMillisReadData = millis();
client.setServer(mqtt_server, 1883);

//เริ่มอ่านข้อมูล DHT Sensor
dht1.begin();  

//Connect to blynk
//Blynk.begin(auth, ssid, pass, "custom server", 8080); 
Blynk.config(blynk_token);
timer.setInterval(1500L, sentWiFi);
timer.setInterval(5000L, dht1SensorData); //Send sensor data to display on app blynk
timer.setInterval(5000L, getsoilSensorData);
timer.setInterval(10000L, clockDisplay);
timer.setInterval(30000L, reconnecting);  //Function reconnect
}
BLYNK_CONNECTED() {
Blynk.syncAll();
}

//Reconnect to MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("nodepzem016_01")) {
      Serial.println("connected1");  
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//Reconnect to blynk
void reconnecting()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    Blynk.syncAll();
    digitalWrite(ledblynk, LOW); //ledpin for check blynk connected 
    // Synchronize time on connection
    rtc.begin();
  }
}

//Read Pzem data
void PZEM017DC()
{
if ((millis() - startMillis1 >= 10000) && (a == 1))
{
setShunt(pzemSlaveAddr);
changeAddress(0XF8, pzemSlaveAddr);
a = 0;
}
currentMillisPZEM = millis();
if (currentMillisPZEM - startMillisPZEM >= periodPZEM)
{
uint8_t result;
result = node.readInputRegisters(0x0000, 6);
if (result == node.ku8MBSuccess)
{
uint32_t tempdouble = 0x00000000;
PZEMVoltage = node.getResponseBuffer(0x0000) / 100.0;
PZEMCurrent = node.getResponseBuffer(0x0001) / 100.0;
tempdouble = (node.getResponseBuffer(0x0003) << 16) + node.getResponseBuffer(0x0002);
PZEMPower = tempdouble / 10.0;
tempdouble = (node.getResponseBuffer(0x0005) << 16) + node.getResponseBuffer(0x0004);
PZEMEnergy = tempdouble;
if (pzemSlaveAddr == 2)
{
}
}
else
{
}
startMillisPZEM = currentMillisPZEM ;
}
currentMillisReadData = millis();
if (currentMillisReadData - startMillisReadData >= periodReadData)
{
int conn = Blynk.connected();
if (conn < 1) {
//digitalWrite(ledblynk, LOW);
} else {
if (status_led == 0) {
status_led = 1;
Blynk.virtualWrite(V1, 255); //-------------------------------------------ส่งค่าเข้า Pin แอป Blynk
//digitalWrite(ledblynk, HIGH);
} else {
status_led = 0;
Blynk.virtualWrite(V1, 0); //-------------------------------------------ส่งค่าเข้า Pin แอป Blynk
//digitalWrite(ledblynk, LOW);
}
Reset_Energy = 0;
}
Serial.print("Vdc : "); Serial.print(PZEMVoltage); Serial.println(" V ");
Serial.print("Idc : "); Serial.print(PZEMCurrent); Serial.println(" A ");
Serial.print("Power : "); Serial.print(PZEMPower); Serial.println(" W ");
Serial.print("Energy : "); Serial.print(PZEMEnergy); Serial.println(" Wh ");
Blynk.virtualWrite(V2, PZEMVoltage);  //V2 Display Voltage
Blynk.virtualWrite(V3, PZEMCurrent);  //V3 Display Current
Blynk.virtualWrite(V4, PZEMEnergy);   //V4 Display Energy
Blynk.virtualWrite(V5, PZEMPower);    //V5 Power
startMillisReadData = millis();

//Publish data to MQTT
static char voltagein[7]; 
dtostrf (PZEMVoltage,6,2,voltagein);
client.publish("solar/voltagein", voltagein); //Publish message to MQTT Topic solar/voltagein

static char currentin[7]; 
dtostrf (PZEMCurrent,6,2,currentin);
client.publish("solar/currentin", currentin); //Publish message to MQTT Topic solar/currentin

static char powerin[7]; 
dtostrf (PZEMPower,6,2,powerin);
client.publish("solar/powerin", powerin);     //Publish message to MQTT Topic solar/powerin

static char energyin[7]; 
dtostrf (PZEMEnergy,6,2,energyin);
client.publish("solar/energyin", energyin);   //Publish message to MQTT Topic solar/energyin 
}
}

//Wi-Fi Signal
long rssi;
void sentWiFi()
{
rssi = WiFi.RSSI();             // อ่านสัญญาณไวไฟ
if (rssi < -99)
{
rssi = -99;
} else {
rssi = rssi + 100;
}
Blynk.virtualWrite(V0, rssi);   //V0 Display Wi-Fi Signal Widget on Blynk
}

//PreTransmission Pzem017 Data
void preTransmission()
{
if (millis() - startMillis1 > 5000)
{
digitalWrite(MAX485_DE, 1);
digitalWrite(MAX485_RE, 1);
delay(1);
}
}

//PostTransmission Pzem017 Data
void postTransmission()
{
if (millis() - startMillis1 > 5000)
{
delay(3);
digitalWrite(MAX485_DE, 0);
digitalWrite(MAX485_RE, 0);
}
}

//Set Shunt address
void setShunt(uint8_t slaveAddr)
{
static uint8_t SlaveParameter = 0x06;
static uint16_t registerAddress = 0x0003;
uint16_t u16CRC = 0xFFFF;
u16CRC = crc16_update(u16CRC, slaveAddr);
u16CRC = crc16_update(u16CRC, SlaveParameter);
u16CRC = crc16_update(u16CRC, highByte(registerAddress));
u16CRC = crc16_update(u16CRC, lowByte(registerAddress));
u16CRC = crc16_update(u16CRC, highByte(NewshuntAddr));
u16CRC = crc16_update(u16CRC, lowByte(NewshuntAddr));
preTransmission();
PZEMSerial.write(slaveAddr);
PZEMSerial.write(SlaveParameter);
PZEMSerial.write(highByte(registerAddress));
PZEMSerial.write(lowByte(registerAddress));
PZEMSerial.write(highByte(NewshuntAddr));
PZEMSerial.write(lowByte(NewshuntAddr));
PZEMSerial.write(lowByte(u16CRC));
PZEMSerial.write(highByte(u16CRC));
delay(10);
postTransmission();
delay(100);
}

//Reset_Energy
BLYNK_WRITE(V6)
{
if (param.asInt() == 1)
{
Reset_Energy = Reset_Energy + 1;
Serial.println(Reset_Energy);
if (Reset_Energy > 2) {
Serial.println("DCresetEnergy");
uint16_t u16CRC = 0xFFFF;
static uint8_t resetCommand = 0x42;
uint8_t slaveAddr = pzemSlaveAddr;
u16CRC = crc16_update(u16CRC, slaveAddr);
u16CRC = crc16_update(u16CRC, resetCommand);
preTransmission();
PZEMSerial.write(slaveAddr);
PZEMSerial.write(resetCommand);
PZEMSerial.write(lowByte(u16CRC));
PZEMSerial.write(highByte(u16CRC));
delay(10);
postTransmission();
delay(100);
}
}
}

//change pzem Address
void changeAddress(uint8_t OldslaveAddr, uint8_t NewslaveAddr)
{
static uint8_t SlaveParameter = 0x06;
static uint16_t registerAddress = 0x0002;
uint16_t u16CRC = 0xFFFF;
u16CRC = crc16_update(u16CRC, OldslaveAddr);
u16CRC = crc16_update(u16CRC, SlaveParameter);
u16CRC = crc16_update(u16CRC, highByte(registerAddress));
u16CRC = crc16_update(u16CRC, lowByte(registerAddress));
u16CRC = crc16_update(u16CRC, highByte(NewslaveAddr));
u16CRC = crc16_update(u16CRC, lowByte(NewslaveAddr));
preTransmission();
PZEMSerial.write(OldslaveAddr);
PZEMSerial.write(SlaveParameter);
PZEMSerial.write(highByte(registerAddress));
PZEMSerial.write(lowByte(registerAddress));
PZEMSerial.write(highByte(NewslaveAddr));
PZEMSerial.write(lowByte(NewslaveAddr));
PZEMSerial.write(lowByte(u16CRC));
PZEMSerial.write(highByte(u16CRC));
delay(10);
postTransmission();
delay(100);
}

//Display Current Date/Time
void clockDisplay()
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  Blynk.virtualWrite(V16, currentTime);
  Blynk.virtualWrite(V17, currentDate);
}

// Update switchStatus1 for Soil Moisture Sensor
BLYNK_WRITE(V10)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}

// Update Soil setting
BLYNK_WRITE(V11)
{   
  SoilsensorLimit1 = param.asInt(); // Get value as integer
}

// Update manualSwitch for control Valve
BLYNK_WRITE(V9)
{
  manualSwitch1 = param.asInt();
}

//Reading Soil Moisture Sensor1 Data
void getsoilSensorData()
{
  float soil_percentage;
  int sensor_analog;
  sensor_analog = analogRead(A0);
  Serial.print("Law Soil data:");
  Serial.println(sensor_analog);
  soil_percentage = ( 100 - ( (sensor_analog/1024.00) * 100 ) );  //Mapping to 100%
  Blynk.virtualWrite(V7,(soil_percentage)); 
  Serial.print("Soil Percentage = ");
  Serial.print(soil_percentage);
  Serial.print("%\n\n");
  delay(1000);
  
//Publish Soil Moisture Sensor Data to MQTT
  static char soil1[7]; 
  dtostrf (soil_percentage,6,2,soil1);    //แปลงค่าที่รับเข้ามาจาก char ไปเป็น string
  client.publish("soil/soil1", soil1);    //Publish message to MQTT Server to topic soil/soil1
    
  if(switchStatus1)
    {
    // auto
      if(soil_percentage < SoilsensorLimit1)  //โหมดอัตโนมัติ ถ้าระดับ % ของ Soil Moisture ที่อ่านค่าได้มีค่าน้อยกว่าที่ตั้งค่าไว้ที่ Slider ให้เปิด วาล์ว และถ้ามากกว่าให้ปิดวาล์ว
      {
        digitalWrite(Relay3_Valve1, LOW);  
        Blynk.virtualWrite(V9, 1);                
        Blynk.setProperty(Widget_LED_Valve1, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์ว");
        LedBlynkValve1.on(); 
      }  
      else
      {
        digitalWrite(Relay3_Valve1, HIGH);
        Blynk.virtualWrite(V9, 0);
        Blynk.virtualWrite(Widget_LED_Valve1, 0);
        Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์ว");                       
        LedBlynkValve1.off();  
    }
  }
  else
  {
    if(manualSwitch1)       //โหมด Manual เปิดปิดด้วยการกดปุ่ม
    {
        digitalWrite(Relay3_Valve1, LOW);        
        Blynk.setProperty(Widget_LED_Valve1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์ว");
        LedBlynkValve1.on(); 
    }
    else
    {
        digitalWrite(Relay3_Valve1, HIGH);
        Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์ว");                       
        LedBlynkValve1.off();
    }
    // manaul
  }
}


// Update Temperture setting
BLYNK_WRITE(V14)
{   
  templimit = param.asInt(); // Get value as integer
}

// Update manualSwitch2 for control cooling fan
BLYNK_WRITE(V15)
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
//Publish Temperature & Humidity to MQTT
  static char temp[7]; 
  dtostrf (t1,6,2,temp);
  client.publish("dht/temp", temp);

  static char humi[7]; 
  dtostrf (h1,6,2,humi);
  client.publish("dht/humi", humi); 
  
  Blynk.virtualWrite(V18, t1);
  Blynk.virtualWrite(V19, h1);
      
  if(switchStatus2)
  {
    // auto
    if(t1 > templimit) //โหมดอัตโนมัติ ถ้าระดับของอุณหภูมิที่อ่านค่าได้มีค่ามากกว่าที่ตั้งค่าไว้ที่ Slider ให้เปิดพัดลมระบายอากาศและถ้าน้อยกว่าให้ปิดพัดลมระบายอากาศ
    {
        digitalWrite(ledfan, LOW);  
        Blynk.virtualWrite(V13, 1);
        Blynk.setProperty(Widget_LED_cfan, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_cfan, "label", "เปิดพัดลมระบายอากาศ");
        LedBlynkcfan.on(); 

    }  
    else
    {
        digitalWrite(ledfan, HIGH);
        Blynk.virtualWrite(V13, 0);
        Blynk.virtualWrite(Widget_LED_cfan, 0);
        Blynk.setProperty(Widget_LED_cfan, "label", "ปิดพัดลมระบายอากาศ");                       
        LedBlynkcfan.off();  
    }
  }
  else
  {
    if(manualSwitch2) //โหมด Manual เปิดปิดด้วยการกดปุ่ม
    {
        digitalWrite(ledfan, LOW);
        Blynk.setProperty(Widget_LED_cfan, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_cfan, "label", "เปิดพัดลมระบายอากาศ");
        LedBlynkcfan.on(); 
    }
    else
    {
        digitalWrite(ledfan, HIGH);
        Blynk.setProperty(Widget_LED_cfan, "label", "ปิดพัดลมระบายอากาศ");                       
        LedBlynkcfan.off();
    }
    // manaul
  }
}

//Loop 
void loop()
{
if (!client.connected()) {
    //reconnect();
  }
  if(!client.loop())
  client.connect("nodepzem016_01");
    
Blynk.run();
timer.run();
PZEM017DC();
}
