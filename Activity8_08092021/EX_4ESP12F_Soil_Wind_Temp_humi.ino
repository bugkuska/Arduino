#define BLYNK_TEMPLATE_ID "TMPLVBo6WCZN"
#define BLYNK_DEVICE_NAME "ESP12FX4WSDL13 C6713"
#define BLYNK_FIRMWARE_VERSION        "0.1.0"

//*****MCU Digital Pin*******//
//Relay1_ledblynk       16
//Relay2_cfan           14
//Relay3_pump           12
//Relay4_ultra_foggy    13 
//MAX485 DI=TX          15
//MAX485_RE_NEG=DE      5
//MAX485_RE             4
//MAX485 RO=RX          2   
//*****MCU Digital Pin*******//

//*****Blynk Virtual Pin*****//
//V1  ปุ่ม เปิด-ปิด พัดลม220VAC
//V2  ปุ่ม เปิด-ปิด ปุ่ม เปิด-ปิดปั้ม 12VDC

//V5  Windspeed

//*****Blynk Virtual Pin*****//

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
#define MAX485_RE      4        //RE
#define MAX485_RE_NEG  5        //DE
// instantiate ModbusMaster object
ModbusMaster node1;
ModbusMaster node2;
ModbusMaster node3;
//======Modbus=====//

//==Define 4CH Relay==//
#define Relay1_ledblynk       16
#define Relay2_cfan           14
#define Relay3_pump           12
#define Relay4_ultra_foggy    13 
//==Define 4CH Relay==//

//==ปุ่ม เปิด-ปิด พัดลมระบายอากาศ==//
//Manual & Auto Switch cfan
//V11 Auto&Manual cfan
//V12 Slider cfan
#define Widget_Btn_btn2 V1  
//Slider for set WindSpeed limit
bool  switchStatus1 = 0; // 0 = manual,1=auto
int   WindLevelLimit1 = 0;
bool  manualSwitch1 = 0;
//==ปุ่ม เปิด-ปิด พัดลมระบายอากาศ==//
     
//==ปุ่ม เปิด-ปิดปั้มน้ำ==//
//Manual & Auto Switch Pump
//V13 Auto&Manual Pump
//V14 Slider Pump
#define Widget_Btn_btn2 V2      
//Slider for set Humi limit
bool  switchStatus2 = 0; // 0 = manual,1=auto
int   SoilLevelLimit2 = 0;
bool  manualSwitch2 = 0;
//==ปุ่ม เปิด-ปิดปั้มน้ำ==//

//==ปุ่ม เปิด-ปิด พ่นหมอกอัลตร้าโซนิค==//
//Manual & Auto Switch Ultra_foggy
//V15 Auto&Manual Ultra_foggy
//V16 Slider Ultra_foggy
#define Widget_Btn_btn3 V3      
//Slider for set Temperature limit
bool  switchStatus3 = 0; // 0 = manual,1=auto
int   TempLevelLimit3 = 0;
bool  manualSwitch3 = 0;
//==ปุ่ม เปิด-ปิด พ่นหมอกอัลตร้าโซนิค==//

//==Modbus Pre & Post Transmission1==//
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
//==Modbus Pre & Post Transmission1==//

//==Modbus Pre & Post Transmission2==//
void preTransmission2()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_RE, 1);
}
void postTransmission2()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_RE, 0);
}
//==Modbus Pre & Post Transmission2==//

//==Modbus Pre & Post Transmission3==//
void preTransmission3()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_RE, 1);
}
void postTransmission3()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_RE, 0);
}
//==Modbus Pre & Post Transmission3==//


//==Wi-Fi and blynk credentials====//   
char auth[] = "jZaep6yoIIM0gxswM7L9tf3fLkS49Urp";
char ssid[] = "smfthailand_trainning";                  //Wi-Fi                
char pass[] = "0814111142";                             //Password เชื่อมต่อ Wi-Fi                         
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
  
   //Modbus slave ID 
  node1.begin(1, mySerial);
  
  node2.begin(2, mySerial);
  
  node3.begin(3, mySerial);
  
  // Callbacks allow us to configure the RS485 transceiver correctly
  node1.preTransmission(preTransmission1);
  node1.postTransmission(postTransmission1);
  
  node2.preTransmission(preTransmission2);
  node2.postTransmission(postTransmission2);
  
  node3.preTransmission(preTransmission3);
  node3.postTransmission(postTransmission3);
  //End Modbus pinMode
 
  Serial.println();

  // Setup Pin Mode
  pinMode(Relay1_ledblynk,OUTPUT);              // NODEMCU PIN gpio16 
  pinMode(Relay2_cfan,OUTPUT);                  // NODEMCU PIN gpio14
  pinMode(Relay3_pump,OUTPUT);                  // NODEMCU PIN gpio12   
  pinMode(Relay4_ultra_foggy,OUTPUT);           // NODEMCU PIN GPIO13         
  
  // Set Defult Relay Status
  digitalWrite(Relay1_ledblynk,LOW);            // NODEMCU PIN gpio16
  digitalWrite(Relay2_cfan,LOW);                // NODEMCU PIN gpio14
  digitalWrite(Relay3_pump,LOW);                // NODEMCU PIN gpio12
  digitalWrite(Relay4_ultra_foggy,LOW);         // NODEMCU PIN gpio13 
  
  Blynk.begin(auth, ssid, pass); 
  timer.setInterval(1000L, wind_rs_n01);
  timer.setInterval(2000L, soil_01);
  timer.setInterval(3000L, DL13_Temp_Humi);
}
//=====Setup Function=====//

//=====================1.WindSpeed Sensor=====================//
//Update switchStatus1 on WindSpeed Sensor
BLYNK_WRITE(V11)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}

//Update WindSpeed setting
BLYNK_WRITE(V12)
{   
  WindLevelLimit1 = param.asInt(); // Get value as integer
}

//Update manualSwitch
BLYNK_WRITE(V1)
{
  manualSwitch1 = param.asInt();
}

//==ModbusRTU WindSpeed Sensor==//

//==Windspeed RS-FSJT-N01==//
void wind_rs_n01()
{
  uint8_t result;
  float wind01 = (node1.getResponseBuffer(0)); //Change m/s to km/h= m/s *3.6, Change km/h to m/s = km/h /3.6 
 
  Serial.println("Get Wind Speed Data");
   result = node1.readHoldingRegisters(0x0000, 1); // Read 2 registers starting at 1)
  if (result == node1.ku8MBSuccess)
  {
    Serial.print("Winspeed: ");
    Serial.println(node1.getResponseBuffer(0));
  }
  delay(1000);
  
  Blynk.virtualWrite(V4, wind01);

  if(switchStatus1)
  {
    // auto
    if(wind01 < WindLevelLimit1)
    {
        digitalWrite(Relay2_cfan, HIGH);  
        Blynk.virtualWrite(V1, 1); 
    }  
    else
    {
        digitalWrite(Relay2_cfan, LOW);
        Blynk.virtualWrite(V1, 0); 
    }
  }
  else
  {
    if(manualSwitch1)
    {
        digitalWrite(Relay2_cfan, HIGH);        

    }
    else
    {
        digitalWrite(Relay2_cfan, LOW);
    }
    // manaul
  }
}
//==Windspeed RS-FSJT-N01==//

//===================2.SoilMoisture Sensor===================//
//Update switchStatus2 on SoilMoisture 
BLYNK_WRITE(V13)
{   
  switchStatus2 = param.asInt(); // Get value as integer
}

//Update SoilMoisture setting
BLYNK_WRITE(V14)
{   
  SoilLevelLimit2 = param.asInt(); // Get value as integer
}

// Update manualSwitch2
BLYNK_WRITE(V2)
{
  manualSwitch2 = param.asInt();
}

//==SoilMoisture Sensor==//

void soil_01(){ 
  uint8_t result;  
  float soil_01 = (node2.getResponseBuffer(2)/10.0f);
  
  Serial.println("Get Soil Moisture Data");
  result = node2.readHoldingRegisters(0x0000, 3); // Read 2 registers starting at 1)
  if (result == node2.ku8MBSuccess)
  {
    Serial.print("Soil Moisture Sensor: ");
    Serial.println(node2.getResponseBuffer(2)/10.0f);
  }
  delay(1000);
  Blynk.virtualWrite(V5,soil_01);

if(switchStatus2)
  {
    // auto
    if(soil_01 < SoilLevelLimit2)
    {
        digitalWrite(Relay3_pump, HIGH);  
        Blynk.virtualWrite(V2, 1);
    }  
    else
    {
        digitalWrite(Relay3_pump, LOW);
        Blynk.virtualWrite(V2, 0);
    }
  }
  else
  {
    if(manualSwitch2)
    {
        digitalWrite(Relay3_pump, HIGH); 
    }
    else
    {
        digitalWrite(Relay3_pump, LOW);
    }
    // manaul
  }
}
//==SoilMoisture Sensor==//
//===================2.SoilMoisture Sensor===================//

//====================3.DL13 Temp & Humi====================//
//Update switchStatus3 on Temperature 
BLYNK_WRITE(V15)
{   
  switchStatus3 = param.asInt(); // Get value as integer
}

//Update Temperature setting
BLYNK_WRITE(V16)
{   
  TempLevelLimit3 = param.asInt(); // Get value as integer
}

//Update manualSwitch3
BLYNK_WRITE(V3)
{
  manualSwitch3 = param.asInt();
}

//=========DL13 Temp & Humi========//
void DL13_Temp_Humi()
{
  uint8_t result;
  float temp = (node3.getResponseBuffer(0)/10.0f);
  float humi = (node3.getResponseBuffer(1)/10.0f);

 
  Serial.println("Get Temp&Humi Data");
  result = node3.readInputRegisters(0x040A, 2); // Read 2 registers starting at 1)
  if (result == node3.ku8MBSuccess)
  {
    Serial.print("Temp: ");
    Serial.println(node3.getResponseBuffer(0)/10.0f);
    Serial.print("Humi: ");
    Serial.println(node3.getResponseBuffer(1)/10.0f);
  }
  delay(3000);
  Blynk.virtualWrite(V6, temp);
  Blynk.virtualWrite(V7, humi);
  
  if(switchStatus3)
  {
    // auto
    if(temp > TempLevelLimit3)
    {
        digitalWrite(Relay4_ultra_foggy, HIGH);  
        Blynk.virtualWrite(V3, 1);
    }  
    else
    {
        digitalWrite(Relay4_ultra_foggy, LOW);
        Blynk.virtualWrite(V3, 0);
    }
  }
  else
  {
    if(manualSwitch3)
    {
        digitalWrite(Relay4_ultra_foggy, HIGH); 
    }
    else
    {
        digitalWrite(Relay4_ultra_foggy, LOW);
    }
    // manaul
  }
}
//=========DL13 Temp & Humi========//
//====================3.DL13 Temp & Humi====================//

//=====Blynk connected=====//
BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    Blynk.syncAll();

 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(Relay1_ledblynk, HIGH);
 }
}

//=====Blynk connected=====//

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
