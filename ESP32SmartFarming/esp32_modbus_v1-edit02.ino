//============Libraries Need for project============//
#include <BlynkSimpleEsp32.h>           
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
int blynkIsDownCount = 0;
BlynkTimer timer;
#include <SimpleTimer.h>
SimpleTimer timer1;
//============Libraries Need for project============//

//=================Blynk Virtual Pin================//
//V0          ไฟสถานะปุ่ม SW1 คุมปั้ม 220VAC
//V1          ปุ่ม เปิด-ปิด SW1 คุมปั้ม 220VAC
//V2          ปุ่ม เปิด-ปิด SW1 คุมปั้ม 12V ตัวที่ 1
//V3          ปุ่ม เปิด-ปิด SW2 คุมปั้ม 12V ตัวที่ 2
//V4          ปุ่ม เปิด-ปิด SW3 คุมปั้ม 12V ตัวที่ 3
//V5          ปุ่ม เปิด-ปิด SW4 คุมปั้ม 12V ตัวที่ 4
//V6          ปุ่ม เปิด-ปิด SW5 คุมปั้ม 12V ตัวที่ 5
//V7          ปุ่ม เปิด-ปิด SW6 คุมปั้ม 12V ตัวที่ 6

//V30         ไฟสถานะปุ่ม วาล์ว 12V ตัวที่ 1
//V8          เปิด-ปิด วาล์ว 12V ตัวที่ 1
//V25         Soil Moistuer Sensor
//V61         Auto&Manual Valve1 and Pump
//V62         Slider SoilMoisture Sensor

//V31         ไฟสถานะปุ่ม วาล์ว 12V ตัวที่ 2
//V9          ปุ่ม เปิด-ปิด วาล์ว 12V ตัวที่ 2
//V24         GZWS-N01 Light
//V63         Auto&Manual Valve2 
//V64         Slider Lux Sensor

//V32         ไฟสถานะปุ่ม วาล์ว 12V ตัวที่ 3
//V10         ปุ่ม เปิด-ปิด วาล์ว 12V ตัวที่ 3
//V22         GZWS-N01 Humidity
//V23         GZWS Temperature Sensor
//V65         Auto&Manual Valve3 
//V66         Slider GZWS Temperature Sensor

//V33         ไฟสถานะปุ่ม วาล์ว 12V ตัวที่ 4
//V11         ปุ่ม เปิด-ปิด วาล์ว 12V ตัวที่ 4

//V12         ไฟสถานะปุ่ม พัดลมระบายอากาศภายในตู้ควบคุม
//V13         ปุ่ม เปิด-ปิด พัดลมระบายอากาศภายในตู้ควบคุม
//V20         WTR10E Temperature
//V21         WTR10E Humidity
//V59         Auto&Manual Cooling Fan1
//V60         Slider for WTR10-E Temp setting
//=================Blynk Virtual Pin================//

//============ Wi-Fi & Blynk Credentials============//
//=====Wi-Fi & Blynk Credential=====//
char ssid[] = "smfthailand_trainning";                   //ชื่อ SSID ที่เราต้องการเชื่อมต่อ        
char pass[] = "0814111142";                   //รหัสผ่าน WI-FI
char auth[] = "";                   //Auth token from blynk app 
//============ Wi-Fi & Blynk Credentials============//

//=============Define ESP32 Digital Pin=============//
#define Relay1_Pump220      26
#define Relay2_LedBlynk     25
#define Relay3_LedFan       33
//Relay4_btn4               32
//=============Define ESP32 Digital Pin=============//

//=========Pool size for Modbus Write command=======//
int8_t pool_size1;
//=========Pool size for Modbus Write command=======//

//============Define Blynk Virtual Pin==============//
//=====Pump 220VAC=======//
#define Widget_LED_SW1_Pump220 V0         //ไฟสถานะปุ่ม SW1 คุมปั้ม 220VAC
#define Widget_Btn_SW1_Pump220 V1         //ปุ่ม เปิด-ปิด SW1 คุมปั้ม 220VAC
WidgetLED LedBlynkPump220(Widget_LED_SW1_Pump220);
//=====Pump 220VAC=======//

//======Pump 12VDC=======//
#define Widget_Btn_SW1_Pump12V1 V2        //ปุ่ม เปิด-ปิด SW1 คุมปั้ม 12V ตัวที่ 1
#define Widget_Btn_SW2_Pump12V2 V3        //ปุ่ม เปิด-ปิด SW2 คุมปั้ม 12V ตัวที่ 2
#define Widget_Btn_SW3_Pump12V3 V4        //ปุ่ม เปิด-ปิด SW3 คุมปั้ม 12V ตัวที่ 3
#define Widget_Btn_SW4_Pump12V4 V5        //ปุ่ม เปิด-ปิด SW4 คุมปั้ม 12V ตัวที่ 4
#define Widget_Btn_SW5_Pump12V5 V6        //ปุ่ม เปิด-ปิด SW5 คุมปั้ม 12V ตัวที่ 5
#define Widget_Btn_SW6_Pump12V6 V7        //ปุ่ม เปิด-ปิด SW6 คุมปั้ม 12V ตัวที่ 6
//======Pump 12VDC=======//

//=======Valve 12VDC=====//
//========Valve1=========//
#define Widget_LED_SW7_Valve1 V30           //ไฟสถานะปุ่ม วาล์ว 12V ตัวที่ 1
#define Widget_Btn_SW7_Valve1 V8            //ปุ่ม เปิด-ปิด วาล์ว 12V ตัวที่ 1
WidgetLED LedBlynkValve1(Widget_LED_SW7_Valve1);
bool switchStatus2 = 0;                     // 0 = manual,1=auto
int SoilsensorLimit2 = 0;
bool manualSwitch2 = 0;
//V25 Soil Moistuer Sensor
//V61 Auto&Manual Valve1 and Pump
//V62 Slider SoilMoisture Sensor
//========Valve1=========//

//========Valve2=========//
#define Widget_LED_SW8_Valve2 V31           //ไฟสถานะปุ่ม วาล์ว 12V ตัวที่ 2
#define Widget_Btn_SW8_Valve2 V9            //ปุ่ม เปิด-ปิด วาล์ว 12V ตัวที่ 2
WidgetLED LedBlynkValve2(Widget_LED_SW8_Valve2);
bool switchStatus3 = 0;                   // 0 = manual,1=auto
int LuxsensorLimit3 = 0;
bool manualSwitch3 = 0;
//V24 Lux Sensor
//V63 Auto&Manual Valve2 
//V64 Slider Lux Sensor
//========Valve2=========//

//========Valve3=========//
#define Widget_LED_SW9_Valve3 V32           //ไฟสถานะปุ่ม วาล์ว 12V ตัวที่ 3
#define Widget_Btn_SW9_Valve3 V10           //ปุ่ม เปิด-ปิด วาล์ว 12V ตัวที่ 3
WidgetLED LedBlynkValve3(Widget_LED_SW9_Valve3);
bool switchStatus4 = 0;                     // 0 = manual,1=auto
int TempsensorLimit4 = 0;
bool manualSwitch4 = 0;
//V23 GZWS Temperature Sensor
//V65 Auto&Manual Valve3 
//V66 Slider GZWS Temperature Sensor
//========Valve3=========//

//========Valve4=========//
#define Widget_LED_SW10_Valve4 V33            //ไฟสถานะปุ่ม วาล์ว 12V ตัวที่ 4
#define Widget_Btn_SW10_Valve4 V11            //ปุ่ม เปิด-ปิด วาล์ว 12V ตัวที่ 4
WidgetLED LedBlynkValve4(Widget_LED_SW10_Valve4);
//========Valve4=========//

//=======Valve 12VDC=====//

//======Cooling Fan======//
#define Widget_LED_Fan1 V12              //ไฟสถานะปุ่ม พัดลมระบายอากาศภายในตู้ควบคุม
#define Widget_Btn_Fan1 V13              //ปุ่ม เปิด-ปิด พัดลมระบายอากาศภายในตู้ควบคุม
WidgetLED LedBlynkFan1(Widget_LED_Fan1);
bool switchStatus1 = 0;                   // 0 = manual,1=auto
int tempsensorLimit1 = 0;
bool manualSwitch1 = 0;
//V20 WTR10E Temperature
//V21 WTR10E Humidity
//V59 Auto&Manual Cooling Fan1
//V60 Slider for WTR10-E Temp setting
//======Cooling Fan======//

//V22 GZWS-N01 Humidity
//V23 GZWS-N01 Temperature
//V24 GZWS-N01 Light
//V25 Soil Moisture Sensor

//====Modbus Libraries====//
#include <ModbusMaster.h>
#define MAX485_DE      18   //DE
#define MAX485_RE_NEG  19   //RE
#define RX2 16              //RO
#define TX2 17              //DI
//====Modbus Libraries====//

//==instantiate ModbusMaster object==//
ModbusMaster node1;       //Modbus RTU 4CH Relay1 Slave ID1
ModbusMaster node2;       //Modbus RTU 4CH Relay2 Slave ID2
ModbusMaster node3;       //Modbus RTU 4CH Relay3 Slave ID3
ModbusMaster node4;       //WTR10E Temperature&Humidity Slave ID4
ModbusMaster node5;       //GZWS-N01 Humidity&Temperature and Light Slave ID5
ModbusMaster node6;       //Modbus RTU Soil Moisture Sensor Slave ID6
//==instantiate ModbusMaster object==//

//======pre&post transmission1=======//
void preTransmission1()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission1()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}
//======pre&post transmission1=======//

//======pre&post transmission2=======//
void preTransmission2()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission2()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}
//======pre&post transmission2=======//

//======pre&post transmission3=======//
void preTransmission3()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission3()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}
//======pre&post transmission3=======//

//======pre&post transmission4=======//
void preTransmission4()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission4()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}
//======pre&post transmission4=======//

//======pre&post transmission5=======//
void preTransmission5()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission5()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}
//======pre&post transmission5=======//

//======pre&post transmission6=======//
void preTransmission6()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission6()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}
//======pre&post transmission6=======//

//==============================Setup Function============================//
void setup()
{
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);   //RX2=16,RO ,TX2=17, DI

  //Set up pinMode
  pinMode(Relay1_Pump220,OUTPUT);
  pinMode(Relay2_LedBlynk,OUTPUT);
  pinMode(Relay3_LedFan,OUTPUT);

  //Set default status
  digitalWrite(Relay1_Pump220,LOW);
  digitalWrite(Relay2_LedBlynk,LOW);
  digitalWrite(Relay3_LedFan,LOW);
   
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
  
  //=====Modbus slave ID=====// 
  node1.begin(1, Serial2); //Modbus RTU 4CH Relay1
  node2.begin(2, Serial2); //Modbus RTU 4CH Relay2
  node3.begin(3, Serial2); //Modbus RTU 4CH Relay3
  node4.begin(4, Serial2); //WTR10E Temperature&Humidity
  node5.begin(5, Serial2); //GZWS-N01 Humidity&Temperature and Light
  node6.begin(6, Serial2); //Modbus RTU Soil Moisture Sensor
  //=====Modbus slave ID=====// 
  
  // Callbacks allow us to configure the RS485 transceiver correctly
  //node1 for Modbus RTU 4CH Relay1
  node1.preTransmission(preTransmission1);
  node1.postTransmission(postTransmission1);
  
  //node2 for Modbus RTU 4CH Relay2
  node2.preTransmission(preTransmission2);
  node2.postTransmission(postTransmission2);
  
  //node3 for Modbus RTU 4CH Relay3
  node3.preTransmission(preTransmission3);
  node3.postTransmission(postTransmission3);
  
  //node4 for WTR10E Temperature&Humidity 
  node4.preTransmission(preTransmission4);
  node4.postTransmission(postTransmission4);
  
  //node5 for GZWS-N01 Humidity&Temperature and Light
  node5.preTransmission(preTransmission5);
  node5.postTransmission(postTransmission5);
  
  //node6 for Modbus RTU Soil Moisture Sensor
  node6.preTransmission(preTransmission6);
  node6.postTransmission(postTransmission6);
  
  Serial.println();
  
  //Begin connect to blynk-cloud.com
  Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  timer.setInterval(30000L, reconnecting); 
  timer.setInterval(5000L, wtr10e_01);
  timer.setInterval(10000L, gzws_lux);
  timer.setInterval(5000L, gzws_temp);
  timer.setInterval(5000L, soil_01);  
}
//==============================Setup Function============================//

//===========================WTR10E-01 Slave ID4==========================//
//=====Update switchStatus1 on WTR10E-01=====//
BLYNK_WRITE(V59)
{   
  switchStatus1 = param.asInt();            // Get value as integer
}
//=====Update switchStatus1 on WTR10E-01=====//

//=====Update WTR10E-01 setting=====//
BLYNK_WRITE(V60)
{   
  tempsensorLimit1 = param.asInt();         // Get value as integer
}
//=====Update WTR10E-01 setting=====//

//=====Update manualSwitch======//
BLYNK_WRITE(V13)
{
  manualSwitch1 = param.asInt();
}
//=====Update manualSwitch======//

//=======WTR10E-01 Slave ID4========//
void wtr10e_01(){
  uint8_t result1;
 float temp1 = (node4.getResponseBuffer(0)/10.0f);
 float humi1 = (node4.getResponseBuffer(1)/10.0f);
 
  Serial.println("WTR10E-01 Temp&Humi data1");
  result1 = node4.readHoldingRegisters(0x0000, 2); // Read 2 registers starting at 1)
  if (result1 == node4.ku8MBSuccess)
  {
    Serial.print("Temp1: ");
    Serial.println(node4.getResponseBuffer(0)/10.0f);
    Serial.print("Humi1: ");
    Serial.println(node4.getResponseBuffer(1)/10.0f);
  }
  //delay(1000);
   
  Blynk.virtualWrite(V20, temp1);
  Blynk.virtualWrite(V21, humi1);
  
  if(switchStatus1)
  {
    // auto
    if(temp1 > tempsensorLimit1)              //ถ้าอุณหภูมิมากกว่าค่าที่เรากำหนดไว้บนสไลด์เดอร์ เงื่อนไขเป็นจริง
    {
        digitalWrite(Relay3_LedFan, HIGH);    
        Blynk.virtualWrite(V13, 1);                
        Blynk.setProperty(Widget_LED_Fan1, "color", "#C70039");
        Blynk.setProperty(Widget_LED_Fan1, "label", "เปิดพัดลม");
        LedBlynkFan1.on(); 
    }  
    else
    {
        digitalWrite(Relay3_LedFan, LOW); 
        Blynk.virtualWrite(V13, 0);
        Blynk.virtualWrite(Widget_LED_Fan1, 0);
        Blynk.setProperty(Widget_LED_Fan1, "label", "ปิดพัดลม");                       
        LedBlynkFan1.off();  
    }
  }
  else
  {
    if(manualSwitch1)
    {
        digitalWrite(Relay3_LedFan, HIGH);         
        Blynk.setProperty(Widget_LED_Fan1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_Fan1, "label", "เปิดพัดลม");
        LedBlynkFan1.on(); 
    }
    else
    {
        digitalWrite(Relay3_LedFan, LOW);  
        Blynk.setProperty(Widget_LED_Fan1, "label", "ปิดพัดลม");                       
        LedBlynkFan1.off(); 
    }
    // manaul
  }
}
//===========================WTR10E-01 Slave ID4==========================//

//====================Soil Moisture Sensor Slave ID6 =====================//
//=====Update switchStatus2 on Soil Moisture Sensor=====//
BLYNK_WRITE(V61)
{   
  switchStatus2 = param.asInt();            // Get value as integer
}
//=====Update switchStatus2 on Soil Moisture Sensor=====//

//=====Update Soil Moisture Sensor setting=====//
BLYNK_WRITE(V62)
{   
  SoilsensorLimit2 = param.asInt();         // Get value as integer
}
//=====Update Soil Moisture Sensor setting=====//

//=====Update manualSwitch2======//
BLYNK_WRITE(V8)
{
  manualSwitch2 = param.asInt();
}
//=====Update manualSwitch2======//

void soil_01(){ 
  uint8_t result;  
  float soil_01 = (node6.getResponseBuffer(2)/10.0f);
  
  Serial.println("Get Soil Moisture Data");
  result = node6.readHoldingRegisters(0x0000, 3); // Read 2 registers starting at 1)
  if (result == node6.ku8MBSuccess)
  {
    Serial.print("Soil Moisture Sensor: ");
    Serial.println(node6.getResponseBuffer(2)/10.0f);
  }
  delay(1000);
  Blynk.virtualWrite(V25,soil_01);

  if(switchStatus2)
  {
    // auto
    if(soil_01 < SoilsensorLimit2)              //ถ้าค่าความชื้นในดินน้อยกว่าค่าที่เรากำหนดไว้บนสไลด์เดอร์ เงื่อนไขเป็นจริง
    {
        //==Valve1==//
        //digitalWrite(Relay3_LedFan, HIGH);    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node3.writeSingleRegister(0x01,0x0100);
        Blynk.virtualWrite(V8, 1);                
        Blynk.setProperty(Widget_LED_SW7_Valve1, "color", "#C70039");
        Blynk.setProperty(Widget_LED_SW7_Valve1, "label", "เปิดวาล์ว1");
        LedBlynkValve1.on(); 
        //==Valve1==//
        
        //==Pump220==//
        digitalWrite(Relay1_Pump220,HIGH);
        Blynk.virtualWrite(Widget_Btn_SW1_Pump220,1);   
        Blynk.virtualWrite(V1, 1);                
        Blynk.setProperty(Widget_LED_SW1_Pump220 , "color", "#C70039");
        Blynk.setProperty(Widget_LED_SW1_Pump220 , "label", "เปิดปั้มน้ำ220VAC");
        LedBlynkPump220.on(); 
       //==Pump220==// 
    }  
    else
    {
        //==Valve1==//
        //digitalWrite(Relay3_LedFan, LOW); 
        //Modbus command to ON/OFF Relay           
        pool_size1 = node3.writeSingleRegister(0x01,0x0200);
        Blynk.virtualWrite(V8, 0);
        Blynk.virtualWrite(Widget_LED_SW7_Valve1, 0);
        Blynk.setProperty(Widget_LED_SW7_Valve1, "label", "ปิดวาล์ว1");                       
        LedBlynkValve1.off(); 
        //==Valve1==//  
        //==Pump220==//
        digitalWrite(Relay1_Pump220,LOW);
        Blynk.virtualWrite(Widget_Btn_SW1_Pump220,0); 
        Blynk.virtualWrite(V1, 0);
        Blynk.virtualWrite(Widget_LED_SW1_Pump220, 0);
        Blynk.setProperty(Widget_LED_SW1_Pump220, "label", "ปิดปั้มน้ำ220VAC");                       
        LedBlynkPump220.off();  
        //==Pump220==//
    }
  }
  else
  {
    if(manualSwitch2)
    {
        //==Valve1==//
        //digitalWrite(Relay3_LedFan, HIGH);  
        pool_size1 = node3.writeSingleRegister(0x01,0x0100);       
        Blynk.setProperty(Widget_LED_SW7_Valve1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_SW7_Valve1, "label", "เปิดวาล์ว1");
        LedBlynkValve1.on(); 
        //==Valve1==//
        //==Pump220==//
        digitalWrite(Relay1_Pump220,HIGH);       
        Blynk.virtualWrite(Widget_Btn_SW1_Pump220,1);  
        Blynk.setProperty(Widget_LED_SW1_Pump220, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_SW1_Pump220, "label", "เปิดปั้มน้ำ220VAC");
        LedBlynkPump220.on(); 
        //==Pump220==//
    }
    else
    {
        //==Valve1==//
        //digitalWrite(Relay3_LedFan, LOW); 
        pool_size1 = node3.writeSingleRegister(0x01,0x0200); 
        Blynk.setProperty(Widget_LED_SW7_Valve1, "label", "ปิดวาล์ว1");                       
        LedBlynkValve1.off(); 
        //==Valve1==//
        //==Pump220==//
        digitalWrite(Relay1_Pump220,LOW);  
        Blynk.virtualWrite(Widget_Btn_SW1_Pump220,0);
        Blynk.setProperty(Widget_LED_SW1_Pump220, "label", "ปิดปั้มน้ำ220VAC");                       
        LedBlynkPump220.off(); 
        //==Pump220==//
    }
    // manaul
  } 
}
//====================Soil Moisture Sensor Slave ID6 =====================//

//==================GZWS Lux Sensior Slave ID 5===========================//
BLYNK_WRITE(V63)
{   
  switchStatus3 = param.asInt();            // Get value as integer
}
//=====Update switchStatus3 on Lux Sensor=====//

//=====Update Lux Sensor setting=====//
BLYNK_WRITE(V64)
{   
  LuxsensorLimit3 = param.asInt();         // Get value as integer
}
//=====Update Lux Sensor setting=====//

//=====Update manualSwitch3======//
BLYNK_WRITE(V9)
{
  manualSwitch3 = param.asInt();
}
//=====Update manualSwitch3======//

void gzws_lux(){
  uint8_t result; 
  float gzws_light = (node5.getResponseBuffer(2));
  float light_percentage;
  
  Serial.println("Get GZWS Lux Data");
  result = node5.readHoldingRegisters(0x0000, 3); // Read 2 registers starting at 1)
  if (result == node5.ku8MBSuccess)
  {
    light_percentage = (gzws_light = node5.getResponseBuffer(2));
    light_percentage = map(light_percentage, 0,65535, 0,100);
    
    Serial.print("Light: ");
    Serial.println(light_percentage);
    //Serial.println(node5.getResponseBuffer(2));
  }
  delay(1000);
  Blynk.virtualWrite(V24,light_percentage);


  if(switchStatus3)
  {
    // auto
    if(light_percentage < LuxsensorLimit3)              //ถ้าค่าความเข้มแสงน้อยกว่าค่าที่เรากำหนดไว้บนสไลด์เดอร์ เงื่อนไขเป็นจริง
    {
        //==Valve2==//
        //Modbus command to ON/OFF Relay           
        pool_size1 = node3.writeSingleRegister(0x02,0x0100);
        Blynk.virtualWrite(V9, 1);                
        Blynk.setProperty(Widget_LED_SW8_Valve2, "color", "#C70039");
        Blynk.setProperty(Widget_LED_SW8_Valve2, "label", "เปิดวาล์ว2");
        LedBlynkValve2.on(); 
        //==Valve2==//
    }  
    else
    {
        //==Valve2==//
        //Modbus command to ON/OFF Relay    
        pool_size1 = node3.writeSingleRegister(0x02,0x0200);       
        Blynk.virtualWrite(V9, 0);
        Blynk.virtualWrite(Widget_LED_SW8_Valve2, 0);
        Blynk.setProperty(Widget_LED_SW8_Valve2, "label", "ปิดวาล์ว2");                       
        LedBlynkValve2.off(); 
        //==Valve2==//  
    }
  }
  else
  {
    if(manualSwitch3)
    {
        //==Valve2==//
        pool_size1 = node3.writeSingleRegister(0x02,0x0100);     
        Blynk.setProperty(Widget_LED_SW8_Valve2, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_SW8_Valve2, "label", "เปิดวาล์ว2");
        LedBlynkValve2.on(); 
        //==Valve2==//
    }
    else
    {
        //==Valve2==//
        pool_size1 = node3.writeSingleRegister(0x02,0x0200);
        Blynk.setProperty(Widget_LED_SW8_Valve2, "label", "ปิดวาล์ว2");                       
        LedBlynkValve2.off(); 
        //==Valve2==//
    }
    // manaul
  } 
}
//==================GZWS Lux Sensior Slave ID 5===========================//

//=====================GZWS Humi&Temp Slave ID 5==========================//
BLYNK_WRITE(V65)
{   
  switchStatus4 = param.asInt();            // Get value as integer
}
//=====Update switchStatus4 on Temp=====//

//=====Update Temp Sensor setting=====//
BLYNK_WRITE(V66)
{   
  TempsensorLimit4 = param.asInt();         // Get value as integer
}
//=====Update Temp Sensor setting=====//

//=====Update manualSwitch4======//
BLYNK_WRITE(V10)
{
  manualSwitch4 = param.asInt();
}
//=====Update manualSwitch4======//
void gzws_temp(){
  uint8_t result; 
  float gzws_humidity = (node5.getResponseBuffer(0)/10.0f);
  float gzws_temperature = (node5.getResponseBuffer(1)/10.0f);  
  Serial.println("Get GZWS Temp&Humi Data");
  result = node5.readHoldingRegisters(0x0000, 3); // Read 2 registers starting at 1)
  if (result == node5.ku8MBSuccess)
  {
    Serial.print("Humidity: ");
    Serial.println(node5.getResponseBuffer(0)/10.0f);
    Serial.print("Temperature: ");
    Serial.println(node5.getResponseBuffer(1)/10.0f);
  }
  delay(1000);
  Blynk.virtualWrite(V22,gzws_humidity);
  Blynk.virtualWrite(V23,gzws_temperature);

   if(switchStatus4)
  {
    // auto
    if(gzws_temperature > TempsensorLimit4)              //ถ้าค่าอุณหภูมิมากกว่าค่าที่เรากำหนดไว้บนสไลด์เดอร์ เงื่อนไขเป็นจริง
    {
        //==Valve3==//
        //Modbus command to ON/OFF Relay           
        pool_size1 = node3.writeSingleRegister(0x03,0x0100);
        Blynk.virtualWrite(V10, 1);                
        Blynk.setProperty(Widget_LED_SW9_Valve3, "color", "#C70039");
        Blynk.setProperty(Widget_LED_SW9_Valve3, "label", "เปิดวาล์ว3");
        LedBlynkValve3.on(); 
        //==Valve3==//

        //==Valve4==//
        //Modbus command to ON/OFF Relay           
        pool_size1 = node3.writeSingleRegister(0x04,0x0100);
        Blynk.virtualWrite(V11, 1);                
        Blynk.setProperty(Widget_LED_SW10_Valve4, "color", "#C70039");
        Blynk.setProperty(Widget_LED_SW10_Valve4, "label", "เปิดวาล์ว4");
        LedBlynkValve4.on(); 
        //==Valve4==//
    }  
    else
    {
        //==Valve3==//
        //Modbus command to ON/OFF Relay     
        pool_size1 = node3.writeSingleRegister(0x03,0x0200);     
        Blynk.virtualWrite(V10, 0);
        Blynk.virtualWrite(Widget_LED_SW9_Valve3, 0);
        Blynk.setProperty(Widget_LED_SW9_Valve3, "label", "ปิดวาล์ว3");                       
        LedBlynkValve3.off(); 
        //==Valve3==//  

        //==Valve4==//
        //Modbus command to ON/OFF Relay     
        pool_size1 = node3.writeSingleRegister(0x04,0x0200);     
        Blynk.virtualWrite(V11, 0);
        Blynk.virtualWrite(Widget_LED_SW10_Valve4, 0);
        Blynk.setProperty(Widget_LED_SW10_Valve4, "label", "ปิดวาล์ว4");                       
        LedBlynkValve4.off(); 
        //==Valve4==// 
    }
  }
  else
  {
    if(manualSwitch4)
    {
        //==Valve3==//
        pool_size1 = node3.writeSingleRegister(0x03,0x0100);         
        Blynk.setProperty(Widget_LED_SW9_Valve3, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_SW9_Valve3, "label", "เปิดวาล์ว3");
        LedBlynkValve3.on(); 
        //==Valve3==//

        //==Valve4==//
        pool_size1 = node3.writeSingleRegister(0x04,0x0100);         
        Blynk.setProperty(Widget_LED_SW10_Valve4, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_SW10_Valve4, "label", "เปิดวาล์ว4");
        LedBlynkValve4.on(); 
        //==Valve4==//
    }
    else
    {
        //==Valve3==//
        pool_size1 = node3.writeSingleRegister(0x03,0x0200);   
        Blynk.setProperty(Widget_LED_SW9_Valve3, "label", "ปิดวาล์ว3");                       
        LedBlynkValve3.off(); 
        //==Valve3==//

        //==Valve4==//
        pool_size1 = node3.writeSingleRegister(0x04,0x0200);   
        Blynk.setProperty(Widget_LED_SW10_Valve4, "label", "ปิดวาล์ว4");                       
        LedBlynkValve4.off(); 
        //==Valve4==//
    }
    // manaul
  } 
  
}
//=====================GZWS Humi&Temp Slave ID 5==========================//

/*
//======BUTTON ON/OFF SW1 220VAC ======//
 BLYNK_WRITE(Widget_Btn_SW1_Pump220){
      int valueSW1_Pump220 = param.asInt();
      if(valueSW1_Pump220 == 1){    
        digitalWrite(Relay1_Pump220,HIGH);
        Blynk.virtualWrite(Widget_Btn_SW1_Pump220,1);
        Blynk.setProperty(Widget_LED_SW1_Pump220, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_SW1_Pump220, "label", "ปั้มน้ำกำลังทำงาน");
        LedBlynkPump220.on(); 
      }
       else{                    
        digitalWrite(Relay1_Pump220,LOW);
        Blynk.virtualWrite(Widget_Btn_SW1_Pump220,0);                
        Blynk.setProperty(Widget_LED_SW1_Pump220, "label", "ปิดปั้มน้ำแล้ว");
        LedBlynkPump220.off(); 
     }
} 
//======BUTTON ON/OFF SW1 220VAC ======//
*/

//====BUTTON ON/OFF SW1 12VDC====//
 BLYNK_WRITE(Widget_Btn_SW1_Pump12V1){
      int valueSW1 = param.asInt();
      if(valueSW1 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node1.writeSingleRegister(0x01,0x0100);
      }
       else{                    
       pool_size1 = node1.writeSingleRegister(0x01,0x0200);
     }
} 
//====BUTTON ON/OFF SW1 12VDC====//

//====BUTTON ON/OFF SW2 12VDC====//
 BLYNK_WRITE(Widget_Btn_SW2_Pump12V2){
      int valueSW2 = param.asInt();
      if(valueSW2 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node1.writeSingleRegister(0x02,0x0100);
      }
       else{                    
       pool_size1 = node1.writeSingleRegister(0x02,0x0200);
     }
} 
//====BUTTON ON/OFF SW2 12VDC====//

//====BUTTON ON/OFF SW3 12VDC====//
 BLYNK_WRITE(Widget_Btn_SW3_Pump12V3){
      int valueSW3 = param.asInt();
      if(valueSW3 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node1.writeSingleRegister(0x03,0x0100); 
      }
       else{                    
       pool_size1 = node1.writeSingleRegister(0x03,0x0200);
     }
} 
//====BUTTON ON/OFF SW3 12VDC====//

//====BUTTON ON/OFF SW4 12VDC====//
 BLYNK_WRITE(Widget_Btn_SW4_Pump12V4){
      int valueSW4 = param.asInt();
      if(valueSW4 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node2.writeSingleRegister(0x01,0x0100);
      }
       else{                    
       pool_size1 = node2.writeSingleRegister(0x01,0x0200);
     }
} 
//====BUTTON ON/OFF SW4 12VDC====//

//====BUTTON ON/OFF SW5 12VDC====//
 BLYNK_WRITE(Widget_Btn_SW5_Pump12V5){
      int valueSW5 = param.asInt();
      if(valueSW5 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node2.writeSingleRegister(0x02,0x0100);
      }
       else{                    
       pool_size1 = node2.writeSingleRegister(0x02,0x0200);
     }
} 
//====BUTTON ON/OFF SW5 12VDC====//

//====BUTTON ON/OFF SW6 12VDC====//
 BLYNK_WRITE(Widget_Btn_SW6_Pump12V6){
      int valueSW6 = param.asInt();
      if(valueSW6 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node2.writeSingleRegister(0x03,0x0100);
      }
       else{                    
       pool_size1 = node2.writeSingleRegister(0x03,0x0200);
     }
} 
//====BUTTON ON/OFF SW6 12VDC====//

/*
//====BUTTON ON/OFF SW7 Valve1====//
 BLYNK_WRITE(Widget_Btn_SW7_Valve1){
      int valueSW7 = param.asInt();
      if(valueSW7 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node3.writeSingleRegister(0x01,0x0100);
      }
       else{                    
       pool_size1 = node3.writeSingleRegister(0x01,0x0200);
     }
} 
//====BUTTON ON/OFF SW7 Valve1====//
*/
/*
//====BUTTON ON/OFF SW8 Valve2====//
 BLYNK_WRITE(Widget_Btn_SW8_Valve2){
      int valueSW8 = param.asInt();
      if(valueSW8 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node3.writeSingleRegister(0x02,0x0100);
      }
       else{                    
       pool_size1 = node3.writeSingleRegister(0x02,0x0200);
     }
} 
//====BUTTON ON/OFF SW8 Valve2====//
*/
/*
//====BUTTON ON/OFF SW9 Valve3====//
 BLYNK_WRITE(Widget_Btn_SW9_Valve3){
      int valueSW9 = param.asInt();
      if(valueSW9 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node3.writeSingleRegister(0x03,0x0100);
      }
       else{                    
       pool_size1 = node3.writeSingleRegister(0x03,0x0200);
     }
} 
//====BUTTON ON/OFF SW9 Valve3====//

//====BUTTON ON/OFF SW10 Valve4====//
 BLYNK_WRITE(Widget_Btn_SW10_Valve4){
      int valueSW10 = param.asInt();
      if(valueSW10 == 1){    
        //Modbus command to ON/OFF Relay           
        pool_size1 = node3.writeSingleRegister(0x04,0x0100);
      }
       else{                    
       pool_size1 = node3.writeSingleRegister(0x04,0x0200);
     }
} 
//====BUTTON ON/OFF SW10 Valve4====//
*/

//=====Blynk connected=====//
BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    Blynk.syncAll();

 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(Relay2_LedBlynk,HIGH);
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
    digitalWrite(Relay2_LedBlynk,HIGH);
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
