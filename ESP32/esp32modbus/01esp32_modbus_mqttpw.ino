//อ้างจาก https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/
//#include <FS.h>                   //this needs to be first, or it all crashes and burns...
//#include <SPIFFS.h>               //เพิ่ม
#include <WiFi.h>                 //https://github.com/esp8266/Arduino
#include <WiFiClient.h>
//needed for library
//#include <DNSServer.h>
//#include <WebServer.h>
//#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
//#include <ArduinoJson.h>          //Ver 5.13.4   //https://github.com/bblanchon/ArduinoJson
//------------------------------------------------------------------------------------------------------------------------//
#include <BlynkSimpleEsp32.h>      //  Blynk_Release_v0.6.1 
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
int blynkIsDownCount = 0;
BlynkTimer timer;

//Modbus
#define RXD2            16
#define TXD2            17
HardwareSerial rs485(1);
#include "modbusRTU.h"

//Define pin for clear and config AP&Token
#define AP_Config       14

//10A 4channel Relay extend 

#define pump            23        // Relay connect to pump 
#define ledbb           27        //Check blynk connected 
#define ledfan          26        //Cooling Fan

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

char ssid[] = "";                         //ชื่อ SSID ที่เราต้องการเชื่อมต่อ        
char pass[] = "";                               //รหัสผ่าน WI-FI
char auth[] = "";                         //Auth token from blynk app 

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

//MQTT
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
const char* mqtt_server = "";
WiFiClient esp32_modbus01;
PubSubClient client(mqtt_server, 1883, callback,esp32_modbus01);

long lastMsg1 = 0;
long lastMsg2 = 0;
long lastMsg3 = 0;
long lastMsg4 = 0;


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

//bool shouldSaveConfig = false;

//callback notifying us of the need to save config
/*void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
*/
//Setup
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
 rs485.begin(9600, SERIAL_8N1, RXD2, TXD2);
 //delay(3000);
  //OLED
  display.init();
  display.display();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.display();
  
  // Setup Pin Mode
  pinMode(AP_Config, INPUT_PULLUP);//กำหนดโหมดใช้งานให้กับขา AP_Config เป็นขา กดปุ่ม ค้าง เพื่อตั้งค่า AP config 
  pinMode(pump, OUTPUT);              // Pin 23 relay 10A in1 connect to Manatic + overload
  pinMode(ledbb,OUTPUT);              // Pin 27 relay 10A in2 connect to Green PoltLamp check blynk connected
  pinMode(ledfan,OUTPUT);             // Pin 26 relay 10A in3 connect to Red PoltLamp start cooling fan 
  
  // Set Defult Relay Status
  digitalWrite(pump, HIGH);           // Pin 23 relay 10A in1 connect to Manatic + overload
  digitalWrite(ledbb,HIGH);           // Pin 27 relay 10A in2 connect to Green PoltLamp check blynk connected
  digitalWrite(ledfan,HIGH);          // Pin 26 relay 10A in3 connect to Red PoltLamp start cooling fan   

  relayControl_modbusRTU(1,1,0);  //Address, channel, logic 0=off,1=on , Valve1
  relayControl_modbusRTU(1,2,0);  //Address, channel, logic 0=off,1=on , Valve2
  relayControl_modbusRTU(1,3,0);  //Address, channel, logic 0=off,1=on , Valve3
  relayControl_modbusRTU(1,4,0);  //Address, channel, logic 0=off,1=on , Valve4
  
  relayControl_modbusRTU(1,1,0);  //Address, channel, logic 0=off,1=on , Valve5
  relayControl_modbusRTU(1,2,0);  //Address, channel, logic 0=off,1=on , Valve6
  relayControl_modbusRTU(1,3,0);  //Address, channel, logic 0=off,1=on , Valve7
  relayControl_modbusRTU(1,4,0);  //Address, channel, logic 0=off,1=on , Valve8
 
  //Begin Sync time
  rtc.begin();
 
  Serial.println("local ip"); //แสดงข้อความใน Serial Monitor
  delay(100);
  Serial.println(WiFi.localIP());//แสดงข้อความใน Serial Monitor

 //MQTT
   client.setServer(mqtt_server, 1883);
   
  // Blynk.config(blynk_token);////เริ่มการเชื่อมต่อ Blynk Server แบบปกติ
    Blynk.begin(auth, ssid, pass);
    //timer.setInterval(30000L, dht11SensorData); 
    timer.setInterval(10000L,wtr10e_node1);    //Read Modbus SHT20 Sensor1
    timer.setInterval(10000L,wtr10e_node2);    //Read Modbus SHT20 Sensor2
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

   //MQTT
  if (!client.connected()) {
    reconnectmqtt();
  }
  //client.loop();
  if(!client.loop())
    client.connect("esp32_modbus01");
    
  if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();
}

void reconnectmqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect    
    if (client.connect("esp32_modbus01", "mqtt_user" , "mqtt_password")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      ESP.restart();
    }
  }
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
        /*
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); */
      }
       else{                    
        relayControl_modbusRTU(1,1,0);
        Blynk.setProperty(Widget_LED_Valve1, "label", "ปิดวาล์ว1");
        LedBlynkValve1.off();     
        /*
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();  */         
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
        /*
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); */
      }
       else{              
        relayControl_modbusRTU(1,2,0);
        Blynk.setProperty(Widget_LED_Valve2, "label", "ปิดวาล์ว2");
        LedBlynkValve2.off();     
        /*
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();   */        
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
        /*
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); */
      }
       else{              
        relayControl_modbusRTU(1,3,0);
        Blynk.setProperty(Widget_LED_Valve3, "label", "ปิดวาล์ว3");
        LedBlynkValve3.off();     
        /*
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();  */        
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
        /*
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); */
      }
       else{              
        relayControl_modbusRTU(1,4,0);
        Blynk.setProperty(Widget_LED_Valve4, "label", "ปิดวาล์ว4");
        LedBlynkValve4.off();     
        /*
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();   */        
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
        /*
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); */
      }
       else{              
        relayControl_modbusRTU(2,1,0);
        Blynk.setProperty(Widget_LED_Valve5, "label", "ปิดวาล์ว5");
        LedBlynkValve5.off();     
        /*
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();   */        
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
        /*
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); */
      }
       else{              
        relayControl_modbusRTU(2,2,0);
        Blynk.setProperty(Widget_LED_Valve6, "label", "ปิดวาล์ว6");
        LedBlynkValve6.off();     
        /*
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();   */        
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
        /*
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); */
      }
       else{              
        relayControl_modbusRTU(2,3,0);
        Blynk.setProperty(Widget_LED_Valve7, "label", "ปิดวาล์ว7");
        LedBlynkValve7.off();     
        /*
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();   */        
     }
}

//****BUTTON ON/OFF Valve8****
BLYNK_WRITE(Widget_Btn_Valve8){
      int valueValve8 = param.asInt();
      if(valueValve8 == 1){      
        relayControl_modbusRTU(2,4,1);
        Blynk.setProperty(Widget_LED_Valve8, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Valve8, "label", "เปิดวาล์ว8");
        LedBlynkValve8.on();
        /*
        digitalWrite(pump, LOW);
        Blynk.virtualWrite(Widget_Btn_Pump,1);
        Blynk.setProperty(Widget_LED_Pump, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Pump, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump.on(); */
      }
       else{              
        relayControl_modbusRTU(2,4,0);
        

        Blynk.setProperty(Widget_LED_Valve8, "label", "ปิดวาล์ว8");
        LedBlynkValve8.off();     
        /*
        digitalWrite(pump, HIGH);      
        Blynk.virtualWrite(Widget_Btn_Pump,0);                
        Blynk.setProperty(Widget_LED_Pump, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump.off();   */        
     }
}

void wtr10e_node1()
{
int id = 3;
  float temp1 = sht20ReadTemp_modbusRTU(id);
  float humi1 = sht20ReadHumi_modbusRTU(id);
 
  Serial.printf("Info: sht20[0x01] temperature1 = %.1f\r\n",temp1);
  //delay(3000);
 vTaskDelay(500);
  Serial.printf("Info: sht20[0x01] humidity1 = %.1f\r\n",humi1);
  vTaskDelay(500);
 //delay(3000);

//Publish data to MQTT
/* long now = millis();
  if (now - lastMsg1 > 30000) {
    lastMsg1 = now;
    
    static char temperature1[7];
    dtostrf(temp1, 6, 2, temperature1);    
    client.publish("esp32_modbus01/temperature1", temperature1);

    static char humidity1[7];
    dtostrf(humi1, 6, 2, humidity1);
    client.publish("esp32_modbus01/humidity1", humidity1);
    */
  Blynk.virtualWrite(V30, temp1);
  Blynk.virtualWrite(V31, humi1);    
}

void wtr10e_node2()
{
int id4 = 4;
  float temp2 = sht20ReadTemp_modbusRTU(id4);
  float humi2 = sht20ReadHumi_modbusRTU(id4);
 
  Serial.printf("Info: sht20[0x02] temperature2 = %.1f\r\n",temp2);
  //delay(3000);
  vTaskDelay(500);
  Serial.printf("Info: sht20[0x02] humidity2 = %.1f\r\n",humi2);
  vTaskDelay(500);
 //delay(3000);

//Publish data to MQTT
/*  long now = millis();
  if (now - lastMsg2 > 30000) {
    lastMsg2 = now;
   
    static char temperature2[7];
    dtostrf(temp2, 6, 2, temperature2);    
    client.publish("esp32_modbus01/temperature2", temperature2);

    static char humidity2[7];
    dtostrf(humi2, 6, 2, humidity2);
    client.publish("esp32_modbus01/humidity2", humidity2);
     */
  Blynk.virtualWrite(V32, temp2);
  Blynk.virtualWrite(V33, humi2);
    
}
