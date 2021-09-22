#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_FIRMWARE_VERSION        "0.1.0"

//*****MCU Digital Pin*******//
//Relay1_ledblynk       16
//Relay2_cfan           14
//Relay3_dcpump         12
//Relay4                13 //ยังไม่ต่อใช้งาน
//MAX485 DI=TX          15
//MAX485_RE_NEG=DE      5
//MAX485_RE             4
//MAX485 RO=RX          2   
//ว่าง                    0
//*****MCU Digital Pin*******//

//*****Blynk Virtual Pin*****//
//V1  ปุ่ม เปิด-ปิด พัดลม220VAC
//V2  ปุ่ม เปิด-ปิด ปุ่ม เปิด-ปิดปั้ม 12VDC

//V5  Temperature
//V6  Humidity

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

#define MAX485_RE      4  //RE
#define MAX485_RE_NEG  5  //DE

// instantiate ModbusMaster object
ModbusMaster node;
//======Modbus=====//

#define Relay1_ledblynk       16

//ปุ่ม เปิด-ปิด พัดลม220VAC
//V11 Auto&Manual cfan
//V12 Slider cfan
//Manual & Auto Switch cfan
#define Relay2_cfan   14
#define Widget_Btn_btn2 V1     
//Slider for set temp limit
bool switchStatus1 = 0; // 0 = manual,1=auto
int TempLevelLimit1 = 0;
bool manualSwitch1 = 0;
     

//ปุ่ม เปิด-ปิดปั้ม 12VDC Pump
//V13 Auto&Manual 12VDC Pump
//V14 Slider 12VDC Pump
#define Relay3_dcpump   12
#define Widget_Btn_btn2 V2          
//Slider for set Humi limit
bool switchStatus2 = 0; // 0 = manual,1=auto
int HumiLevellimit = 0;
bool manualSwitch2 = 0;

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
char auth[] = "";
char ssid[] = "";                  //Wi-Fi                
char pass[] = "";                             //Password เชื่อมต่อ Wi-Fi                         
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
  pinMode(Relay1_ledblynk,OUTPUT);            // NODEMCU PIN gpio16 
  pinMode(Relay2_cfan,OUTPUT);                // NODEMCU PIN gpio14
  pinMode(Relay3_dcpump,OUTPUT);              // NODEMCU PIN gpio12   
  //pinMode(Relay4_btn4,OUTPUT);              // NODEMCU PIN GPIO13         
  
  // Set Defult Relay Status
  digitalWrite(Relay1_ledblynk,LOW);          // NODEMCU PIN gpio16
  digitalWrite(Relay2_cfan,LOW);              // NODEMCU PIN gpio14
  digitalWrite(Relay3_dcpump,LOW);            // NODEMCU PIN gpio12
  //digitalWrite(Relay4_btn4,LOW);            // NODEMCU PIN gpio13 
  
  Blynk.begin(auth, ssid, pass); 
  timer.setInterval(1000L, dl13_temp);
  timer.setInterval(1100L, dl13_humi);
}
//=====Setup Function=====//

// Update switchStatus1 on Temperature
BLYNK_WRITE(V11)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}

// Update Temperature setting
BLYNK_WRITE(V12)
{   
  TempLevelLimit1 = param.asInt(); // Get value as integer
}

// Update manualSwitch
BLYNK_WRITE(V1)
{
  manualSwitch1 = param.asInt();
}

//==DL13 Modbus Temperature==//
void dl13_temp()
{
  uint8_t result;
  float temp = (node.getResponseBuffer(0)/10.0f);
  //float humi = (node.getResponseBuffer(1)/10.0f);

 
  Serial.println("get data");
  result = node.readInputRegisters(0x040A, 2); // Read 2 registers starting at 1)
  if (result == node.ku8MBSuccess)
  {
    Serial.print("Temp: ");
    Serial.println(node.getResponseBuffer(0)/10.0f);
    //Serial.print("Humi: ");
    //Serial.println(node.getResponseBuffer(1)/10.0f);
  }
    Blynk.virtualWrite(V5, temp);
    //Blynk.virtualWrite(V6, humi);
    delay(1000);

  if(switchStatus1)
  {
    // auto
    if(temp > TempLevelLimit1)
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
//==DL13 Modbus Temperature==//


// Update switchStatus2 on Humidity
BLYNK_WRITE(V13)
{   
  switchStatus2 = param.asInt(); // Get value as integer
}

// Update Humidity setting
BLYNK_WRITE(V14)
{   
  HumiLevellimit = param.asInt(); // Get value as integer
}

// Update manualSwitch2
BLYNK_WRITE(V2)
{
  manualSwitch2 = param.asInt();
}

//==DL13 Modbus Humidity==//
void dl13_humi()
{
  uint8_t result;
  //float temp = (node.getResponseBuffer(0)/10.0f);
  float humi = (node.getResponseBuffer(1)/10.0f);

 
  Serial.println("get data");
  result = node.readInputRegisters(0x040A, 2); // Read 2 registers starting at 1)
  if (result == node.ku8MBSuccess)
  {
    //Serial.print("Temp: ");
    //Serial.println(node.getResponseBuffer(0)/10.0f);
    Serial.print("Humi: ");
    Serial.println(node.getResponseBuffer(1)/10.0f);
  }
    //Blynk.virtualWrite(V5, temp);
    Blynk.virtualWrite(V6, humi);
    delay(1000);

  if(switchStatus2)
  {
    // auto
    if(humi < HumiLevellimit)
    {
        digitalWrite(Relay3_dcpump, HIGH);  
        Blynk.virtualWrite(V2, 1);
    }  
    else
    {
        digitalWrite(Relay3_dcpump, LOW);
        Blynk.virtualWrite(V2, 0);
    }
  }
  else
  {
    if(manualSwitch2)
    {
        digitalWrite(Relay3_dcpump, HIGH); 
    }
    else
    {
        digitalWrite(Relay3_dcpump, LOW);
    }
    // manaul
  }
}
//==DL13 Modbus Humidity==//


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
