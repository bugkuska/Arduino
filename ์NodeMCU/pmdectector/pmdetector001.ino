#include <FS.h>                   //this needs to be first, or it all crashes and burns...
//#define BLYNK_DEBUG
#define BLYNK_PRINT Serial        // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <BlynkSimpleEsp8266.h>   //https://github.com/blynkkk/blynk-library
#include <DNSServer.h>            //https://github.com/esp8266/Arduino
#include <ESP8266WebServer.h>     //https://github.com/esp8266/Arduino
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <SimpleTimer.h>          //https://github.com/jfturcot/SimpleTimer
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

//Line Notify
#include <TridentTD_LineNotify.h>
//#define LINE_TOKEN  ""
#define LINE_TOKEN  ""

//DHT
#include <DHT.h>                  //https://github.com/adafruit/DHT-sensor-library
#define DHTPIN D7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

int blynkIsDownCount=0;

#include <SimpleTimer.h>
BlynkTimer timer;

//new
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// Set the LCD address to 0x27 or 0x3F for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);

//#include <SPI.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
//Adafruit_SSD1306 OLED(-1);
//
#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

int PM01Value=0;          //define PM1.0 value of the air detector module
int PM2_5Value=0;         //define PM2.5 value of the air detector module
int PM10Value=0;         //define PM10 value of the air detector module

//define your default values here, if there are different values in config.json, they are overwritten.
//char mqtt_server[40];
//char mqtt_port[6] = "8080";
char blynk_token[34] = "";

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

//Relay
const int Relay1 = D4;
const int Relay2 = D5;
const int Relay3 = D6;

//BlynkConnectted
const int ledblynk = D3;

//Led status
boolean stateled1=0;
boolean prevStateled1 = 0;
boolean stateled2=0;
boolean prevStateled2 = 0;
boolean stateled3=0;
boolean prevStateled3 = 0;
//bool swState = false;

void setup()
{
  Serial.begin(9600);   
  Serial.setTimeout(1500); 
  //new
  //OLED.begin(SSD1306_SWITCHCAPVCC,0x3C);    


 
  //new
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
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read  

  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 32);

  //WiFiManager
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  
  //add all your parameters here
  //wifiManager.addParameter(&custom_mqtt_server);
  //wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_blynk_token);

  //reset settings - for testing
 //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("pmDetecter_node01", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

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

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  
  //LineNotify
  LINE.setToken(LINE_TOKEN);  // กำหนด Line Token

  // Setup Pin Mode
  pinMode(Relay1,OUTPUT); // NODEMCU PIN D4
  pinMode(Relay2,OUTPUT); // NODEMCU PIN D5
  pinMode(Relay3,OUTPUT); // NODEMCU PIN D6
  pinMode(ledblynk,OUTPUT); // NODEMCU PIN D3

  // Set Defult Relay Status
  digitalWrite(Relay1, HIGH);
  digitalWrite(Relay2, HIGH);
  digitalWrite(Relay3, HIGH);
  //digitalWrite(Relay4, HIGH);
  digitalWrite(ledblynk, HIGH);

  //Start read DHT11
  dht.begin();
  
  Blynk.config(blynk_token);
  timer.setInterval(1000L, checkledstate);
  timer.setInterval(1000L, reconnecting);  
  timer.setInterval(1000L, dht11Sensor); 
}

//Relay control
BLYNK_WRITE(V10) //Blynk Virtual Pin V10 to Button 1 Control Relay 1
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay1, LOW);
      Serial.println("Relay 1 On");   
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay1, HIGH);
      Serial.println("Relay 1 Off");    
    }
}
BLYNK_WRITE(V11) //Blynk Virtual Pin V11 to Button 2 Control Relay 2 
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay2, LOW);
      Serial.println("Relay 2 On");  
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay2, HIGH);
      Serial.println("Relay 2 Off");    
    }
}
BLYNK_WRITE(V12) //Blynk Virtual Pin V12 to Button 3 Control Relay 3
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay3, LOW);
      Serial.println("Relay 3 On");     
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay3, HIGH);
      Serial.println("Relay 3 Off");    
    }
}


void loop()
{
if (Blynk.connected())
  {
    Blynk.run();
  }
   timer.run();  

  if(Serial.find(0x42)){    //start to read when detect 0x42
    Serial.readBytes(buf,LENG);

    if(buf[0] == 0x4d){
      if(checkValue(buf,LENG)){
        PM01Value=transmitPM01(buf); //count PM1.0 value of the air detector module
        PM2_5Value=transmitPM2_5(buf);//count PM2.5 value of the air detector module
        PM10Value=transmitPM10(buf); //count PM10 value of the air detector module 
      }           
    } 
  }

//static unsigned long lcdTimer=millis();  
 //   if (millis() - lcdTimer >=1000) 
    {
 //     lcdTimer=millis(); 
      Serial.print("PM1.0: ");  
      Serial.print(PM01Value);
      Serial.println("  ug/m3");            
    
      Serial.print("PM2.5: ");  
      Serial.print(PM2_5Value);
      Serial.println("  ug/m3");     
      
      Serial.print("PM1 0: ");  
      Serial.print(PM10Value);
      Serial.println("  ug/m3");   
      Serial.println();

      
      // initialize the LCD
      lcd.begin();
     // lcd.backlight();                               
      lcd.setCursor(3, 0); 
      lcd.println("  Dust Sensor  ");
      
      lcd.setCursor(0,1);
      lcd.print("   PM1.0 =   ");
      lcd.print(PM01Value);
      lcd.println(" ug/m3");

      lcd.setCursor(0,2);
      lcd.print("   PM2.5 =   ");
      lcd.print(PM2_5Value);
      lcd.println(" ug/m3");
   
      lcd.setCursor(0,3);
      lcd.print("   PM1 0 =   ");
      lcd.print(PM10Value);
      lcd.println(" ug/m3");
 
      //lcd.display(); // แสดงตัวอักษรทั้งหมด
  
      Blynk.virtualWrite(V0, PM01Value);      
      Blynk.virtualWrite(V1, PM2_5Value); 
      Blynk.virtualWrite(V2, PM10Value);
       
      if (PM2_5Value>100){
      Blynk.virtualWrite(V10, 0);
      digitalWrite(Relay1, LOW);
      Serial.println("Relay1 ON");
      LINE.notify("ค่า PM 2.5 = "+String(PM2_5Value)+" เกิน ทำการพ่นละอองน้ำ"); 
      }
      if (PM2_5Value<100){
      Blynk.virtualWrite(V10, 1);
      digitalWrite(Relay1, HIGH);
      Serial.println("Relay1 OFF");
      }   
    }
  }

BLYNK_CONNECTED()
{
  Serial.println(".");//per debug
  Blynk.syncVirtual(V10);
  Blynk.syncVirtual(V11);
  Blynk.syncVirtual(V12);
  digitalWrite(ledblynk, LOW); //ledpin for check blynk connected
  Serial.println("Blynk Connected");
}

void reconnecting()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    Blynk.syncVirtual(V10);
    Blynk.syncVirtual(V11);
    Blynk.syncVirtual(V12);
    digitalWrite(ledblynk, LOW); //ledpin for check blynk connected
  }
}

// Check Status LED Widget
void checkledstate()
{
stateled1=digitalRead(Relay1); // V10 Pin D4 Control Relay 1
  if (stateled1 != prevStateled1)
  {
    if (stateled1==0) Blynk.virtualWrite(V17,255); 
    if (stateled1==1) Blynk.virtualWrite(V17,0); 
  }  
  prevStateled1=stateled1;

stateled2=digitalRead(Relay2); // V11 Pin D5 Control Relay 2
  if (stateled2 != prevStateled2)
  {
    if (stateled2==0) Blynk.virtualWrite(V18,255); 
    if (stateled2==1) Blynk.virtualWrite(V18,0); 
  }  
  prevStateled2=stateled2;

stateled3=digitalRead(Relay3); // V12 Pin D6 Control Relay 3
  if (stateled3 != prevStateled3)
  {
    if (stateled3==0) Blynk.virtualWrite(V19,255); 
    if (stateled3==1) Blynk.virtualWrite(V19,0); 
  }  
  prevStateled3=stateled3;
}

 void dht11Sensor(){
 float h = dht.readHumidity();
 float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)){ 
  Serial.println("Read from DHT Sensor");
  return;
  }
  Serial.print("Humidity :");
  Serial.println(h);
  Serial.print("Temperature :");
  Serial.println(t);
      
  Blynk.virtualWrite(V3, h);
  Blynk.virtualWrite(V4, t);
 /* if(t>50){
   //Blynk.virtualWrite(V15,0);
   //Blynk.virtualWrite(V17,255); 
   //digitalWrite(Relay3, LOW);
   //Serial.println("Relay 3 On");
   //LINE.notify("อุณหภูมิขณะนี้เกิน "+String(t)+" องศา"); 
   //Blynk.email("bugkuska@gmail.com", "ESP8622 Alert", "Alert");
   //Blynk.notify("ESP8266 Alert - แจ้งเตือน อุณภูมิเกิน 35 องศาเซลเซียล ");
  }
  //if(t<35){
   //Blynk.virtualWrite(V15,1);
   //Blynk.virtualWrite(V17,0); 
   //digitalWrite(Relay3, HIGH);
   //Serial.println("Relay 3 Off");
   //Blynk.email("bugkuska@gmail.com", "ESP8622 Alert", "No Alert");
   //Blynk.notify("ESP8266 Alert - แจ้งเตือน อุณภูมิเกิน 35 องศาเซลเซียล ");
//}*/
}

char checkValue(unsigned char *thebuf, char leng)
{  
  char receiveflag=0;
  int receiveSum=0;
  for(int i=0; i<(leng-2); i++){
  receiveSum=receiveSum+thebuf[i];
  }
  receiveSum=receiveSum + 0x42;
 
  if(receiveSum == ((thebuf[leng-2]<<8)+thebuf[leng-1]))  //check the serial data 
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}
int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val=((thebuf[3]<<8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}
//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val=((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
  }
//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val=((thebuf[7]<<8) + thebuf[8]); //count PM10 value of the air detector module  
  return PM10Val;
}
