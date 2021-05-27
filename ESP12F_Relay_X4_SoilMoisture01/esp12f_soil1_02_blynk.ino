//*****Define MCU Digital Pin*******//
//Relay1_btn1       16
//Relay2_btn2       14
//Relay3_btn3       12
//Relay4_btn4       13
//MAX485 DI=TX      15
//MAX485_RE_NEG=DE  5
//MAX485_RE         4
//MAX485 RO=RX      2   
//ว่าง                0
//*****Define MCU Digital Pin*******//

//*****Define Blynk Virtual Pin*****//
//V1  ไฟสถานะปุ่ม 1
//V2  ปุ่ม เปิด-ปิด 1
//V3  ไฟสถานะปุ่ม 2
//V4  ปุ่ม เปิด-ปิด 2
//V5  ไฟสถานะปุ่ม 3
//V6  ปุ่ม เปิด-ปิด 3
//V7  ไฟสถานะปุ่ม 4
//V8  ปุ่ม เปิด-ปิด 4
//V11 ค่าความชื้นในดิน Slave ID1
//*****Define Blynk Virtual Pin*****//


//=====Libraries=====//
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
//#include <SimpleTimer.h>
BlynkTimer timer;
int blynkIsDownCount = 0;

//======Modbus=====//
#include <SoftwareSerial.h>
#include <ModbusMaster.h>
SoftwareSerial mySerial(2, 15); // RO=RX, DI=TX

#define MAX485_RE      4  //RE
#define MAX485_RE_NEG  5  //DE

// instantiate ModbusMaster object
ModbusMaster node;
//======Modbus=====//

//BTN1
#define Relay1_btn1   16
#define Widget_LED_btn1 V1          //ไฟสถานะปุ่ม 1
#define Widget_Btn_btn1 V2          //ปุ่ม เปิด-ปิด 1
WidgetLED LedBlynkbtn1(Widget_LED_btn1);

//BTN2
#define Relay2_btn2   14
#define Widget_LED_btn2 V3          //ไฟสถานะปุ่ม 2
#define Widget_Btn_btn2 V4          //ปุ่ม เปิด-ปิด 2
WidgetLED LedBlynkbtn2(Widget_LED_btn2);

//BTN3
#define Relay3_btn3   12
#define Widget_LED_btn3 V5          //ไฟสถานะปุ่ม 3
#define Widget_Btn_btn3 V6          //ปุ่ม เปิด-ปิด 3
WidgetLED LedBlynkbtn3(Widget_LED_btn3);

//BTN4
#define Relay4_btn4   13
#define Widget_LED_btn4 V7          //ไฟสถานะปุ่ม 4
#define Widget_Btn_btn4 V8          //ปุ่ม เปิด-ปิด 4
WidgetLED LedBlynkbtn4(Widget_LED_btn4);

//==Modbus Pre & Post Transmission==//
void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_RE, 1);
}
void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_RE, 0);
}
//==Modbus Pre & Post Transmission==//

//==Wi-Fi and blynk credentials====//   
char auth[] = "";                 //Blynk auth token
char ssid[] = "";                 //Wi-Fi                
char pass[] = "";                 //Password เชื่อมต่อ Wi-Fi                         
//==Wi-Fi and blynk credentials====//  

//=====Setup Function=====//
void setup()
{
  //Modbus pinMode
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_RE, OUTPUT);
  // Init in receive mode
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_RE, 0);
  Serial.println("start init serial 0");
  Serial.begin(9600);
  
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
  
  while (!Serial) {
    Serial.println("loop for init serial 0"); // wait for serial port to connect. Needed for Native USB only
  }
  Serial.println("start init software serial");
  mySerial.begin(9600);
  while (!mySerial) {
    Serial.println("loop for init software serial"); // wait for serial port to connect. Needed for Native USB only
  }
  
  //Modbus slave ID 1
  node.begin(1, mySerial);
  
  // Callbacks allow us to configure the RS485 transceiver correctly
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  //End Modbus pinMode
 
  Serial.println();

  // Setup Pin Mode
  pinMode(Relay1_btn1,OUTPUT);                // NODEMCU PIN gpio16 
  pinMode(Relay2_btn2,OUTPUT);                // NODEMCU PIN gpio14
  pinMode(Relay3_btn3,OUTPUT);                // NODEMCU PIN gpio12   
  pinMode(Relay4_btn4,OUTPUT);                // NODEMCU PIN GPIO13         
  
  // Set Defult Relay Status
  digitalWrite(Relay1_btn1,LOW);              // NODEMCU PIN gpio16
  digitalWrite(Relay2_btn2,LOW);              // NODEMCU PIN gpio14
  digitalWrite(Relay3_btn3,LOW);              // NODEMCU PIN gpio12
  digitalWrite(Relay4_btn4,LOW);              // NODEMCU PIN gpio13 
  
  Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  timer.setInterval(30000L, reconnecting);  
  timer.setInterval(1000L, Modbus_SoilMoisture01);
}
//=====Setup Function=====//

//=====Modbus_SoilMoisture01=====//
void Modbus_SoilMoisture01()
{
  uint8_t result;
  
  float soilmoisture01 = (node.getResponseBuffer(2)/10.0f);
   
  Serial.println("get data");
  result = node.readHoldingRegisters(0x0000, 3); // Read 2 registers starting at 1)
  if (result == node.ku8MBSuccess)
  {
    Serial.print("Soil Moisture Sensor1: ");
    Serial.println(node.getResponseBuffer(2)/10.0f);
  }
  delay(1000);
  Blynk.virtualWrite(V11,soilmoisture01);
}
//=====Modbus_SoilMoisture01=====//

//****BUTTON ON/OFF btn1****//
 BLYNK_WRITE(Widget_Btn_btn1){
      int valuebtn1 = param.asInt();
      if(valuebtn1 == 1){
        digitalWrite(Relay1_btn1,HIGH);
        Blynk.setProperty(Widget_LED_btn1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_btn1, "label", "เปิดปุ่มที่ 1");
        LedBlynkbtn1.on();       
      }
       else{              
        digitalWrite(Relay1_btn1,LOW);
        Blynk.setProperty(Widget_LED_btn1, "label", "ปิดปุ่มที่ 1");
        LedBlynkbtn1.off();          
     }
}
//****BUTTON ON/OFF btn1****//

//****BUTTON ON/OFF btn2****//
 BLYNK_WRITE(Widget_Btn_btn2){
      int valuebtn2 = param.asInt();
      if(valuebtn2 == 1){
        digitalWrite(Relay2_btn2,HIGH);
        Blynk.setProperty(Widget_LED_btn2, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_btn2, "label", "เปิดปุ่มที่ 2");
        LedBlynkbtn2.on();
        
      }
       else{              
        digitalWrite(Relay2_btn2,LOW);
        Blynk.setProperty(Widget_LED_btn2, "label", "ปิดปุ่มที่ 2");
        LedBlynkbtn2.off();          
     }
}
//****BUTTON ON/OFF btn2****//

//****BUTTON ON/OFF btn3****//
 BLYNK_WRITE(Widget_Btn_btn3){
      int valuebtn3 = param.asInt();
      if(valuebtn3 == 1){
        digitalWrite(Relay3_btn3,HIGH);
        Blynk.setProperty(Widget_LED_btn3, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_btn3, "label", "เปิดปุ่มที่ 3");
        LedBlynkbtn3.on();
        
      }
       else{              
        digitalWrite(Relay3_btn3,LOW);
        Blynk.setProperty(Widget_LED_btn3, "label", "ปิดปุ่มที่ 3");
        LedBlynkbtn3.off();          
     }
}
//****BUTTON ON/OFF btn3****//

//****BUTTON ON/OFF btn4****//
 BLYNK_WRITE(Widget_Btn_btn4){
      int valuebtn4 = param.asInt();
      if(valuebtn4 == 1){
        digitalWrite(Relay4_btn4,HIGH);
        Blynk.setProperty(Widget_LED_btn4, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_btn4, "label", "เปิดปุ่มที่ 4");
        LedBlynkbtn4.on();
        
      }
       else{              
        digitalWrite(Relay4_btn4,LOW);
        Blynk.setProperty(Widget_LED_btn4, "label", "ปิดปุ่มที่ 4");
        LedBlynkbtn4.off();          
     }
}
//****BUTTON ON/OFF btn4****//

//=====Blynk connected=====//
BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    Blynk.syncAll();

 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
 }
}

//=====Blynk connected=====//

//=====Blynk reconnect=====//
void reconnecting()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    Blynk.syncAll();
  }
  if (blynkIsDownCount >= 10){
    ESP.reset();
  }
}
//=====Blynk reconnect=====//

//=====Loop function======//
void loop()
{

  if (Blynk.connected())
  {
    Blynk.run();
    Blynk.syncAll();
  }
   timer.run();  
}
//=====Loop function======//
