#include <WiFi.h>                       //https://github.com/esp8266/Arduino
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>          //  Blynk_Release_v0.6.1 
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
int blynkIsDownCount = 0;
BlynkTimer timer;
char ssid[] = "";                         //ชื่อ SSID ที่เราต้องการเชื่อมต่อ        
char pass[] = "";                               //รหัสผ่าน WI-FI
char auth[] = "";                         //Auth token from blynk app 

#include <Wire.h> 
#include "ETT_ModbusRTU.h"
#include <HardwareSerial.h>
#define SerialDebug  Serial                                                                       // USB Serial(Serial0)

#define SerialRS485_RX_PIN    16//26
#define SerialRS485_TX_PIN    17//27
#define RS485_DIRECTION_PIN   5//25  
#define SerialRS485  Serial1                                                                      // Serial1(IO17=TXD,IO16=RXD)

uint16_t InputRegister[8];   
uint16_t HoldingRegister[8]; 
uint8_t u8state;                                                                                  // machine state
uint8_t u8query;                                                                                  // pointer to message query
uint16_t moisture_int16_value;
float moisture_float_value;
float moisture_percentage1;

Modbus master(0,                                                                                  // node id = 0(master)
              SerialRS485,                                                                        // Serial2
              RS485_DIRECTION_PIN);                                                               // RS485 Modbus

modbus_t telegram[2];                                                                             // 2-Modbus Frame Service
unsigned long u32wait;
unsigned long lastScanBusTime = 0;

void setup() 
{
   
   SerialDebug.begin(115200);
  while(!SerialDebug);

  SerialRS485.begin(9600, SERIAL_8N1, SerialRS485_RX_PIN, SerialRS485_TX_PIN);
  while(!SerialRS485);

  SerialDebug.println();
  SerialDebug.println("Interface...Soil Moisture-H Modbus RTU");
  
  //===============================================================================================
  // telegram 0: Read Input Register
  //===============================================================================================
  telegram[0].u8id = 5;                                                                           // Slave Address
  //===============================================================================================
  telegram[0].u8fct = 4;                                                                          // Function 0x04(Read Input Register)  
  telegram[0].u16RegAdd = 0;                                                                      // Start Address Read(0x0000)
  telegram[0].u16CoilsNo = 8;                                                                     // Number of Register to Read(8 Input Register)
  telegram[0].au16reg = InputRegister;                                                            // Pointer to Buffer Save Input Register
  //===============================================================================================
  
  //===============================================================================================
  // telegram 1: Read Holding Register
  //===============================================================================================
  telegram[1].u8id = 5;                                                                           // Slave Address
  //===============================================================================================
  telegram[1].u8fct = 3;                                                                          // Function 0x03(Read Holding Register)
  telegram[1].u16RegAdd = 512;                                                                    // Start Address Read(0x0200:512)  
  telegram[1].u16CoilsNo = 1;                                                                     // Number of Register to Read(1 Holding Register)
  telegram[1].au16reg = HoldingRegister;                                                          // Pointer to Buffer Save Holding Register
  //===============================================================================================
  
  //===============================================================================================
  master.begin(SerialRS485);                                                                      // Mosbus Interface
  master.setTimeOut(2000);                                                                        // if there is no answer in 2000 ms, roll over
  u32wait = millis() + 2000;
  u8state = u8query = 0; 
  //===============================================================================================
  lastScanBusTime = millis();
  //===============================================================================================

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
//------------------------------------------------------------------------------------------------------------------------//
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L,soil1);    //Read Modbus SHT20 Sensor1
  timer.setInterval(10000L, reconnectblynk);  //Function reconnect blynk 
}

void soil1() {
  switch( u8state ) 
  {
    case 0: 
      //===========================================================================================
      if (millis() > u32wait) u8state++;                                                          // wait state
      //===========================================================================================
    break;
    
    case 1: 
      //===========================================================================================
      master.query(telegram[u8query]);                                                            // send query (only once)
      u8state++;
      u8query++;
      //===========================================================================================
      if(u8query > 1) u8query = 0;                                                                // telegram[0],telegram[1], ----> ,telegram[0]
      //===========================================================================================
    break;
    
    case 2:
      //===========================================================================================
      if(master.poll())                                                                           // check incoming messages
      {
        //=========================================================================================
        // Start of Soil Moisture-H Modbus RTU Service Data
        //=========================================================================================
        Serial.println("Demo ET-MEGA32U4 RS485 & SOIL MOISTURE-H MODBUS RTU");
        //=========================================================================================
        
        //=========================================================================================
        // Modbus Holding Register
        //=========================================================================================
        Serial.print("Soil Moisture Modbus Slave ID = ");
        Serial.println(HoldingRegister[0]);
        //=========================================================================================
        
        //=========================================================================================
        // Modbus Input Register
        // 0 = Soil Temperature/100
        // 1 = Soil Moisture/100
        //=========================================================================================
        moisture_int16_value = InputRegister[1];                                                  // Soil Moisture/100
        //=========================================================================================
        Serial.print("Soil Moisture Value = ");
        Serial.print(moisture_int16_value);                                                       // Soil Moisture/100
        Serial.print(" : ");
        //Serial.print((float)moisture_int16_value/100.0);                                          // Soil Moisture
        moisture_percentage1 = ((float)moisture_int16_value/100.0);
        Serial.print(moisture_percentage1);
        Blynk.virtualWrite(V0,moisture_percentage1);
        Serial.print("%RH");
        //=========================================================================================
        Serial.println();
        //=========================================================================================
        // End of Soil Moisture-H Modbus RTU Service Data
        //=========================================================================================
      }  
      //===========================================================================================

      //===========================================================================================
      if(master.getState() == COM_IDLE) 
      {
        u8state = 0;
        u32wait = millis() + 1000; 
      }
      //===========================================================================================
      
      /*
      else 
      */ 
    break;
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
  }
}

void loop() 
{
   if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();
}
