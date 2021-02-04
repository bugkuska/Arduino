#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
BlynkTimer timer;
int blynkIsDownCount = 0;

char ssid[] = "";                         //ชื่อ SSID ที่เราต้องการเชื่อมต่อ        
char pass[] = "";                         //รหัสผ่าน WI-FI
char auth[] = "";                         //Auth token from blynk app 

//Modbus
#define RXD2            16
#define TXD2            17
HardwareSerial rs485(1);
#include "modbusRTU.h"

//Define pin for clear and config AP&Token
#define AP_Config       14

//Built-in relay
#define relay1  19
#define relay2  18
#define relay3  5
#define relay4  25

//10A 4channel Relay extend 
#define pump            23        // Relay connect to pump 
#define ledbb           27        //Check blynk connected 
#define ledfan          26        //Cooling Fan
//2

//Define pin for analog input
#define INPUT_1_A1 39
#define INPUT_2_A2 34
#define INPUT_3_A3 35
#define INPUT_4_A4 32

//RTC Widget
#include <TimeLib.h>
#include <WidgetRTC.h>
WidgetRTC rtc;

//OLED Display
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
SSD1306Wire  display(0x3c, 21, 22);

//SHCT3 Humidity and Temperature Sensor
#include <Arduino.h>
#include "SHTC3.h"
SHTC3 s(Wire);
/*
//MQTT
#include <PubSubClient.h>
//const char* mqtt_server = "35.185.187.230";
//Mqtt Broker settings
const char* mqtt_server = "35.185.187.230";
//const char* mqttPort = 1883;
const char* mqttUser = "admin";
const char* mqttPassword = "8owmp";

WiFiClient esp32_modbus01;
PubSubClient client(esp32_modbus01);
long lastMsg1 = 0;
long lastMsg2 = 0;
long lastMsg3 = 0;
long lastMsg4 = 0;
*/

/////////////////////////////////////////Define Virtual Pin////////////////////////////////
//V2 ไฟสถานะปุ่ม Valve1
//V3 ปุ่ม เปิด-ปิด Valve1
//V4 ไฟสถานะปุ่ม Valve2
//V5 ปุ่ม เปิด-ปิด Valve2
//V6 ไฟสถานะปุ่ม Valve3
//V7 ปุ่ม เปิด-ปิด Valve3
//V8 ไฟสถานะปุ่ม Valve4
//V9 ปุ่ม เปิด-ปิด Valve4
//V10 ไฟสถานะปุ่ม Valve5
//V11 ปุ่ม เปิด-ปิด Valve5
//V12 ไฟสถานะปุ่ม Valve6
//V13 ปุ่ม เปิด-ปิด Valve6
//V14 ไฟสถานะปุ่ม Valve7
//V15 ปุ่ม เปิด-ปิด Valve7
//V16 ไฟสถานะปุ่ม Valve8
//V17 ปุ่ม เปิด-ปิด Valve8

//V30 modbus_temp1 
//V31 modbus_humi1
//V32 modbus_temp2 
//V33 modbus_humi2

//V40 Time
//V41 Date
//////////////////////////////////////////Define Virtual Pin////////////////////////////////

//Valve1
#define Widget_LED_Valve1 V2              //ไฟสถานะปุ่ม Valve1
#define Widget_Btn_Valve1 V3              //ปุ่ม เปิด-ปิด Valve1
WidgetLED LedBlynkValve1(Widget_LED_Valve1);

//Valve2
#define Widget_LED_Valve2 V4              //ไฟสถานะปุ่ม Valve2
#define Widget_Btn_Valve2 V5              //ปุ่ม เปิด-ปิด Valve2
WidgetLED LedBlynkValve2(Widget_LED_Valve2);

//Valve3
#define Widget_LED_Valve3 V6             //ไฟสถานะปุ่ม Valve3
#define Widget_Btn_Valve3 V7             //ปุ่ม เปิด-ปิด Valve3
WidgetLED LedBlynkValve3(Widget_LED_Valve3);

//Valve4
#define Widget_LED_Valve4 V8             //ไฟสถานะปุ่ม Valve4
#define Widget_Btn_Valve4 V9             //ปุ่ม เปิด-ปิด Valve4
WidgetLED LedBlynkValve4(Widget_LED_Valve4);

//Valve5
#define Widget_LED_Valve5 V10              //ไฟสถานะปุ่ม Valve5
#define Widget_Btn_Valve5 V11              //ปุ่ม เปิด-ปิด Valve5
WidgetLED LedBlynkValve5(Widget_LED_Valve5);

//Valve6
#define Widget_LED_Valve6 V12              //ไฟสถานะปุ่ม Valve6
#define Widget_Btn_Valve6 V13              //ปุ่ม เปิด-ปิด Valve6
WidgetLED LedBlynkValve6(Widget_LED_Valve6);

//Valve7
#define Widget_LED_Valve7 V14             //ไฟสถานะปุ่ม Valve7
#define Widget_Btn_Valve7 V15             //ปุ่ม เปิด-ปิด Valve7
WidgetLED LedBlynkValve7(Widget_LED_Valve7);

//Valve8
#define Widget_LED_Valve8 V16             //ไฟสถานะปุ่ม Valve8
#define Widget_Btn_Valve8 V17             //ปุ่ม เปิด-ปิด Valve8
WidgetLED LedBlynkValve8(Widget_LED_Valve8);

//Pump
#define Widget_LED_Pump V18         //ไฟสถานะปุ่ม pump
#define Widget_Btn_Pump V19         //ปุ่ม เปิด-ปิด pump
WidgetLED LedBlynkPump(Widget_LED_Pump);

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

  //OLED
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
  
   // Setup Pin Mode
    pinMode(pump, OUTPUT);                // Pin 2 relay 10A in1 connect to Manatic + overload
    pinMode(ledbb,OUTPUT);                // Pin 26 relay 10A in2 connect to Green PoltLamp check blynk connected
    pinMode(ledfan,OUTPUT);               // Pin 27 relay 10A in3 connect to Red PoltLamp start cooling fan 
    pinMode(relay1,OUTPUT);
    
  
  // Set Defult Relay Status
    digitalWrite(pump, HIGH);             //Nodemcu pin D0 connect to relay 30A in1
    digitalWrite(ledbb,HIGH);             // Nodemcu pin D3 connect to relay 30A in2
    digitalWrite(ledfan,HIGH);            // Nodemcu pin D3 connect to relay 30A in2
    digitalWrite(relay1,HIGH);
    
    //dht.begin();
    
    Wire.begin();
    rtc.begin();
     //MQTT
  // client.setServer(mqtt_server, 1883);
   //PubSubClient client(server, 1883, mqttUser, mqttPassword);
  //Connect to Blynk Server
  //Blynk.begin(auth,ssid,pass, "103.253.73.204", 8080); 
   Blynk.begin(auth, ssid, pass);
    //timer.setInterval(30000L, dht11SensorData); 
    timer.setInterval(10000L,wtr10e_node3);    //Read Modbus SHT20 Sensor1
    timer.setInterval(10000L,wtr10e_node4);    //Read Modbus SHT20 Sensor2
    timer.setInterval(10000L, clockDisplay);  //Display Date/Time
    timer.setInterval(10000L, reconnectblynk);  //Function reconnect blynk  
}

//blynk conneted
BLYNK_CONNECTED()
{
 Blynk.syncAll();
 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(ledbb,LOW);
    Serial.println("ledbb on");
 }
}
   
//Loop 
void loop() {    
  display.display();

/*
  //MQTT
  if (!client.connected()) {
    reconnectmqtt();
  }
  //client.loop();
  if(!client.loop())
    client.connect("esp32_modbus01");
   */
    
  if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();
}

//Reconnect mqtt
/*void reconnectmqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect    
    if (client.connect("esp32_modbus01")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      //ESP.restart();
    }
  }
}
*/
//Reconnect to blynk
void reconnectblynk()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    Blynk.syncAll();
    digitalWrite(ledbb, LOW); //ledpin for check blynk connected 
  }
}

//Display Current Date/Time
void clockDisplay()
{
  display.clear();
  display.drawString(0,5,"#ESP32_Modbus01#");
  display.drawString(0,25,"Time :" + String(hour()) + "/" + minute() + "/" + second());
  display.drawString(0,40,"Date :" + String(day()) + " /" + month() + " /" + year()); 
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
        relayControl_modbusRTU(1,1,1);        
        Blynk.setProperty(Widget_LED_Valve1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve1, "label", "เปิดวาล์ว1");
        LedBlynkValve1.on();
        
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); 
      }
       else{                    
        relayControl_modbusRTU(1,1,0);
        Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์ว1");
        LedBlynkValve1.off();     
        
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();          
     }
}     
     
//****BUTTON ON/OFF Valve2****
 BLYNK_WRITE(Widget_Btn_Valve2){
      int valueValve2 = param.asInt();
      if(valueValve2 == 1){
        relayControl_modbusRTU(1,2,1);
        Blynk.setProperty(Widget_LED_Valve2, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve2, "label", "เปิดวาล์ว2");
        LedBlynkValve2.on();
        
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); 
      }
       else{              
        relayControl_modbusRTU(1,2,0);
        Blynk.setProperty(Widget_LED_Valve2, "label", "ปิดวาล์ว2");
        LedBlynkValve2.off();     
        
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();           
     }
}

//****BUTTON ON/OFF Valve3****
BLYNK_WRITE(Widget_Btn_Valve3){
      int valueValve3 = param.asInt();
      if(valueValve3 == 1){             
        relayControl_modbusRTU(1,3,1);
        Blynk.setProperty(Widget_LED_Valve3, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve3, "label", "เปิดวาล์ว3");
        LedBlynkValve3.on();
        
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); 
      }
       else{              
        relayControl_modbusRTU(1,3,0);
        Blynk.setProperty(Widget_LED_Valve3, "label", "ปิดวาล์ว3");
        LedBlynkValve3.off();     
        
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();         
     }
}
      
//****BUTTON ON/OFF Valve4****
BLYNK_WRITE(Widget_Btn_Valve4){
      int valueValve4 = param.asInt();
      if(valueValve4 == 1){      
        relayControl_modbusRTU(1,4,1);
        Blynk.setProperty(Widget_LED_Valve4, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve4, "label", "เปิดวาล์ว4");
        LedBlynkValve4.on();
        
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); 
      }
       else{              
        relayControl_modbusRTU(1,4,0);
        Blynk.setProperty(Widget_LED_Valve4, "label", "ปิดวาล์ว4");
        LedBlynkValve4.off();     
        
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();           
     }
}

//****BUTTON ON/OFF Valve5****
BLYNK_WRITE(Widget_Btn_Valve5){
      int valueValve5 = param.asInt();
      if(valueValve5 == 1){      
        relayControl_modbusRTU(2,1,1);
        Blynk.setProperty(Widget_LED_Valve5, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve5, "label", "เปิดวาล์ว5");
        LedBlynkValve5.on();
        
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); 
      }
       else{              
        relayControl_modbusRTU(2,1,0);
        Blynk.setProperty(Widget_LED_Valve5, "label", "ปิดวาล์ว5");
        LedBlynkValve5.off();     
        
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();           
     }
}

//****BUTTON ON/OFF Valve6****
BLYNK_WRITE(Widget_Btn_Valve6){
      int valueValve6 = param.asInt();
      if(valueValve6 == 1){      
        relayControl_modbusRTU(2,2,1);
        Blynk.setProperty(Widget_LED_Valve6, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve6, "label", "เปิดวาล์ว6");
        LedBlynkValve6.on();
        
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); 
      }
       else{              
        relayControl_modbusRTU(2,2,0);
        Blynk.setProperty(Widget_LED_Valve6, "label", "ปิดวาล์ว6");
        LedBlynkValve6.off();     
        
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();          
     }
}

//****BUTTON ON/OFF Valve7****
BLYNK_WRITE(Widget_Btn_Valve7){
      int valueValve7 = param.asInt();
      if(valueValve7 == 1){      
        relayControl_modbusRTU(2,3,1);
        Blynk.setProperty(Widget_LED_Valve7, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve7, "label", "เปิดวาล์ว7");
        LedBlynkValve7.on();
        
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); 
      }
       else{              
        relayControl_modbusRTU(2,3,0);
        Blynk.setProperty(Widget_LED_Valve7, "label", "ปิดวาล์ว7");
        LedBlynkValve7.off();     
       
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();           
     }
}

//****BUTTON ON/OFF Valve8****
BLYNK_WRITE(Widget_Btn_Valve8){
      int valueValve8 = param.asInt();
      if(valueValve8 == 1){      
        relayControl_modbusRTU(2,4,1);
        /* test
        relayControl_modbusRTU(6,1,1);
        relayControl_modbusRTU(6,2,1);
        relayControl_modbusRTU(6,3,1);
        relayControl_modbusRTU(6,4,1);
        relayControl_modbusRTU(6,5,1);
        relayControl_modbusRTU(6,6,1);
        relayControl_modbusRTU(6,7,1);
        relayControl_modbusRTU(6,8,1);
        relayControl_modbusRTU(6,9,1);
        relayControl_modbusRTU(6,10,1);
        relayControl_modbusRTU(6,11,1);
        relayControl_modbusRTU(6,12,1);
        relayControl_modbusRTU(6,13,1);
        relayControl_modbusRTU(6,14,1);
        relayControl_modbusRTU(6,15,1);
        relayControl_modbusRTU(6,16,1);
        //endtest*/
        
        Blynk.setProperty(Widget_LED_Valve8, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve8, "label", "เปิดวาล์ว8");
        LedBlynkValve8.on();
        
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); 
      }
       else{              
        relayControl_modbusRTU(2,4,0);
        //test
        /*relayControl_modbusRTU(6,1,0);
        relayControl_modbusRTU(6,2,0);
        relayControl_modbusRTU(6,3,0);
        relayControl_modbusRTU(6,4,0);
        relayControl_modbusRTU(6,5,0);
        relayControl_modbusRTU(6,6,0);
        relayControl_modbusRTU(6,7,0);
        relayControl_modbusRTU(6,8,0);
        relayControl_modbusRTU(6,9,0);
        relayControl_modbusRTU(6,10,0);
        relayControl_modbusRTU(6,11,0);
        relayControl_modbusRTU(6,12,0);
        relayControl_modbusRTU(6,13,0);
        relayControl_modbusRTU(6,14,0);
        relayControl_modbusRTU(6,15,0);
        relayControl_modbusRTU(6,16,0);
        //endtest
*/
        Blynk.setProperty(Widget_LED_Valve8, "label", "ปิดวาล์ว8");
        LedBlynkValve8.off();     
        
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();         
     }
}

void wtr10e_node3()
{
int id3 = 3;
  float temp3 = sht20ReadTemp_modbusRTU(id3);
  float humi3 = sht20ReadHumi_modbusRTU(id3);
 
  Serial.printf("Info: sht20[0x03] temperature3 = %.1f\r\n",temp3);
  //delay(3000);
 vTaskDelay(500);
  Serial.printf("Info: sht20[0x03] humidity3 = %.1f\r\n",humi3);
  vTaskDelay(500);
 //delay(3000);

/*
//Publish data to MQTT
/* long now = millis();
  if (now - lastMsg1 > 30000) {
    lastMsg1 = now;
    */
    //static char temperature3[7];
    //dtostrf(temp3, 6, 2, temperature3);    
    //client.publish("esp32_modbus01/temperature3", temperature3);

    //static char humidity3[7];
    //dtostrf(humi3, 6, 2, humidity3);
    //client.publish("esp32_modbus01/humidity3", humidity3);
    
  Blynk.virtualWrite(V30, temp3);
  Blynk.virtualWrite(V31, humi3);    
}

void wtr10e_node4()
{
int id4 = 4;
  float temp4 = sht20ReadTemp_modbusRTU(id4);
  float humi4 = sht20ReadHumi_modbusRTU(id4);
 
  Serial.printf("Info: sht20[0x04] temperature4 = %.1f\r\n",temp4);
  //delay(3000);
  vTaskDelay(500);
  Serial.printf("Info: sht20[0x04] humidity4 = %.1f\r\n",humi4);
  vTaskDelay(500);
 //delay(3000);

//Publish data to MQTT
/*  long now = millis();
  if (now - lastMsg2 > 30000) {
    lastMsg2 = now;
    */
   // static char temperature4[7];
    //dtostrf(temp4, 6, 2, temperature4);    
    //client.publish("esp32_modbus01/temperature4", temperature4);

    //static char humidity4[7];
    //dtostrf(humi4, 6, 2, humidity4);
   //client.publish("esp32_modbus01/humidity4", humidity4);
    
  Blynk.virtualWrite(V32, temp4);
  Blynk.virtualWrite(V33, humi4);
    
}
