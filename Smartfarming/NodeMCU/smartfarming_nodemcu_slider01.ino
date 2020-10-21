/*
Hardware
- NodeMCU V3 With Nodebase
- Relay 4 CH
- Relay 2 CH
- DHT11
- Soil Moisture 
Software
- Arduino IDE
- Blynk 
*/
/*
ความสามารถของระบบ
- ควบคุมปั้มน้ำ AC/DC ไม่เกิน 3 HP 
- ควบคุมวาล์ว 1 โซน
- เลือกโหมดการทำงาน Auto/Manual
- ตั้งเวลาเปิด-ปิด ปั้มน้ำ วาล์วน้ำ
- ระบบระบายความร้อนภายในตู้ควบคุม เลือกโหมดการทำงาน Auto/Manual
*/

#define BLYNK_PRINT Serial  
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = ""; //ชื่อ SSID ที่เราต้องการเชื่อมต่อ        
char pass[] = ""; //รหัสผ่าน WI-FI
char auth[] = ""; //Auth token from blynk app

BlynkTimer timer;

//RTC Widget
#include <TimeLib.h>
#include <WidgetRTC.h>
WidgetRTC rtc;

//Valve1
//V11 Auto&Manual valve1
//V12 Slider valve1
//Manual & Auto Switch 1
//V0 Soil Moisture
#define Widget_LED_Valve1 V1        //ไฟสถานะปุ่ม Valve1
#define Widget_Btn_Valve1 V2         //ปุ่ม เปิด-ปิด Valve1
WidgetLED LedBlynkValve1(Widget_LED_Valve1);

//Cooling Fan
//V13 Auto&Manual Cfan
//V14 Slider Cfan
#define Widget_LED_cfan V3        //ไฟสถานะปุ่ม Valve2
#define Widget_Btn_cfan V4         //ปุ่ม เปิด-ปิด Valve2
WidgetLED LedBlynkcfan(Widget_LED_cfan);

//Pump
#define Widget_LED_Pump V5        //ไฟสถานะปุ่ม pump
#define Widget_Btn_Pump V6         //ปุ่ม เปิด-ปิด pump
WidgetLED LedBlynkPump(Widget_LED_Pump);

//V7 Temperature1
//V8 Humidity1

//DHT11-01
#include <DHT.h>
#define DHTPIN1 D1
#define DHTTYPE1 DHT11
DHT dht1(DHTPIN1, DHTTYPE1);

int blynkIsDownCount=0;

//Slider for set SoilMoisture limit
bool switchStatus1 = 0; // 0 = manual,1=auto
int soilMoistureLimit1 = 0;
bool manualSwitch1 = 0;

//Slider for set temperature limit
bool switchStatus2 = 0; // 0 = manual,1=auto
int templimit = 0;
bool manualSwitch2 = 0;

//Define variable and pin connected to MCU
const int pump              = D0; // Relay5 connect to pump
const int ledblynk          = D2; //Check blynk connected
const int Relay1_Valve1     = D4;   //Relay1_Valve1
const int Relay2_Valve2     = D5;   //Relay2_Valve2
const int Relay3_Fan1       = D6;   //C-Fan1

void setup()
{
  // put your setup code here, to run once:
  
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
  pinMode(pump, OUTPUT);              // Nodemcu pin D0 connect to relay 10A in1
  pinMode(ledblynk,OUTPUT);           // Nodemcu pin D2 connect to relay 10A in2
  pinMode(Relay1_Valve1,OUTPUT);      // NODEMCU PIN D4 connect to relay 10A in1
  pinMode(Relay2_Valve2,OUTPUT);      // NODEMCU PIN D5 connect to relay 10A in2
  pinMode(Relay3_Fan1,OUTPUT);        // NODEMCU PIN D6 connect to relay 10A in3
  
  // Set Defult Relay Status
  digitalWrite(pump, HIGH);      //Nodemcu pin D0 connect to relay 10A in1
  digitalWrite(ledblynk,HIGH);         // Nodemcu pin D2 connect to relay 10A in2 
  digitalWrite(Relay1_Valve1,HIGH);    // NODEMCU PIN D4 connect to relay 10A in1
  digitalWrite(Relay2_Valve2,HIGH);    // NODEMCU PIN D5 connect to relay 10A in2
  digitalWrite(Relay3_Fan1,HIGH);    // NODEMCU PIN D6 connect to relay 10A in3
 
  //Start read DHT11-01
  dht1.begin();  //เริ่มอ่านข้อมูล DHT Sensor
  
  //Connect to Blynk Server
  Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);  
  timer.setInterval(5000L, dht1SensorData); //Send sensor data to display on app blynk
  timer.setInterval(5000L, getSoilMoisterData);
  timer.setInterval(10000L, clockDisplay);  
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
  }
   timer.run();  
}//loop end

BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    Blynk.syncAll();

    // Synchronize time on connection
    rtc.begin();

  if (Blynk.connected())
    {
    digitalWrite(ledblynk, LOW); //ledpin for check blynk connected
    Serial.println("Blynk Connected");
    }
}

// update switchStatus2 on Temperature
BLYNK_WRITE(V13)
{   
  switchStatus2 = param.asInt(); // Get value as integer
}

// update temperture setting
BLYNK_WRITE(V14)
{   
  templimit = param.asInt(); // Get value as integer
}

// update manualSwitch2
BLYNK_WRITE(V4)
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
  Blynk.virtualWrite(V7, t1);
  Blynk.virtualWrite(V8, h1);
  
  if(switchStatus2)
  {
    // auto
    if(t1 > templimit)
    {
        digitalWrite(Relay3_Fan1, LOW);  
        Blynk.virtualWrite(V4, 1);
        Blynk.setProperty(Widget_LED_cfan, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_cfan, "label", "เปิดพัดลมระบายอากาศ");
        LedBlynkcfan.on(); 

    }  
    else
    {
        digitalWrite(Relay3_Fan1, HIGH);
        Blynk.virtualWrite(V4, 0);
        Blynk.virtualWrite(Widget_LED_cfan, 0);
        Blynk.setProperty(Widget_LED_cfan, "label", "ปิดพัดลมระบายอากาศ");                       
        LedBlynkcfan.off();  
    }
  }
  else
  {
    if(manualSwitch2)
    {
        digitalWrite(Relay3_Fan1, LOW);
        Blynk.setProperty(Widget_LED_cfan, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_cfan, "label", "เปิดพัดลมระบายอากาศ");
        LedBlynkcfan.on(); 
    }
    else
    {
        digitalWrite(Relay3_Fan1, HIGH);
        Blynk.setProperty(Widget_LED_cfan, "label", "ปิดพัดลมระบายอากาศ");                       
        LedBlynkcfan.off();
    }
    // manaul
  }
}

// update switchStatus1 on Valve1
BLYNK_WRITE(V11)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}

// update soilMosture setting
BLYNK_WRITE(V12)
{   
  soilMoistureLimit1 = param.asInt(); // Get value as integer
}

// update manualSwitch
BLYNK_WRITE(V2)
{
  manualSwitch1 = param.asInt();
}


//SoilMoisture Sensor
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

if(switchStatus1)
  {
    // auto
    if(moisture_percentage < soilMoistureLimit1)
    {
        digitalWrite(Relay1_Valve1, LOW);  
        Blynk.virtualWrite(V2, 1);
                
        Blynk.setProperty(Widget_LED_Valve1, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์ว 1");
        LedBlynkValve1.on(); 

        digitalWrite(pump,LOW);
        Blynk.virtualWrite(V5, 1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Pump, "label", "เปิดปั้มน้ำ");
        LedBlynkPump.on();
    }  
    else
    {
        digitalWrite(Relay1_Valve1, HIGH);
        Blynk.virtualWrite(V2, 0);
        Blynk.virtualWrite(Widget_LED_Valve1, 0);
        Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์ว 1");                       
        LedBlynkValve1.off();  

        digitalWrite(pump,HIGH);
        Blynk.virtualWrite(V5, 0);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำ");
        LedBlynkPump.off();
    }
  }
  else
  {
    if(manualSwitch1)
    {
        digitalWrite(Relay1_Valve1, LOW);        
        Blynk.setProperty(Widget_LED_Valve1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์ว 1");
        LedBlynkValve1.on(); 

        digitalWrite(pump,LOW);
        Blynk.virtualWrite(V5, 1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "เปิดปั้มน้ำ");
        LedBlynkPump.on();
        
    }
    else
    {
        digitalWrite(Relay1_Valve1, HIGH);
        Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์ว 1");                       
        LedBlynkValve1.off();

        digitalWrite(pump,HIGH);
        Blynk.virtualWrite(V5, 0);
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำ");
        LedBlynkPump.off();
        
    }
    // manaul
  }
}

//Display Current Date/Time
void clockDisplay()
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  Blynk.virtualWrite(V9, currentTime);
  Blynk.virtualWrite(V10, currentDate);
}
