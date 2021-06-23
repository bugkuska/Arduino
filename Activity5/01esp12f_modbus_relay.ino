#define BLYNK_TEMPLATE_ID "TMPLUxSOswRx"
#define BLYNK_DEVICE_NAME "ESP12F"

#define BLYNK_FIRMWARE_VERSION  "0.1.0"
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
//#include <SimpleTimer.h>
BlynkTimer timer;
int blynkIsDownCount = 0;


//=====Wi-Fi & Blynk Credential=====//
char ssid[] = "smfthailand_trainning";                   //ชื่อ SSID ที่เราต้องการเชื่อมต่อ        
char pass[] = "0814111142";                   //รหัสผ่าน WI-FI
char auth[] = "";   //Auth token from blynk app 
//=====Wi-Fi & Blynk Credential=====//

//==Pool size for Modbus Write command==//
int8_t pool_size1;
//==Pool size for Modbus Write command==//

//==Define Blynk Virtual Pin for SW1-SW8==//
#define Widget_Btn_SW1 V1         //ปุ่ม เปิด-ปิด SW1
#define Widget_Btn_SW2 V2         //ปุ่ม เปิด-ปิด SW2
#define Widget_Btn_SW3 V3         //ปุ่ม เปิด-ปิด SW3
#define Widget_Btn_SW4 V4         //ปุ่ม เปิด-ปิด SW4
#define Widget_Btn_SW5 V5         //ปุ่ม เปิด-ปิด SW5
#define Widget_Btn_SW6 V6         //ปุ่ม เปิด-ปิด SW6
#define Widget_Btn_SW7 V7         //ปุ่ม เปิด-ปิด SW7
#define Widget_Btn_SW8 V8         //ปุ่ม เปิด-ปิด SW8
//==Define Blynk Virtual Pin for SW1-SW8==//

//=====Libraries=====//
#include <SoftwareSerial.h>
#include <ModbusMaster.h>
SoftwareSerial mySerial(2, 15); // RO=RX, DI=TX
#define MAX485_RE      4  //RE
#define MAX485_RE_NEG  5  //DE

//==instantiate ModbusMaster object==//
ModbusMaster node1;       //12CH Modbus RTU Relay1

//==instantiate ModbusMaster object==//

//==pre&post transmission1==//
void preTransmission1()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_RE, 1);
}

void postTransmission1()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_RE, 0);
}
//==pre&post transmission1==//

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
  node1.begin(1, mySerial);
  // Callbacks allow us to configure the RS485 transceiver correctly
  node1.preTransmission(preTransmission1);
  node1.postTransmission(postTransmission1);
  Serial.println();
  
  //Begin connect to blynk-cloud.com
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
   Blynk.begin(auth, ssid, pass);
  timer.setInterval(30000L, reconnecting); 
}
//=====Setup Function=====//

//****BUTTON ON/OFF SW1****//
 BLYNK_WRITE(Widget_Btn_SW1){ 
      int valueSW1 = param.asInt();
      if(valueSW1 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node1.writeSingleRegister(0x01,0x0100); 
      }
       else{                    
       pool_size1 = node1.writeSingleRegister(0x01,0x0200);
     }
} 
//****BUTTON ON/OFF SW1****//


/*void checkphysic_btn_state()
{
  stateled1=digitalRead(Relay1_Valve1);   //Check ON/OFF สวิตส์1
  if (stateled1 != prevStateled1)
  {
      if (stateled1==1) Blynk.virtualWrite(V1,1);
      if (stateled1==0) Blynk.virtualWrite(V1,0);
  }
  prevStateled1=stateled1;
}
*/

//****BUTTON ON/OFF SW2****//
 BLYNK_WRITE(Widget_Btn_SW2){
      int valueSW2 = param.asInt();
      if(valueSW2 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node1.writeSingleRegister(0x02,0x0100);
      }
       else{                    
       pool_size1 = node1.writeSingleRegister(0x02,0x0200);
     }
} 
//****BUTTON ON/OFF SW2****//

//****BUTTON ON/OFF SW3****//
 BLYNK_WRITE(Widget_Btn_SW3){
      int valueSW3 = param.asInt();
      if(valueSW3 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node1.writeSingleRegister(0x03,0x0100);
      }
       else{                    
       pool_size1 = node1.writeSingleRegister(0x03,0x0200);
     }
} 
//****BUTTON ON/OFF SW3****//

//****BUTTON ON/OFF SW4****//
 BLYNK_WRITE(Widget_Btn_SW4){
      int valueSW4 = param.asInt();
      if(valueSW4 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node1.writeSingleRegister(0x04,0x0100); 
      }
       else{                    
       pool_size1 = node1.writeSingleRegister(0x04,0x0200);
     }
} 
//****BUTTON ON/OFF SW4****//

//****BUTTON ON/OFF SW5****//
 BLYNK_WRITE(Widget_Btn_SW5){
      int valueSW5 = param.asInt();
      if(valueSW5 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node1.writeSingleRegister(0x05,0x0100);
      }
       else{                    
       pool_size1 = node1.writeSingleRegister(0x05,0x0200);
     }
} 
//****BUTTON ON/OFF SW5****//

//****BUTTON ON/OFF SW6****//
 BLYNK_WRITE(Widget_Btn_SW6){
      int valueSW6 = param.asInt();
      if(valueSW6 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node1.writeSingleRegister(0x06,0x0100);
      }
       else{                    
       pool_size1 = node1.writeSingleRegister(0x06,0x0200);
     }
} 
//****BUTTON ON/OFF SW6****//

//****BUTTON ON/OFF SW7****//
 BLYNK_WRITE(Widget_Btn_SW7){
      int valueSW7 = param.asInt();
      if(valueSW7 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node1.writeSingleRegister(0x07,0x0100);
      }
       else{                    
       pool_size1 = node1.writeSingleRegister(0x07,0x0200);
     }
} 
//****BUTTON ON/OFF SW7****//

//****BUTTON ON/OFF SW8****//
 BLYNK_WRITE(Widget_Btn_SW8){
      int valueSW8 = param.asInt();
      if(valueSW8 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node1.writeSingleRegister(0x08,0x0100);
      }
       else{                    
       pool_size1 = node1.writeSingleRegister(0x08,0x0200);
     }
} 

//****BUTTON ON/OFF SW8****//

//=====Blynk connected=====//
BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    Blynk.syncAll();

 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    pool_size1 = node1.writeSingleRegister(0x09,0x0100);
 }
}
//=====Blynk connected=====//

//=====Reconnect to blynk=====//
void reconnecting()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    Blynk.syncAll();
  }
}
//=====Reconnect to blynk=====//

//=======Loop function=======//
void loop() 
{
   if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();
}
//=======Loop function=======//
