//อ้างอิง https://solarduino.com/pzem-017-dc-energy-meter-online-monitoring-with-blynk-app
////////// Pin App blynk////////////////
// V0 = สัญญาณ Wi-Fi
// V1 = Ledblynk
// V2 = Voltage
// V3 = Cerrunt
// V4 = Energy
// V5 = Power
// V6 = Reset Energy
// V7 = LDR Sensor1
// V8 = ไฟสถานะปุ่มเปิดปิด
// V9 = ปุ่มเปิดปิดไฟ
// V10 = Auto&Manual Light
// V11 = Slider Light
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
// D5 = Connect to Relay3 IN3 >> Switch ON/OFF Light NC3, COM3, NO3
// D6 = Connect ot Relay4 IN4 >> Switch ON/OFF Cfan NC4, COM4, NO4
// D7 = Connect ot DHT VCC, GND, Out
//////////////////////////

//////MCU Analog Pin/////
// A0 = LDR Sensor
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
WiFiClient Solar_Node2_OUT;      //ถ้าเชื่อมต่อเข้า MQTT Server หลายโหนดพร้อมกันให้เปลี่ยยนชื่อให้ไม่เหมือนกัน
PubSubClient client(Solar_Node2_OUT);

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
#define DHTPIN D7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//Light
//V10 Auto&Manual Light
//V11 Slider Light Limit
//V7 LDR Sensor1
#define Widget_LED_Light V8          //ไฟสถานะปุ่ม 
#define Widget_Btn_Light V9          //ปุ่ม เปิด-ปิด Light
WidgetLED LedBlynkLight(Widget_LED_Light);

//Slider for set Light limit
bool switchStatus1 = 0;               // 0 = manual,1=auto
int LDRsensorLimit1 = 0;
bool manualSwitch1 = 0;

//Cooling Fan
//V14 Auto&Manual Cfan
//V15 Slider Cfan
#define Widget_LED_Cfan V12           //ไฟสถานะปุ่มพัดลมระบายอากาศ
#define Widget_Btn_Cfan V13           //ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
WidgetLED LedBlynkCfan(Widget_LED_Cfan);

//Slider for set temperature limit
bool switchStatus2  = 0;              // 0 = manual,1=auto
int templimit       = 0;
bool manualSwitch2  = 0;

#define ledblynk      D4
#define Relay3_Light  D5
#define Relay4_Cfan   D6

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
 
  if (!wifiManager.autoConnect("Solar_Node2_OUT", "password")) {
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
  
//Set pin mode
  pinMode(MAX485_DE, OUTPUT);         //NODEMCU PIN D0
  pinMode(MAX485_RE, OUTPUT);         //NODEMCU PIN D1
  //SWSERIAL_8N2, D2, D3                    
  pinMode(ledblynk,OUTPUT);           // NODEMCU PIN D4
  pinMode(Relay3_Light,OUTPUT);       // NODEMCU PIN D5
  pinMode(Relay4_Cfan,OUTPUT);        // NODEMCU PIN D6

//Set Default status >> Relay Active LOW
  digitalWrite(MAX485_DE, 0);
  digitalWrite(MAX485_RE, 0);
  digitalWrite(ledblynk,HIGH);        // NODEMCU PIN D4
  digitalWrite(Relay3_Light,HIGH);    // NODEMCU PIN D5
  digitalWrite(Relay4_Cfan,HIGH);     // NODEMCU PIN D6
  
node.preTransmission(preTransmission);
node.postTransmission(postTransmission);
node.begin(pzemSlaveAddr, PZEMSerial);
delay(1000);
startMillisReadData = millis();
client.setServer(mqtt_server, 1883);

//เริ่มอ่านข้อมูล DHT Sensor
dht.begin();  

//Connect to blynk
//Blynk.begin(auth, ssid, pass, "custom server", 8080); 
Blynk.config(blynk_token);
timer.setInterval(1500L, sentWiFi);
timer.setInterval(5000L, getDHTSensorData); //Send sensor data to display on app blynk
timer.setInterval(5000L, getLDRSensorData);
timer.setInterval(10000L, clockDisplay);
timer.setInterval(30000L, reconnecting);  //Function reconnect
}

BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    Blynk.syncAll();

 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(ledblynk,LOW);
 }
}

//Reconnect to MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Solar_Node2_OUT")) {
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
   if (blynkIsDownCount >= 5){
    ESP.restart();
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
static char voltageout[7]; 
dtostrf (PZEMVoltage,6,2,voltageout);
client.publish("Solar_Node2_OUT/voltage_out", voltageout); //Publish message to MQTT Topic Solar_Node2_OUT/voltageout

static char currentout[7]; 
dtostrf (PZEMCurrent,6,2,currentout);
client.publish("Solar_Node2_OUT/current_out", currentout); //Publish message to MQTT Topic Solar_Node2_OUT/currentout

static char powerout[7]; 
dtostrf (PZEMPower,6,2,powerout);
client.publish("Solar_Node2_OUT/power_out", powerout);     //Publish message to MQTT Topic Solar_Node2_OUT/powerout

static char energyout[7]; 
dtostrf (PZEMEnergy,6,2,energyout);
client.publish("Solar_Node2_OUT/energy_out", energyout);   //Publish message to MQTT Topic Solar_Node2_OUT/energyout 
}
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

// Update switchStatus1 for LDR Sensor
BLYNK_WRITE(V10)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}

// Update LDR setting
BLYNK_WRITE(V11)
{   
  LDRsensorLimit1 = param.asInt(); // Get value as integer
}

// Update manualSwitch for control Light
BLYNK_WRITE(V9)
{
  manualSwitch1 = param.asInt();
}

//Reading LDR Sensor1 Data
void getLDRSensorData()
{
  float ldr_percentage;
  int sensor_analog;
  sensor_analog = analogRead(A0);
  Serial.print("Law LDR data:");
  Serial.println(sensor_analog);
  ldr_percentage = ( 100 - ( (sensor_analog/1024.00) * 100 ) );  //Mapping to 100%
  Blynk.virtualWrite(V7,(ldr_percentage)); 
  Serial.print("LDR Percentage = ");
  Serial.print(ldr_percentage);
  Serial.print("%\n\n");
  delay(1000);
  
//Publish Soil Moisture Sensor Data to MQTT
  static char ldr[7]; 
  dtostrf (ldr_percentage,6,2,ldr);    //แปลงค่าที่รับเข้ามาจาก char ไปเป็น string
  client.publish("Solar_Node2_OUT/ldr", ldr);    //Publish message to MQTT Server to topic Solar_Node2_OUT/ldr
    
  if(switchStatus1)
    {
    // auto
      if(ldr_percentage < LDRsensorLimit1)  //โหมดอัตโนมัติ ถ้าระดับ % ของ LDR ที่อ่านค่าได้มีค่าน้อยกว่าที่ตั้งค่าไว้ที่ Slider ให้เปิดไฟและถ้ามากกว่าให้ปิดไฟ
      {
        digitalWrite(Relay3_Light, LOW);  
        Blynk.virtualWrite(V9, 1);                
        Blynk.setProperty(Widget_LED_Light, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Light, "label", "เปิดไฟ");
        LedBlynkLight.on(); 
      }  
      else
      {
        digitalWrite(Relay3_Light, HIGH);
        Blynk.virtualWrite(V9, 0);
        Blynk.virtualWrite(Widget_LED_Light, 0);
        Blynk.setProperty(Widget_LED_Light, "label", "ปิดไฟ");                       
        LedBlynkLight.off();  
    }
  }
  else
  {
    if(manualSwitch1)       //โหมด Manual เปิดปิดด้วยการกดปุ่ม
    {
        digitalWrite(Relay3_Light, LOW);        
        Blynk.setProperty(Widget_LED_Light, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Light, "label", "เปิดไฟ");
        LedBlynkLight.on(); 
    }
    else
    {
        digitalWrite(Relay3_Light, HIGH);
        Blynk.setProperty(Widget_LED_Light, "label", "ปิดไฟ");                       
        LedBlynkLight.off();
    }
    // manaul
  }
}

// Update switchStatus2 for Temperature Sensor
BLYNK_WRITE(V14)
{   
  switchStatus2 = param.asInt(); // Get value as integer
}

// Update Temperture setting
BLYNK_WRITE(V15)
{   
  templimit = param.asInt(); // Get value as integer
}

// Update manualSwitch2 for control cooling fan
BLYNK_WRITE(V13)
{
  manualSwitch2 = param.asInt();
}

//DHT11-01
void getDHTSensorData(){
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)){ 
 Serial.println("Read from DHT Sensor 1");
  return;
  }
//Publish Temperature & Humidity to MQTT
  static char temp[7]; 
  dtostrf (t,6,2,temp);
  client.publish("Solar_Node2_OUT/temp", temp);

  static char humi[7]; 
  dtostrf (h,6,2,humi);
  client.publish("Solar_Node2_OUT/humi", humi); 
  
  Blynk.virtualWrite(V18, t);
  Blynk.virtualWrite(V19, h);
      
  if(switchStatus2)
  {
    // auto
    if(t > templimit) //โหมดอัตโนมัติ ถ้าระดับของอุณหภูมิที่อ่านค่าได้มีค่ามากกว่าที่ตั้งค่าไว้ที่ Slider ให้เปิดพัดลมระบายอากาศและถ้าน้อยกว่าให้ปิดพัดลมระบายอากาศ
    {
        digitalWrite(Relay4_Cfan, LOW);  
        Blynk.virtualWrite(V13, 1);
        Blynk.setProperty(Widget_LED_Cfan, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Cfan, "label", "เปิดพัดลมระบายอากาศ");
        LedBlynkCfan.on(); 

    }  
    else
    {
        digitalWrite(Relay4_Cfan, HIGH);
        Blynk.virtualWrite(V13, 0);
        Blynk.virtualWrite(Widget_LED_Cfan, 0);
        Blynk.setProperty(Widget_LED_Cfan, "label", "ปิดพัดลมระบายอากาศ");                       
        LedBlynkCfan.off();  
    }
  }
  else
  {
    if(manualSwitch2) //โหมด Manual เปิดปิดด้วยการกดปุ่ม
    {
        digitalWrite(Relay4_Cfan, LOW);
        Blynk.setProperty(Widget_LED_Cfan, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Cfan, "label", "เปิดพัดลมระบายอากาศ");
        LedBlynkCfan.on(); 
    }
    else
    {
        digitalWrite(Relay4_Cfan, HIGH);
        Blynk.setProperty(Widget_LED_Cfan, "label", "ปิดพัดลมระบายอากาศ");                       
        LedBlynkCfan.off();
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
  client.connect("Solar_Node2_OUT");
    
Blynk.run();
timer.run();
PZEM017DC();
}
