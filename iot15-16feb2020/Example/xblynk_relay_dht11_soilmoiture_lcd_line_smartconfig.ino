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

//ThingSpeak
#include <ThingspeakClient.h>
WiFiClient client;
ThingspeakClient thingspeak;
const char* host = "api.thingspeak.com";
const String THINGSPEAK_CHANNEL_ID = "";
const String THINGSPEAK_API_READ_KEY = "";  //read key
const char* api   = "";  //write key
unsigned long currentMillis,previousMillis3, readMillis = 0;

//Line Notify
#include <TridentTD_LineNotify.h>
#define LINE_TOKEN  ""
uint8_t lastLDRMsg = 0;   //LDR
uint8_t lastLineMsg = 0;  //Temperature

//DHT11
#include <DHT.h>
#define DHTPIN D7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

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

unsigned long lastMillis = 0;

//Relay
const int ledblynk = D3; //BlynkConnectted
const int ledfan = D4;  //Cooling Fan start
const int Relay1 = D5;  //Relay1
const int Relay2 = D6;  //Relay2

//Led status
boolean stateled1=0;
boolean prevStateled1 = 0;
boolean stateled2=0;
boolean prevStateled2 = 0;

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

  //WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  //WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
    WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

  //wifiManager.resetSettings();
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
  if (!wifiManager.autoConnect("DSD5_KORAT", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  //if you get here you have connected to the WiFi
  Serial.println("connected........:)");
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

  dht.begin();  //เริ่มอ่านข้อมูล DHT Sensor
  
  //Blynk.begin(auth, ssid, pass);
  Blynk.config(blynk_token);
  //Blynk.config(blynk_token, "103.253.73.204", 8080); 
  timer.setInterval(1000L, UpdateDHT11Thingspeak);
  timer.setInterval(1000L, dht11SensorData); //Send sensor data to display on app blynk
  timer.setInterval(1000L, getldrSensorData);
  timer.setInterval(1000L, UpdateldrThingspeak); 
  timer.setInterval(30000L, reconnecting);  //Function reconnect

  // Setup Pin Mode
  pinMode(ledblynk,OUTPUT); // NODEMCU PIN D3
  pinMode(Relay1,OUTPUT);   // NODEMCU PIN D5
  pinMode(Relay2,OUTPUT);   // NODEMCU PIN D6
  pinMode(ledfan,OUTPUT);   // NODEMCU PIN D4

  // Set Defult Relay Status
  digitalWrite(Relay1, HIGH);
  digitalWrite(Relay2, HIGH);
  digitalWrite(ledblynk, HIGH);
  digitalWrite(ledfan, HIGH);
}

BLYNK_WRITE(V10) //Blynk Virtual Pin V10 to Button 1 Control Relay 1
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay1, LOW);
      Serial.println("Relay 1 On");   
      Blynk.virtualWrite(V13,255); 
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay1, HIGH);
      Serial.println("Relay 1 Off");    
       Blynk.virtualWrite(V13,0); 
    }
}
BLYNK_WRITE(V11) //Blynk Virtual Pin V11 to Button 2 Control Relay 2 
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay2, LOW);
      Serial.println("Relay 2 On");  
      Blynk.virtualWrite(V14,255);
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay2, HIGH);
      Serial.println("Relay 2 Off");    
      Blynk.virtualWrite(V14,0);
    }
}

void loop()
{
  if (Blynk.connected())
  {
    Blynk.run();
  }
   timer.run();  
}

BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    Blynk.syncVirtual(V10);
    Blynk.syncVirtual(V11);
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
    digitalWrite(ledblynk, LOW); //ledpin for check blynk connected
  }
}

void dht11SensorData(){
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)){ 
  Serial.println("Read from DHT Sensor");
  return;
  }
  Serial.print("Temperature");
  Serial.println(t);
   Serial.print("Humidity");
  Serial.println(h);

  //LCD
      lcd.begin();
      lcd.backlight();                               

      lcd.setCursor(2, 0);
      lcd.print("DSD5 KORAT");
      
      lcd.setCursor(2, 1); 
      lcd.print("TEMP =  "); 
      lcd.print(t);
      lcd.print("  C ");
      
      lcd.setCursor(2, 2);
      lcd.print("HUMI =  "); 
      lcd.print(h);
      lcd.print("  % ");
  
  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);
  /*if(t>35){
   digitalWrite(ledfan, LOW);
  }
  if(t<35){
    digitalWrite(ledfan,HIGH);
  }*/


/*
   if((t>35)&&(lastLineMsg != 1)){
    digitalWrite(ledfan, LOW);//เปิดพัดลมระบายความร้อนภายในตู้ควบคุม  
    LINE.notify("อุณหภูมิแถวโต๊ะนั่งทานข้าว "+String(t)+" องศา"+"เกินกว่าเกณฑ์ที่กำหนด"); 
    lastLineMsg = 1;
  }
  if((t<35)&&(lastLineMsg != 2)){
   digitalWrite(ledfan, HIGH); //ปิด
   LINE.notify("อุณหภูมิแถวโต๊ะนั่งทานข้าว "+String(t)+" องศา" +"อยู่ในเกณฑ์ปกติ"); 
   lastLineMsg = 2;
   }

  if((t<20)&& (lastLineMsg != 3)){
   LINE.notify("อุณหภูมิแถวโต๊ะนั่งทานข้าว "+String(t)+" องศา" +"เฮ้ย หนาวโว้ย...."); 
   lastLineMsg = 3;
  }
  */
}

void UpdateDHT11Thingspeak() {
  previousMillis3 = millis();
  const int httpPort = 80;
 
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) 
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }  
  if (!client.connect(host, httpPort)) {
   Serial.println("connection failed");
    return;
  }  
  // Create a URI for the request
  String url = "/update?api_key=";
  url += api;
  url +="&field1=";
  url += String(t);
  url +="&field2=";
  url += String(h);
       
  Serial.print("Requesting URL: ");
  Serial.println(url);
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  delay(10);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection");
}

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

  /*
    if((ldr_percentage<=50)&&(lastLDRMsg != 1)){
      //digitalWrite(Relay1, LOW);//เปิด  
      LINE.notify("ค่าแสงตอนนี้ "+String(ldr_percentage)+" เปอร์เซ็นต์"+"=="+"เริ่มมืดแล้ว ควรเปิดไฟ"); 
      lastLDRMsg = 1;
      }
    if((ldr_percentage>50)&&(lastLDRMsg != 2)){
     //digitalWrite(Relay1, HIGH); //ปิด
     LINE.notify("ค่าแสงตอนนี้ "+String(ldr_percentage)+" เปอร์เซ็นต์"+"=="+"สว่างแล้ว ควรปิดไฟ");
     lastLDRMsg = 2;
     }
      */
      
     Serial.println();
     delay(3000);
  }

void UpdateldrThingspeak()
{previousMillis3 = millis();
  const int httpPort = 80;
 
  float ldr_percentage;
  int sensor_analog;
  sensor_analog = analogRead(A0);
  ldr_percentage = ( 100 - ( (sensor_analog/1024.00) * 100 ) ); 
  if (isnan(sensor_analog)) 
  {
    Serial.println("Failed to read from Soil sensor!");
    return;
  }  
  if (!client.connect(host, httpPort)) {
   Serial.println("connection failed");
    return;
  }  
  
  // Create a URI for the request
  String url = "/update?api_key=";
  url += api;
  url +="&field3=";
  url += String(ldr_percentage);
 
  Serial.print("Requesting URL: ");
  Serial.println(url);
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  delay(10);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection");
}

// Check Status LED Widget
void checkledstate()
{
stateled1=digitalRead(Relay1); // V10 Pin D1 Control Relay 1
  if (stateled1 != prevStateled1)
  {
    if (stateled1==0) Blynk.virtualWrite(V17,255); 
    if (stateled1==1) Blynk.virtualWrite(V17,0); 
  }  
  prevStateled1=stateled1;

stateled2=digitalRead(Relay2); // V11 Pin D2 Control Relay 2
  if (stateled2 != prevStateled2)
  {
    if (stateled2==0) Blynk.virtualWrite(V18,255); 
    if (stateled2==1) Blynk.virtualWrite(V18,0); 
  }  
  prevStateled2=stateled2;
}
