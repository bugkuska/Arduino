#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
BlynkTimer timer;
int blynkIsDownCount=0;

// Your WiFi credentials.
char ssid[] = "";                     //ชื่อ SSID ที่เราต้องการเชื่อมต่อ        
char pass[] = "";                         //รหัสผ่าน WI-FI
char auth[] = ""; //Auth token from blynk app 

bool Connected2Blynk = false;

//10A 4channel Relay extend 
#define pump            23            // Relay connect to pump 
#define ledbb           15            //Check blynk connected 
#define ledfan          25            //Cooling Fan

//10A Relay on board esp32all
#define Relay1_Valve1  27             //valve1
#define Relay2_Valve2  14             //valve2
#define Relay3_Valve3  12             //valve3
#define Relay4_Valve4  13             //valve4 



//RTC Widget
#include <TimeLib.h>
#include <WidgetRTC.h>
WidgetRTC rtc;

//DHT11
#include "DHT.h"
#define DHTTYPE DHT11   // DHT 11
#define DHTPin 16
DHT dht(DHTPin, DHTTYPE);

/////////////////////////////////////////Define Virtual Pin////////////////////////////////
//V2 ไฟสถานะปุ่ม Valve1
//V3 ปุ่ม เปิด-ปิด Valve1
//V6 ไฟสถานะปุ่ม Valve2
//V7 ปุ่ม เปิด-ปิด Valve2
//V10 ไฟสถานะปุ่ม Valve3
//V11 ปุ่ม เปิด-ปิด Valve3
//V14 ไฟสถานะปุ่ม Valve4
//V15 ปุ่ม เปิด-ปิด Valve4
//V18 ไฟสถานะปุ่ม pump
//V19 ปุ่ม เปิด-ปิด pump
//V20 Current Time
//V21 Current Date
//V22 ไฟสถานะปุ่ม พัดลมระบายอากาศ
//V23 ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
//V30 Humidity
//V31 Temperature
//V40 Time
//V41 Date
//////////////////////////////////////////Define Virtual Pin////////////////////////////////

//Valve1
#define Widget_LED_Valve1 V2              //ไฟสถานะปุ่ม Valve1
#define Widget_Btn_Valve1 V3              //ปุ่ม เปิด-ปิด Valve1
WidgetLED LedBlynkValve1(Widget_LED_Valve1);

//Valve2
#define Widget_LED_Valve2 V6              //ไฟสถานะปุ่ม Valve2
#define Widget_Btn_Valve2 V7              //ปุ่ม เปิด-ปิด Valve2
WidgetLED LedBlynkValve2(Widget_LED_Valve2);

//Valve3
#define Widget_LED_Valve3 V10             //ไฟสถานะปุ่ม Valve3
#define Widget_Btn_Valve3 V11             //ปุ่ม เปิด-ปิด Valve3
WidgetLED LedBlynkValve3(Widget_LED_Valve3);

//Valve4
#define Widget_LED_Valve4 V14             //ไฟสถานะปุ่ม Valve4
#define Widget_Btn_Valve4 V15             //ปุ่ม เปิด-ปิด Valve4
WidgetLED LedBlynkValve4(Widget_LED_Valve4);

//Pump
#define Widget_LED_Pump V18         //ไฟสถานะปุ่ม pump
#define Widget_Btn_Pump V19         //ปุ่ม เปิด-ปิด pump
WidgetLED LedBlynkPump(Widget_LED_Pump);

//Cooling Fan
#define Widget_LED_Fan V22        //ไฟสถานะปุ่ม พัดลมระบายอากาศ
#define Widget_Btn_Fan V23        //ปุ่ม เปิด-ปิด พัดลมระบายอากาศ
WidgetLED LedBlynkFan(Widget_LED_Fan);

//Setup
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  //Serial.println();
  //Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    delay(500); 
    
   
  // Setup Pin Mode
  pinMode(pump, OUTPUT);              // Pin 23 relay 10A in1 connect to Manatic + overload
  pinMode(ledbb,OUTPUT);              // Pin 27 relay 10A in2 connect to Green PoltLamp check blynk connected
  pinMode(ledfan,OUTPUT);             // Pin 26 relay 10A in3 connect to Red PoltLamp start cooling fan 
  pinMode(Relay1_Valve1,OUTPUT);      // Pin 19 relay 10A in1 valve1
  pinMode(Relay2_Valve2,OUTPUT);      // Pin 18 relay 10A in2 valve2
  pinMode(Relay3_Valve3,OUTPUT);      // Pin 5  relay 10A in3 valve3
  pinMode(Relay4_Valve4,OUTPUT);      // Pin 25 relay 10A in4 valve4
  
  // Set Defult Relay Status
  digitalWrite(pump, LOW);           // Pin 23 relay 10A in1 connect to Manatic + overload
  digitalWrite(ledbb,LOW);           // Pin 27 relay 10A in2 connect to Green PoltLamp check blynk connected
  digitalWrite(ledfan,LOW);          // Pin 26 relay 10A in3 connect to Red PoltLamp start cooling fan  
  digitalWrite(Relay1_Valve1,LOW);   // Pin 19 relay 10A in1 valve1
  digitalWrite(Relay2_Valve2,LOW);   // Pin 18 relay 10A in2 valve2
  digitalWrite(Relay3_Valve3,LOW);   // Pin 5  relay 10A in3 valve3
  digitalWrite(Relay4_Valve4,LOW);   // Pin 25 relay 10A in4 valve4

  //Begin read Humidity and Temperature ==> SHCT3
  dht.begin();

  //Begin Sync time
  rtc.begin();
    
  // Blynk.config(blynk_token);////เริ่มการเชื่อมต่อ Blynk Server แบบปกติ
   Blynk.begin(auth, ssid, pass);
    while (Blynk.connect() == false) {
        // Wait until Blynk is connected
    }
  
  timer.setInterval(10000L,dhtSensorData);  
  timer.setInterval(10000L, clockDisplay);
  timer.setInterval(10000L, reconnectblynk);  //Function reconnect  

}

//blynk conneted
BLYNK_CONNECTED()
{
 Blynk.syncAll();
 
 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(ledbb,HIGH);
    Serial.println("ledbb on");
 }
}

//Loop 
void loop() {     
  if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();//ให้เวลาของ Blynk ทำงาน
}

//Reconnect to blynk
void reconnectblynk()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    Blynk.syncAll();
    digitalWrite(ledbb, HIGH); //ledpin for check blynk connected 
  }
}

//Display Current Date/Time
void clockDisplay()
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  // Send time to the App
  Blynk.virtualWrite(V40, currentTime);
  // Send date to the App
  Blynk.virtualWrite(V41, currentDate);
}

//****BUTTON ON/OFF Valve1****
BLYNK_WRITE(Widget_Btn_Valve1){
int valueValve1 = param.asInt();
  if(valueValve1 == 1){
    digitalWrite(Relay1_Valve1, HIGH);
    Blynk.setProperty(Widget_LED_Valve1, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์วน้ำ");
    LedBlynkValve1.on();
    digitalWrite(pump, HIGH);
    Blynk.virtualWrite(Widget_Btn_Pump,1);
    Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
    LedBlynkPump.on(); 
    }
    else{              
    digitalWrite(Relay1_Valve1, LOW);
    Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์วน้ำเรียบร้อยแล้ว");
    LedBlynkValve1.off();     

    digitalWrite(pump, LOW);      
    Blynk.virtualWrite(Widget_Btn_Pump,0);                
    Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
    LedBlynkPump.off();           
    }
}
     
//****BUTTON ON/OFF Valve2****
BLYNK_WRITE(Widget_Btn_Valve2){
int valueValve2 = param.asInt();
  if(valueValve2 == 1){
    digitalWrite(Relay2_Valve2, HIGH);
    Blynk.setProperty(Widget_LED_Valve2, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Valve2, "label", "เปิดวาล์ว 2");
    LedBlynkValve2.on();

    digitalWrite(pump, HIGH);
    Blynk.virtualWrite(Widget_Btn_Pump,1);
    Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
    LedBlynkPump.on(); 
    }
    else{              
    digitalWrite(Relay2_Valve2, LOW);
    Blynk.setProperty(Widget_LED_Valve2, "label", "ปิดวาล์ว 2");
    LedBlynkValve2.off();     

    digitalWrite(pump, LOW);      
    Blynk.virtualWrite(Widget_Btn_Pump,0);                
    Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
    LedBlynkPump.off();           
    }
}
    
//****BUTTON ON/OFF Valve3****
BLYNK_WRITE(Widget_Btn_Valve3){
int valueValve3 = param.asInt();
  if(valueValve3 == 1){       
    digitalWrite(Relay3_Valve3, HIGH);
    Blynk.setProperty(Widget_LED_Valve3, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Valve3, "label", "เปิดวาล์ว 3");
    LedBlynkValve3.on();

    digitalWrite(pump, HIGH);
    Blynk.virtualWrite(Widget_Btn_Pump,1);
    Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
    LedBlynkPump.on(); 
    }
    else{              
    digitalWrite(Relay3_Valve3, LOW);
    Blynk.setProperty(Widget_LED_Valve3, "label", "ปิดวาล์ว 3");
    LedBlynkValve3.off();     

    digitalWrite(pump, LOW);      
    Blynk.virtualWrite(Widget_Btn_Pump,0);                
    Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
    LedBlynkPump.off();           
    }
}
      
//****BUTTON ON/OFF Valve4****
BLYNK_WRITE(Widget_Btn_Valve4){
int valueValve4 = param.asInt();
  if(valueValve4 == 1){      
    digitalWrite(Relay4_Valve4, HIGH);
    Blynk.setProperty(Widget_LED_Valve4, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Valve4, "label", "เปิดวาล์ว 3");
    LedBlynkValve4.on();

    digitalWrite(pump, HIGH);
    Blynk.virtualWrite(Widget_Btn_Pump,1);
    Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
    LedBlynkPump.on(); 
    }
    else{              
    digitalWrite(Relay4_Valve4, LOW);
    Blynk.setProperty(Widget_LED_Valve4, "label", "ปิดวาล์ว 4");
    LedBlynkValve4.off();     

    digitalWrite(pump, LOW);      
    Blynk.virtualWrite(Widget_Btn_Pump,0);                
    Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
    LedBlynkPump.off();           
    }
}

//****BUTTON ON/OFF Cooling FAN****
BLYNK_WRITE(Widget_Btn_Fan){
int valueFan = param.asInt();
  if(valueFan == 1){
    digitalWrite(ledfan, HIGH);
    Blynk.setProperty(Widget_LED_Fan, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Fan, "label", "เปิดพัดลมระบายอากาศ");
    LedBlynkFan.on();
    }
    else{              
    digitalWrite(ledfan, LOW);
    Blynk.virtualWrite(Widget_Btn_Fan, 0);
    Blynk.setProperty(Widget_LED_Fan, "label", "ปิดพัดลมระบายอากาศ");
    LedBlynkFan.off();                       
    }
}

//DHT11
void dhtSensorData(){
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)){ 
 Serial.println("Read from DHT Sensor");
  return;
  }    
Blynk.virtualWrite(V30, h);
Blynk.virtualWrite(V31, t);

    //******AUTO Cooling FAN*******
  if (t >= 33){       
    digitalWrite(ledfan, HIGH);
    Blynk.virtualWrite(Widget_Btn_Fan, 1);       
    Blynk.setProperty(Widget_LED_Fan, "color", "#00FF00");
    Blynk.setProperty(Widget_LED_Fan, "label", "พัดลมกำลังทำงาน");
    LedBlynkFan.on();
    }   

  if (t < 33){
    digitalWrite(ledfan, LOW);        
    Blynk.virtualWrite(Widget_Btn_Fan, 0);
    Blynk.setProperty(Widget_LED_Fan, "label", "ปิดพัดลมแล้ว");                        
    LedBlynkFan.off();  
    }       
}
