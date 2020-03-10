// NodeMCU + Blynk App Control Relay 4 Channel
#define BLYNK_PRINT Serial  
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
BlynkTimer timer;

//LCD
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

//DHT11
#include <DHT.h>
#define DHTPIN D7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

char ssid[] = ""; //ชื่อ SSID ที่เราต้องการเชื่อมต่อ        
char pass[] = ""; //รหัสผ่าน WI-FI
char auth[] = ""; //Auth token from blynk appl


int blynkIsDownCount=0;  
        
//ประกาศตัวแปร
const int Relay1 = D0;
const int Relay2 = D4;
const int Relay3 = D5;
const int Relay4 = D6;

void setup()
{
  Serial.begin(115200);  
   Serial.println("Starting...");
   WiFi.begin(ssid,pass);
   while (WiFi.status() != WL_CONNECTED)
   {
    delay(250);
    Serial.print(".");
   }
  Serial.println("WiFi Connected");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
  
// Setup Pin Mode
  pinMode(Relay1,OUTPUT); // NODEMCU PIN D0
  pinMode(Relay2,OUTPUT); // NODEMCU PIN D4
  pinMode(Relay3,OUTPUT); // NODEMCU PIN D5
  pinMode(Relay4,OUTPUT); // NODEMCU PIN D6

// Set Defult Relay Status
  digitalWrite(Relay1, HIGH);
  digitalWrite(Relay2, HIGH);
  digitalWrite(Relay3, HIGH);
  digitalWrite(Relay4, HIGH);

//Start read DHT11
  dht.begin();
      
  //Connect to Blynk Server
  Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  
  timer.setInterval(30000L, reconnecting);  
  timer.setInterval(1000L, dht11Sensor); 
  timer.setInterval(1000L,getSoilMoisterData);
}

//Relay1
BLYNK_WRITE(V10) //Blynk Virtual Pin V10 to Button 1 Control Relay 1
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay1, LOW);
      Serial.println("Relay 1  On");
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay1, HIGH);
      Serial.println("Relay 1  Off");
    }
}

//Relay2
BLYNK_WRITE(V11) //Blynk Virtual Pin V11 to Button 2 Control Relay 2 
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay2, LOW);
      Serial.println("Relay 2  On");
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay2, HIGH);
      Serial.println("Relay 2  Off");
    }
}

//Relay3
BLYNK_WRITE(V12) //Blynk Virtual Pin V12 to Button 3 Control Relay 3
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay3, LOW);
      Serial.println("Relay 3  On");
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay3, HIGH);
      Serial.println("Relay 3  Off");
    }
}

//Relay4
BLYNK_WRITE(V13) //Blynk Virtual Pin V13 to Button 4 Control Relay 4
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay4, LOW);
      Serial.println("Relay 4  On");
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay4, HIGH);
      Serial.println("Relay 4  Off");
    }
}
//End Relay control

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
  //Blynk.syncAll();
  Serial.println(".");//per debug
  Blynk.syncVirtual(V10);
  Blynk.syncVirtual(V11);
  Blynk.syncVirtual(V12);
  Blynk.syncVirtual(V13);
}

void reconnecting()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    //Blynk.syncAll();
  Blynk.syncVirtual(V10);
  Blynk.syncVirtual(V11);
  Blynk.syncVirtual(V12);
  Blynk.syncVirtual(V13);
  }
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
  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);

  //LCD
      lcd.begin();
      lcd.backlight();                                   
      lcd.setCursor(1, 0); 
      lcd.print("TEMP =  "); 
      lcd.print(t);
      lcd.print("  C ");
      
      lcd.setCursor(1, 1);
      lcd.print("HUMI =  "); 
      lcd.print(h);
      lcd.print("  % ");
 }

 void getSoilMoisterData(){
  float moisture_percentage;
  int sensor_analog;
  sensor_analog = analogRead(A0);
  Serial.print("Law Soil data:");
  Serial.println(sensor_analog);
  moisture_percentage = ( 100 - ( (sensor_analog/1024.00) * 100 ) );
  Blynk.virtualWrite(V0,(moisture_percentage));
  Serial.print("Moisture Percentage = ");
  Serial.print(moisture_percentage);
  Serial.print("%\n\n");
  delay(1000);
}
