//อ้างอิง https://solarduino.com/pzem-016-ac-energy-meter-online-monitoring-with-blynk-app/ 
////////// Pin App blynk////////////////
// V0 = สัญญาณไวไฟ
// V1 = Ledblynk
// V2 = Voltage
// V3 = Cerrunt
// V4 = Power
// V5 = Energy
// V6 = Power Factor
// V7 = Hz
// V8 = Reset Energy
// V16 = Current Time
// V17 = Current Date  
//////////////////////////

//////MCU Digital Pin/////
// D0 = MAX485_DE
// D1 = MAX485_RE
// D2 = MAX485_RO
// D3 = MAX485_DI
//////////////////////////

/* 0- Blynk Server and Wifi Connection */
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
BlynkTimer timer;
int blynkIsDownCount=0;

char auth[] = "";                                     // Put in the Auth Token for the project from Blynk. You should receive it in your email.
char ssid[] = "";                                     // Key in your wifi name. You can check with your smart phone for your wifi name
char pass[] = "";                                     // Key in your wifi password.

//MQTT
#include <PubSubClient.h>
const char* mqtt_server = "";                         //MQTT Server 
WiFiClient nodepzem016;                               //Change client name if you have multiple ESPs running in your home automation system
PubSubClient client(nodepzem016);

//RTC Widget
#include <TimeLib.h>
#include <WidgetRTC.h>
WidgetRTC rtc;

/* Virtual Serial Port */
#include <SoftwareSerial.h>                           /* include virtual Serial Port coding */
SoftwareSerial PZEMSerial;                            // Move the PZEM DC Energy Meter communication pins from Rx to pin D1 = GPIO 5 & TX to pin D2 = GPIO 4   

/* 1- PZEM-014/016 AC Energy Meter */        
#include <ModbusMaster.h>                             // Load the (modified) library for modbus communication command codes. Kindly install at our website.
#define MAX485_DE      16 //D0                        // Define DE Pin to Arduino pin. Connect DE Pin of Max485 converter module to Pin D0 (GPIO 16) Node MCU board
#define MAX485_RE      5  //D1                        // Define RE Pin to Arduino pin. Connect RE Pin of Max485 converter module to Pin D1 (GPIO 5) Node MCU board
                                                      // These DE anr RE pins can be any other Digital Pins to be activated during transmission and reception process.
static uint8_t pzemSlaveAddr = 0x01;                  // Declare the address of device (meter) in term of 8 bits. You can change to 0x02 etc if you have more than 1 energy meter.
ModbusMaster node;                                    /* activate modbus master codes*/  
float PZEMVoltage =0;                                 /* Declare value for AC voltage */
float PZEMCurrent =0;                                 /* Declare value for AC current*/
float PZEMPower =0;                                   /* Declare value for AC Power */
float PZEMEnergy=0;                                   /* Declare value for energy */
float PZEMHz =0;                                      /* Declare value for frequency */
float PZEMPf=0;                                       /* Declare value for power factor */
unsigned long startMillisPZEM;                        /* start counting time for data collection */
unsigned long currentMillisPZEM;                      /* current counting time for data collection */
const unsigned long periodPZEM = 1000;                // refresh data collection every X seconds (in seconds). Default 1000 = 1 second 

/* 2 - Data submission to Blynk Server  */
unsigned long startMillisReadData;                    /* start counting time for data collection*/
unsigned long currentMillisReadData;                  /* current counting time for data collection*/
const unsigned long periodReadData = 1000;            /* refresh every X seconds (in seconds) in LED Display. Default 1000 = 1 second */
int ResetEnergy = 0;                                  /* reset energy function */
int a = 1;
unsigned long startMillis1;                           // to count time during initial start up (PZEM Software got some error so need to have initial pending time)

//Setup Wi-Fi connection
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

//Reconnect to MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("nodepzem016")) {
      Serial.println("connected1");  
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

long rssi;

//Wi-Fi Signal
void sentWiFi()
{
rssi = WiFi.RSSI(); /// อ่านสัญญาณไวไฟ
if (rssi < -99)
{
rssi = -99;
} else {
rssi = rssi + 100;
}
Blynk.virtualWrite(V0, rssi);                           //ส่งค่าเข้า Pin แอป Blynk
}

//Setup Function
void setup() 
{
/* General*/       
startMillis1 = millis();
        
Serial.begin(9600);                                   /* to display readings in Serial Monitor at 9600 baud rates */
PZEMSerial.begin(9600,SWSERIAL_8N2,4,0);              // 4 = Rx/R0/ GPIO 4 (D2) & 0 = Tx/DI/ GPIO 0 (D3) on NodeMCU 

/* 1- PZEM-014/016 AC Energy Meter */
        
startMillisPZEM = millis();                           /* Start counting time for run code */            
//Set pin mode
pinMode(MAX485_RE, OUTPUT);                           /* Define RE Pin as Signal Output for RS485 converter. Output pin means Arduino command the pin signal to go high or low so that signal is received by the converter*/
pinMode(MAX485_DE, OUTPUT);                           /* Define DE Pin as Signal Output for RS485 converter. Output pin means Arduino command the pin signal to go high or low so that signal is received by the converter*/
//Set Default status
digitalWrite(MAX485_RE, 0);                           /* Arduino create output signal for pin RE as LOW (no output)*/
digitalWrite(MAX485_DE, 0);                           /* Arduino create output signal for pin DE as LOW (no output)*/
                                                              /* both pins no output means the converter is in communication signal receiving mode */
node.preTransmission(preTransmission);                /* Callbacks allow us to configure the RS485 transceiver correctly*/
node.postTransmission(postTransmission);
node.begin(pzemSlaveAddr,PZEMSerial);                 /* Define and start the Modbus RTU communication. Communication to specific slave address and which Serial port */
delay(1000);                                          /* after everything done, wait for 1 second */

/* 2 - Data submission to Blynk Server */
startMillisReadData = millis();                       /* Start counting time for data submission to Blynk Server*/

setup_wifi();
client.setServer(mqtt_server, 1883);
Blynk.begin(auth, ssid, pass);                        /* You can also specify server: Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80); Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);*/
timer.setInterval(1500L, sentWiFi);
timer.setInterval(10000L, clockDisplay);
timer.setInterval(30000L, reconnecting);  
}

BLYNK_CONNECTED() 
{
Blynk.syncAll();
}

//Reconnect to blynk
void reconnecting()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    Blynk.syncAll();
    //digitalWrite(ledblynk, LOW); //ledpin for check blynk connected 
    // Synchronize time on connection
    rtc.begin();
  }
}

//Display Current Date/Time
void clockDisplay()
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  Blynk.virtualWrite(V16, currentTime);
  Blynk.virtualWrite(V17, currentDate);
}

//Loop 
void loop()
{
if (!client.connected()) {
    //reconnect();
  }
  if(!client.loop())
  client.connect("nodepzem016");
    
  Blynk.run();
  timer.run();
  PZEM016AC();
}

//Read Pzem data
void PZEM016AC()
{
if ((millis()- startMillis1 >= 10000) && (a ==1))
  {   
    changeAddress(0XF8, pzemSlaveAddr);                         // By delete the double slash symbol, the meter address will be set as 0x01. 
                                                                // By default I allow this code to run every program startup. Will not have effect if you only have 1 meter
    a = 0;
  }
        
/* 1- PZEM-014/016 AC Energy Meter */

currentMillisPZEM = millis();                                                                     /* count time for program run every second (by default)*/
  if (currentMillisPZEM - startMillisPZEM >= periodPZEM)                                            /* for every x seconds, run the codes below*/
  {    
    uint8_t result;                                                                                 /* Declare variable "result" as 8 bits */   
    result = node.readInputRegisters(0x0000, 9);                                                    /* read the 9 registers (information) of the PZEM-014 / 016 starting 0x0000 (voltage information) kindly refer to manual)*/
    if (result == node.ku8MBSuccess)                                                                /* If there is a response */
      {
        uint32_t tempdouble = 0x00000000;                                                           /* Declare variable "tempdouble" as 32 bits with initial value is 0 */ 
        PZEMVoltage = node.getResponseBuffer(0x0000) / 10.0;                                        /* get the 16bit value for the voltage value, divide it by 10 (as per manual) */
                                                                                                          // 0x0000 to 0x0008 are the register address of the measurement value
        tempdouble =  (node.getResponseBuffer(0x0002) << 16) + node.getResponseBuffer(0x0001);      /* get the currnet value. Current value is consists of 2 parts (2 digits of 16 bits in front and 2 digits of 16 bits at the back) and combine them to an unsigned 32bit */
        PZEMCurrent = tempdouble / 1000.00;                                                         /* Divide the value by 1000 to get actual current value (as per manual) */
              
        tempdouble =  (node.getResponseBuffer(0x0004) << 16) + node.getResponseBuffer(0x0003);      /* get the power value. Power value is consists of 2 parts (2 digits of 16 bits in front and 2 digits of 16 bits at the back) and combine them to an unsigned 32bit */
        PZEMPower = tempdouble / 10.0;                                                              /* Divide the value by 10 to get actual power value (as per manual) */
              
        tempdouble =  (node.getResponseBuffer(0x0006) << 16) + node.getResponseBuffer(0x0005);      /* get the energy value. Energy value is consists of 2 parts (2 digits of 16 bits in front and 2 digits of 16 bits at the back) and combine them to an unsigned 32bit */
        PZEMEnergy = tempdouble;                                                                    
  
        PZEMHz = node.getResponseBuffer(0x0007) / 10.0;                                             /* get the 16bit value for the frequency value, divide it by 10 (as per manual) */
        PZEMPf = node.getResponseBuffer(0x0008) / 100.00;                                           /* get the 16bit value for the power factor value, divide it by 100 (as per manual) */
              
        if (pzemSlaveAddr==2)                                                                       /* just for checking purpose to see whether can read modbus*/
                {
                }
            } 
              else
                {
                }
              startMillisPZEM = currentMillisPZEM ;                                         /* Set the starting point again for next counting time */
        }

/* 2 - Data submission to Blynk Server  */   
  currentMillisReadData = millis();                                                         /* Set counting time for data submission to server*/
    if (currentMillisReadData - startMillisReadData >= periodReadData)                      /* for every x seconds, run the codes below*/  
          {
            Serial.print("Vac : "); Serial.print(PZEMVoltage); Serial.println(" V ");
            Serial.print("Iac : "); Serial.print(PZEMCurrent); Serial.println(" A ");
            Serial.print("Power : "); Serial.print(PZEMPower); Serial.println(" W ");
            Serial.print("Energy : "); Serial.print(PZEMEnergy); Serial.println(" Wh ");
            Serial.print("Power Factor : "); Serial.print(PZEMPf); Serial.println(" pF ");
            Serial.print("Frequency : "); Serial.print(PZEMHz); Serial.println(" Hz ");
            Blynk.virtualWrite(V2,PZEMVoltage);                                             
            Blynk.virtualWrite(V3,PZEMCurrent);
            Blynk.virtualWrite(V4,PZEMPower);
            Blynk.virtualWrite(V5,PZEMEnergy);
            Blynk.virtualWrite(V6,PZEMPf);
            Blynk.virtualWrite(V7,PZEMHz);  
            startMillisReadData = millis();                                                 /* Set the starting point again for next counting time */
         
          //Publish data to mqtt
          /*  static char voltagein[7]; 
            dtostrf (PZEMVoltage,6,2,voltagein);
            client.publish("solar2/voltagein", voltagein);
            */
          } 
}

void preTransmission()                                  /* transmission program when triggered*/
{ 
        /* 1- PZEM-014/016 AC Energy Meter */
        if(millis() - startMillis1 > 5000)              // Wait for 5 seconds as ESP Serial cause start up code crash
        {
          digitalWrite(MAX485_RE, 1);                   /* put RE Pin to high*/
          digitalWrite(MAX485_DE, 1);                   /* put DE Pin to high*/
          delay(1);                                     // When both RE and DE Pin are high, converter is allow to transmit communication
        }
}

void postTransmission()                                 /* Reception program when triggered*/
{        
        /* 1- PZEM-014/016 AC Energy Meter */
        if(millis() - startMillis1 > 5000)              // Wait for 5 seconds as ESP Serial cause start up code crash
        {
          delay(3);                                     // When both RE and DE Pin are low, converter is allow to receive communication
          digitalWrite(MAX485_RE, 0);                   /* put RE Pin to low*/
          digitalWrite(MAX485_DE, 0);                   /* put DE Pin to low*/
        }
}


BLYNK_WRITE(V8)                                               
  {
    if(param.asInt()==1)
    { 
      uint16_t u16CRC = 0xFFFF;                         /* declare CRC check 16 bits*/
      static uint8_t resetCommand = 0x42;               /* reset command code*/
      uint8_t slaveAddr =pzemSlaveAddr;
      u16CRC = crc16_update(u16CRC, slaveAddr);
      u16CRC = crc16_update(u16CRC, resetCommand);
      preTransmission();                                /* trigger transmission mode*/                
      PZEMSerial.write(slaveAddr);                      /* send device address in 8 bit*/
      PZEMSerial.write(resetCommand);                   /* send reset command */
      PZEMSerial.write(lowByte(u16CRC));                /* send CRC check code low byte  (1st part) */
      PZEMSerial.write(highByte(u16CRC));               /* send CRC check code high byte (2nd part) */ 
      delay(10);
      postTransmission();                               /* trigger reception mode*/
      delay(100);
    }
}


void changeAddress(uint8_t OldslaveAddr, uint8_t NewslaveAddr)    //Change the slave address of a node
{
/* 1- PZEM-014/016 AC Energy Meter */
        
  static uint8_t SlaveParameter = 0x06;                           /* Write command code to PZEM */
  static uint16_t registerAddress = 0x0002;                       /* Modbus RTU device address command code */
  uint16_t u16CRC = 0xFFFF;                                       /* declare CRC check 16 bits*/
  u16CRC = crc16_update(u16CRC, OldslaveAddr);                    // Calculate the crc16 over the 6bytes to be send
  u16CRC = crc16_update(u16CRC, SlaveParameter);
  u16CRC = crc16_update(u16CRC, highByte(registerAddress));
  u16CRC = crc16_update(u16CRC, lowByte(registerAddress));
  u16CRC = crc16_update(u16CRC, highByte(NewslaveAddr));
  u16CRC = crc16_update(u16CRC, lowByte(NewslaveAddr));

  preTransmission();                                              /* trigger transmission mode*/
      
  PZEMSerial.write(OldslaveAddr);                                 /* these whole process code sequence refer to manual*/
  PZEMSerial.write(SlaveParameter);
  PZEMSerial.write(highByte(registerAddress));
  PZEMSerial.write(lowByte(registerAddress));
  PZEMSerial.write(highByte(NewslaveAddr));
  PZEMSerial.write(lowByte(NewslaveAddr));
  PZEMSerial.write(lowByte(u16CRC));
  PZEMSerial.write(highByte(u16CRC));
  delay(10);
  postTransmission();                                             /* trigger reception mode*/
  delay(100);
}
