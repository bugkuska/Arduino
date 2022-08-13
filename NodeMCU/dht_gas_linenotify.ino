#define BLYNK_PRINT Serial
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <TridentTD_LineNotify.h>                             
#include <BlynkSimpleEsp8266.h>
#include <SimpleDHT.h>
#include <SPI.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <DNSServer.h>            
#include <ESP8266WebServer.h>  
#include <WiFiManager.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);

#include "DHT.h"

//Pin INPUT
#define DHTPIN 2 // D4
#define DHTTYPE DHT11

//Pin OUTPUT
#define Relay4_FanGas 14  // D5
#define Relay5_FanBox 12  // D6
                                                                                    
char auth[] = "f55c4f4f281441ccb9e0f774f226db38";
char ssid[] = "Stockholm";
char pass[] = "-jone500";
#define LINE_TOKEN  "05NS8Gen34RHi2i6uLASOvax59ORF4R724OShDzeaFr" //Token ที่ได้จากการสมัคร ID Line
#define mqtt_server "35.185.189.134"

#define humidBox "sensor/HumidityBox"
#define tempcBox "sensor/TemperatureBox"
#define tempf "sensor/tempf"
#define MQ135 "sensor/MQ135"

DHT dht1(DHTPIN, DHTTYPE);
BlynkTimer timer;

WiFiClient espClient;
PubSubClient client(espClient);

BLYNK_CONNECTED(){
    Blynk.syncAll();
}

     //ปุ่ม Blynk
     #define Widget_Btn_AMFG V16       //ปุ่ม Auto/Manual FAN GAS
     #define Widget_Btn_FanGas V17     //ปุ่ม เปิด-ปิด FAN GAS
     #define Widget_Btn_AMFB V24         //ปุ่ม Auto/Manual FAN BOX
     #define Widget_Btn_FanBox V26         //ปุ่ม เปิด-ปิด พัดลมในกล่องอุปกรณ์

     //สถานะปุ่ม Blynk
     #define Widget_LED_FanGas V19    //ไฟสถานะปุ่ม FAN GAS
     #define Widget_LED_AMFG V18      //ไฟสถานะปุ่ม AUTO/MANUAL FAN GAS
     #define Widget_LED_FanBox V25        //ไฟสถานะปุ่ม พัดลมในกล่องอุปกรณ์
     #define Widget_LED_AMFB V23        //ไฟสถานะปุ่ม AUTO/MANUAL FAN Box

     WidgetLED LedBlynkFanGas(Widget_LED_FanGas);
     WidgetLED LedBlynkAMFG(Widget_LED_AMFG);
     WidgetLED LedBlynkFanBox(Widget_LED_FanBox);
     WidgetLED LedBlynkAMFB(Widget_LED_AMFB); 

     //ประกาศตัวแปร
     unsigned long previousMillis3 = 0;
     unsigned long previousMillis4 = 0;
     int sec3 = 0, m3 = 0;     // set ค่าเวลาเริ่มต้นจากการเเจ้งเตือนเกิน sec คือวินาที m คือนาที
     int sec4 = 0, m4 = 0;
     
     int GasDensity = 600;     // ค่าความหนาแน่นของแก๊สที่จะให้เเจ้งเตือนผ่านไลน์
     int Humidity = 90;        // ค่าความชื้นที่จะให้เเจ้งเตือนผ่านไลน์
     
     bool auto4 = false;
     bool auto5 = false;

void setup() {  
  Serial.begin(115200);
  setup_wifi();
  Blynk.begin(auth, ssid, pass, "blynk.jpnet.co.th",8080);
  client.setServer(mqtt_server, 1883);
  dht1.begin();
  lcd.init();
  lcd.backlight();
  
  //pinMode OUTPUT
  pinMode(Relay4_FanGas, OUTPUT);
  digitalWrite(Relay4_FanGas, LOW);

  pinMode(Relay5_FanBox, OUTPUT);
  digitalWrite(Relay5_FanBox, LOW);

  //เรียกใช้ Function
  timer.setInterval(1000, sendMQ135);
  timer.setInterval(1000, sendDHTBox);
}

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

void setup_wifi() {
  delay(100);

  WiFi.begin(ssid, pass);
  WiFiManager wifiManager;
  wifiManager.setTimeout(180);
  if(!wifiManager.autoConnect("AutoConnectWiFi"))
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  WiFi.printDiag(Serial);
  Serial.println();
  Serial.println("connected...OK");
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
    
      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;  
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);
      clientName += "-";
      clientName += String(micros() & 0xff, 16);
      Serial.print("Connecting to ");
      Serial.print(mqtt_server);
      Serial.print(" as ");
      Serial.println(clientName);


    // Attempt to connect
    // If you do not want to use a username and password, change next line to
  if (client.connect((char*) clientName.c_str())) {
    //if (client.connect((char*) clientName.c_str()), mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  LINE.setToken(LINE_TOKEN);
  
  if (!client.connected()) {
    reconnect();
    }
    client.loop();
    // Wait a few seconds between measurements.
    delay(100);
    
  Blynk.run();   //Run the Blynk 
  timer.run();                    
}

void sendMQ135(){
  float MQ = analogRead(A0);                                                                                                 
  Blynk.virtualWrite(V15, MQ);

   //******AUTO FAN GAS*******
     if(MQ >= 600 ){
      if(auto4 == true){               
        digitalWrite(Relay4_FanGas, HIGH);
        Blynk.virtualWrite(Widget_Btn_FanGas, 1);       
        Blynk.setProperty(Widget_Btn_FanGas, "onBackColor", "#008080");
        Blynk.setProperty(Widget_LED_FanGas, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_FanGas, "label", "Buzzer กำลังทำงาน");
        Blynk.virtualWrite(Widget_Btn_AMFG, 1);
        LedBlynkFanGas.on();
        }
     }
      if(MQ < 90){
        if(auto4 == true){
          digitalWrite(Relay4_FanGas, LOW);
          Blynk.virtualWrite(Widget_Btn_FanGas, 0);
          Blynk.setProperty(Widget_Btn_FanGas, "offBackColor", "#008080");
          Blynk.setProperty(Widget_LED_FanGas, "label", "ปิด Buzzer แล้ว");
          Blynk.virtualWrite(Widget_Btn_AMFG, 1);                        
          LedBlynkFanGas.off();  
          }       
     }
         client.publish(MQ135, String(MQ).c_str(), true);

         unsigned long currentMillis3 = millis(); // เมธอด millis() เป็นตัวจับเวลาเริ่มตั้งแต่บอร์ดทำงานเเละนับไป 50 วันจะวนกลับมาเริ่มใหม่อีกครั้งมีหน่วยเป็นมิลิเซ็ก (1000 = 1 นาที)
         if(currentMillis3 - previousMillis3 >= 1000) {
          sec3++;
          if(sec3 >= 60){
            m3++;
            sec3 = 0;
          }
          if(m3 >= 1){ 
            m3 = 0; 
            GasDensity = 600;  //ถ้าเวลาครบ 1 นาทีตามที่กำหนดให้ความหนาแน่นของแก๊สเท่ากับ 600
          }
          if(int(MQ)>= GasDensity){   //ถ้าระดับน้ำมากกว่าค่าที่กำหนด(600) ให้แจ้งเตือนผ่านไลน์และให้บวกระดับน้ำแจ้งเตื่อนต่อไปอีก 1 (++)
            LINE.notify("\n--------------------------------\n!!! Buzzer กำลังทำงาน !!!\n--------------------------------\n ความหนาแน่นของแก๊ส : "+String(MQ)+" ppm");
            Serial.println("!!! แจ้งเตือนความหนาแน่นของแก๊สเกินกำหนด !!!");
            GasDensity = 1100;
            delay(1000);
            }
            previousMillis3 = currentMillis3;
         }      
          Serial.print("GasDensity : ");
          Serial.println(MQ);
          Serial.print(m3);Serial.print(":");
          Serial.println(sec3);
          delay(1000);
}

void sendDHTBox(){
  float h = dht1.readHumidity();
  float t = dht1.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  float f = dht1.readTemperature(true);
  
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
 
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V22, h);
  Blynk.virtualWrite(V21, t);

  
     //******AUTO FAN*******
     if(h >= 90){    
      if(auto5 == true){          
        digitalWrite(Relay5_FanBox, HIGH);
        Blynk.virtualWrite(Widget_Btn_FanBox, 1);       
        Blynk.setProperty(Widget_Btn_FanBox, "onBackColor", "#008080");
        Blynk.setProperty(Widget_LED_FanBox, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_FanBox, "label", "พัดลมกำลังทำงาน");
        Blynk.virtualWrite(Widget_Btn_AMFB, 1);
        LedBlynkFanBox.on();
        }   
     }
      if(h < 90){
        if(auto5 == true){
          digitalWrite(Relay5_FanBox, LOW);
          Blynk.virtualWrite(Widget_Btn_FanBox, 0);
          Blynk.setProperty(Widget_Btn_FanBox, "offBackColor", "#008080");
          Blynk.setProperty(Widget_LED_FanBox, "label", "ปิดพัดลมแล้ว");
          Blynk.virtualWrite(Widget_Btn_AMFB, 1);                        
          LedBlynkFanBox.off();  
          }       
     }
          client.publish(tempcBox, String(t).c_str(), true);
          client.publish(humidBox, String(h).c_str(), true);
       
         
         unsigned long currentMillis4 = millis(); // เมธอด millis() เป็นตัวจับเวลาเริ่มตั้งแต่บอร์ดทำงานเเละนับไป 50 วันจะวนกลับมาเริ่มใหม่อีกครั้งมีหน่วยเป็นมิลิเซ็ก (1000 = 1 นาที)
         if(currentMillis4 - previousMillis4 >= 1000) {
          sec4++;
          if(sec4 >= 60){
            m4++;
            sec4 = 0;
          }
          if(m4 >= 1){ 
            m4 = 0; 
            Humidity = 90;  //ถ้าเวลาครบ 1 นาทีตามที่กำหนดให้ความชื้นเท่ากับ 90 องศา
          }

          if(int(h)>= Humidity){   //ถ้าความชื้นมากกว่าค่าที่กำหนด(90) ให้แจ้งเตือนผ่านไลน์และให้บวกความชื้นแจ้งเตื่อนต่อไปอีก 1 (++)
            LINE.notify("\n--------------------------------\n!!! พัดลมภายในกล่องกำลังทำงาน !!!\n--------------------------------\n ความชื้น : "+String(h)+" %");
            Serial.println("!!! แจ้งเตือนความชื้นเกินกำหนด !!!");
            Humidity = 200;
            delay(1000);
            }
            previousMillis4 = currentMillis4;
         }
          Serial.print("Humidity : ");
          Serial.println(h);
          Serial.print(m4);Serial.print(":");
          Serial.println(sec4);
          delay(1000); 

          lcd.setCursor(0,0);
          lcd.print("Humidity");
          lcd.setCursor(0,1);
          lcd.print("Temperatu");
          lcd.setCursor(10,0);
          lcd.print(h + String("%"));
          lcd.setCursor(10,1);
          lcd.print(t + String("C"));
}

//*****BUTTON AUTO/MANUAL FAN GAS*********
     BLYNK_WRITE(Widget_Btn_AMFG){
      int valueAMFG = param.asInt();
      if(valueAMFG == 1){
        auto4 = true; 
        Blynk.virtualWrite(Widget_Btn_AMFG, 1);
        Blynk.setProperty(Widget_Btn_AMFG, "onBackColor", "#008080");
        Blynk.setProperty(Widget_LED_AMFG, "color", "#FFFF00");
        Blynk.setProperty(Widget_LED_AMFG, "label", "AUTO");
        LedBlynkAMFG.on();       
      }
       else {
        auto4 = false; 
        Blynk.virtualWrite(Widget_Btn_AMFG, 0);
        Blynk.setProperty(Widget_Btn_AMFG, "offBackColor", "#008080");
        Blynk.setProperty(Widget_LED_AMFG, "color", "#330066");
        Blynk.setProperty(Widget_LED_AMFG, "label", "MANUAL");
        LedBlynkAMFG.on();                          
      }
}

 //*****BUTTON AUTO/MANUAL FAN BOX*********
     BLYNK_WRITE(Widget_Btn_AMFB){
      int valueAMFB = param.asInt();
      if(valueAMFB == 1){
        auto5 = true; 
        Blynk.virtualWrite(Widget_Btn_AMFB, 1);
        Blynk.setProperty(Widget_Btn_AMFB, "onBackColor", "#008080");
        Blynk.setProperty(Widget_LED_AMFB, "color", "#FFFF00");
        Blynk.setProperty(Widget_LED_AMFB, "label", "AUTO");
        LedBlynkAMFB.on();       
      }
       else {
        auto5 = false; 
        Blynk.virtualWrite(Widget_Btn_AMFB, 0);
        Blynk.setProperty(Widget_Btn_AMFB, "offBackColor", "#008080");
        Blynk.setProperty(Widget_LED_AMFB, "color", "#330066");
        Blynk.setProperty(Widget_LED_AMFB, "label", "MANUAL");
        LedBlynkAMFB.on();                          
      }
}

      //*************MANUAL*********
     //****BUTTON ON/OFF FAN GAS****
     BLYNK_WRITE(Widget_Btn_FanGas){
      int valueFanGas = param.asInt();
      if(valueFanGas == 1){
        digitalWrite(Relay4_FanGas, HIGH);
        Blynk.setProperty(Widget_Btn_FanGas, "onBackColor", "#008080");
        Blynk.setProperty(Widget_LED_FanGas, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_FanGas, "label", "Buzzer กำลังทำงาน"); 
        LedBlynkFanGas.on();

      }
       else{              
        digitalWrite(Relay4_FanGas, LOW);                      
        Blynk.virtualWrite(Widget_Btn_AMFG, 0);
        Blynk.setProperty(Widget_Btn_FanGas, "offBackColor", "#008080");
        Blynk.setProperty(Widget_LED_FanGas, "label", "ปิด Buzzer แล้ว");
        LedBlynkFanGas.off();              
     }
}

     //*************MANUAL*********
     //****BUTTON ON/OFF FAN****
     BLYNK_WRITE(Widget_Btn_FanBox){
      int valueFanBox = param.asInt();
      if(valueFanBox == 1){
        digitalWrite(Relay5_FanBox, HIGH);
        Blynk.setProperty(Widget_Btn_FanBox, "onBackColor", "#008080");
        Blynk.setProperty(Widget_LED_FanBox, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_FanBox, "label", "พัดลมกำลังทำงาน");
        LedBlynkFanBox.on();

      }
       else{              
        digitalWrite(Relay5_FanBox, LOW);                      
        Blynk.virtualWrite(Widget_Btn_AMFB, 0);
        Blynk.setProperty(Widget_Btn_FanBox, "offBackColor", "#008080");
        Blynk.setProperty(Widget_LED_FanBox, "label", "ปิดพัดลมแล้ว");
        LedBlynkFanBox.off();              
     }
}
