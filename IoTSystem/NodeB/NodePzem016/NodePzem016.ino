//อ้างอิง https://solarduino.com/pzem-016-ac-energy-meter-online-monitoring-with-blynk-app/ 
////////// Pin App blynk////////////////
// V0 = สัญญาณ Wi-Fi
// V1 = Ledblynk
// V2 = Voltage
// V3 = Cerrunt
// V4 = Power
// V5 = Energy
// V6 = Power Factor
// V7 = Hz
// V8 = Reset Energy
//////////////////////////

//////MCU Digital Pin/////
// D0 = MAX485_DE
// D1 = MAX485_RE
// D2 = MAX485_RO
// D3 = MAX485_DI
// D4 = Ledblynk
//////////////////////////
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <SimpleTimer.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
BlynkTimer timer;
int blynkIsDownCount=0;

//Max485
#include <ModbusMaster.h>
ModbusMaster node;
#define MAX485_DE 16          //NodeMCU D0
#define MAX485_RE 5           //NodeMCU D1
#define Ledblynk  2           //NodeMCU D4
//Serail communication
#include <SoftwareSerial.h>
SoftwareSerial PZEMSerial;

//Pzem016
static uint8_t pzemSlaveAddr = 0x01;                  // Declare the address of device (meter) in term of 8 bits. You can change to 0x02 etc if you have more than 1 energy meter.
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

char blynk_token[34] = "";                            //ไม่ต้องกำหนด เราจะกำหนดผ่าน web config

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

//Setup Function
void setup()
{
// put your setup code here, to run once:      
startMillis1 = millis();        
Serial.begin(9600);                                   /* to display readings in Serial Monitor at 9600 baud rates */
PZEMSerial.begin(9600,SWSERIAL_8N1,4,0);              // NodeMCU D2,D3 4 = Rx/R0/ GPIO 4 (D2) & 0 = Tx/DI/ GPIO 0 (D3) on NodeMCU 
/* 1- PZEM-014/016 AC Energy Meter */       
startMillisPZEM = millis();                           /* Start counting time for run code */            

//Set pin mode
pinMode(Ledblynk, OUTPUT);
pinMode(MAX485_RE, OUTPUT);                           /* Define RE Pin as Signal Output for RS485 converter. Output pin means Arduino command the pin signal to go high or low so that signal is received by the converter*/
pinMode(MAX485_DE, OUTPUT);                           /* Define DE Pin as Signal Output for RS485 converter. Output pin means Arduino command the pin signal to go high or low so that signal is received by the converter*/
//Set Default status
digitalWrite(Ledblynk, 0);
digitalWrite(MAX485_RE, 0);                           /* Arduino create output signal for pin RE as LOW (no output)*/
digitalWrite(MAX485_DE, 0);                           /* Arduino create output signal for pin DE as LOW (no output)*/
                                                              /* both pins no output means the converter is in communication signal receiving mode */
node.preTransmission(preTransmission);                /* Callbacks allow us to configure the RS485 transceiver correctly*/
node.postTransmission(postTransmission);
node.begin(pzemSlaveAddr,PZEMSerial);                 /* Define and start the Modbus RTU communication. Communication to specific slave address and which Serial port */
delay(1000);                                          /* after everything done, wait for 1 second */
/* 2 - Data submission to Blynk Server */
startMillisReadData = millis();                       /* Start counting time for data submission to Blynk Server*/
  
  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          //strcpy(mqtt_server, json["mqtt_server"]);
          //strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(blynk_token, json["blynk_token"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
      WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
  
  //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.addParameter(&custom_blynk_token);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
//if (WiFi.SSID()!="") wifiManager.setConfigPortalTimeout(60);
 
  if (!wifiManager.autoConnect("Node_Pzem016", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(100);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
  } 
  if ((WiFi.status()!=WL_CONNECTED) )
  {
      Serial.println("failed to connect");
  } 
  else
  {
    //if you get here you have connected to the WiFi
    Serial.println("connected........:)");
    Serial.print("local ip: ");
    Serial.println(WiFi.localIP());
  }
  
  //read updated parameters
  //strcpy(mqtt_server, custom_mqtt_server.getValue());
  //strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(blynk_token, custom_blynk_token.getValue());
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    //json["mqtt_server"] = mqtt_server;
    //json["mqtt_port"] = mqtt_port;
    json["blynk_token"] = blynk_token;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  
  //Connect to Blynk Server
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);  
  Blynk.config(blynk_token); 
  timer.setInterval(30000L, reconnecting);  //Function reconnect
}

BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    Blynk.syncAll();

 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(Ledblynk,HIGH);
 }
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
    digitalWrite(Ledblynk, LOW); //ledpin for check blynk connected 
  }
   if (blynkIsDownCount >= 10){
    ESP.restart();
  }
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

//Loop 
void loop()
{ 
  Blynk.run();
  timer.run();
  PZEM016AC();
}
