#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <SoftwareSerial.h>                       //include virtual Serial Port coding 
SoftwareSerial PZEMSerial;  
#include <ModbusMaster.h>                             
#define MAX485_DE  16                             //D0//DE                       
#define MAX485_RE  5                              //D1  /RE
static uint8_t pzemSlaveAddr = 0x01;                  
static uint16_t NewshuntAddr = 0x0000;                
        
ModbusMaster node;                                    
        
float PZEMVoltage =0;                                 
float PZEMCurrent =0;                                 
float PZEMPower =0;                                   
float PZEMEnergy=0;                                   
 
unsigned long startMillisPZEM;                        
unsigned long currentMillisPZEM;                      
const unsigned long periodPZEM = 1000;                 
  
unsigned long startMillisReadData;                    
unsigned long currentMillisReadData;                  
const unsigned long periodReadData = 1000;            
int ResetEnergy = 0;                                  
int a = 1;
 unsigned long startMillis1;     


// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "";
const char* password = "";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "";

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient nodeSolarMonout;
PubSubClient client(nodeSolarMonout);

// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client2")) {
      Serial.println("connected2");  
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() { 
startMillis1 = millis();
        
Serial.begin(9600);                                     
PZEMSerial.begin(9600,SWSERIAL_8N1,4,0);    //D2=RO,D3=DI
startMillisPZEM = millis();                           
pinMode(MAX485_RE, OUTPUT);                           
pinMode(MAX485_DE, OUTPUT);                           
digitalWrite(MAX485_RE, 0);                           
digitalWrite(MAX485_DE, 0);                           
        
node.preTransmission(preTransmission);                
node.postTransmission(postTransmission);
node.begin(pzemSlaveAddr,PZEMSerial);                          
delay(1000);      
                                    
startMillisReadData = millis();
setup_wifi();
client.setServer(mqtt_server, 1883);
}

// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {
if (!client.connected()) {
    //reconnect();
}
if(!client.loop())
client.connect("ESP8266Client2");

if ((millis()- startMillis1 >= 10000) && (a ==1)) {
          setShunt(pzemSlaveAddr);                                          
          changeAddress(0XF8, pzemSlaveAddr);                             
          a = 0;
        }                                                    
        
currentMillisPZEM = millis(); 
        
if (currentMillisPZEM - startMillisPZEM >= periodPZEM) {    
uint8_t result;                                                                                 
result = node.readInputRegisters(0x0000, 6);                                                    
if (result == node.ku8MBSuccess){
uint32_t tempdouble = 0x00000000;                                                           
PZEMVoltage = node.getResponseBuffer(0x0000) / 100.0;                                       
                                                                                                          
PZEMCurrent = node.getResponseBuffer(0x0001) / 100.0;                                       
              
tempdouble =  (node.getResponseBuffer(0x0003) << 16) + node.getResponseBuffer(0x0002);      
PZEMPower = tempdouble / 10.0;                                                              
              
tempdouble =  (node.getResponseBuffer(0x0005) << 16) + node.getResponseBuffer(0x0004);      
PZEMEnergy = tempdouble;                                                                    
              
if (pzemSlaveAddr==2){
  //
                }
            } 
              else
                {
                }
              startMillisPZEM = currentMillisPZEM ;                                                       
}
                                                                                                        
                
currentMillisReadData = millis();                                                                 
if (currentMillisReadData - startMillisReadData >= periodReadData)                                 
          {
Serial.print("Vdc : "); Serial.print(PZEMVoltage); Serial.println(" V ");
Serial.print("Idc : "); Serial.print(PZEMCurrent); Serial.println(" A ");
Serial.print("Power : "); Serial.print(PZEMPower); Serial.println(" W ");
Serial.print("Energy : "); Serial.print(PZEMEnergy); Serial.println(" Wh ");

startMillisReadData = millis();

static char voltageout[7]; 
dtostrf (PZEMVoltage,6,2,voltageout);
client.publish("solar/voltageout", voltageout);

static char currentout[7]; 
dtostrf (PZEMCurrent,6,2,currentout);
client.publish("solar/currentout", currentout);

static char powerout[7]; 
dtostrf (PZEMPower,6,2,powerout);
client.publish("solar/powerout", powerout);

static char energyout[7]; 
dtostrf (PZEMEnergy,6,2,energyout);
client.publish("solar/energyout", energyout);             
          }        
}
  
void preTransmission()                                                                                    
{
/* 1- PZEM-017 DC Energy Meter */
if(millis() - startMillis1 > 5000)                                                                
{
digitalWrite(MAX485_RE, 1);                                                                     
digitalWrite(MAX485_DE, 1);                                                                     
delay(1);                                                                                       
}
}

void postTransmission()                                                                                   
{
        
/* 1- PZEM-017 DC Energy Meter */
if(millis() - startMillis1 > 5000)                                                                
{
delay(3);                                                                                       
digitalWrite(MAX485_RE, 0);                                                                     
digitalWrite(MAX485_DE, 0);                                                                     
}
}

void setShunt(uint8_t slaveAddr)                                                                          
{

/* 1- PZEM-017 DC Energy Meter */
        
static uint8_t SlaveParameter = 0x06;                                                             
static uint16_t registerAddress = 0x0003;                                                         
        
uint16_t u16CRC = 0xFFFF;                                                                         /* declare CRC check 16 bits*/
u16CRC = crc16_update(u16CRC, slaveAddr);                                                         // Calculate the crc16 over the 6bytes to be send
u16CRC = crc16_update(u16CRC, SlaveParameter);
u16CRC = crc16_update(u16CRC, highByte(registerAddress));
u16CRC = crc16_update(u16CRC, lowByte(registerAddress));
u16CRC = crc16_update(u16CRC, highByte(NewshuntAddr));
u16CRC = crc16_update(u16CRC, lowByte(NewshuntAddr));
      
preTransmission();                                                                                /* trigger transmission mode*/
      
PZEMSerial.write(slaveAddr);                                                                      /* these whole process code sequence refer to manual*/
PZEMSerial.write(SlaveParameter);
PZEMSerial.write(highByte(registerAddress));
PZEMSerial.write(lowByte(registerAddress));
PZEMSerial.write(highByte(NewshuntAddr));
PZEMSerial.write(lowByte(NewshuntAddr));
PZEMSerial.write(lowByte(u16CRC));
PZEMSerial.write(highByte(u16CRC));
delay(10);
postTransmission();                                                                               /* trigger reception mode*/
delay(100);
}


void changeAddress(uint8_t OldslaveAddr, uint8_t NewslaveAddr)                                            //Change the slave address of a node
{
/* 1- PZEM-017 DC Energy Meter */
    
static uint8_t SlaveParameter = 0x06;                                                             /* Write command code to PZEM */
static uint16_t registerAddress = 0x0002;                                                         /* Modbus RTU device address command code */
uint16_t u16CRC = 0xFFFF;                                                                         /* declare CRC check 16 bits*/
u16CRC = crc16_update(u16CRC, OldslaveAddr);                                                      // Calculate the crc16 over the 6bytes to be send
u16CRC = crc16_update(u16CRC, SlaveParameter);
u16CRC = crc16_update(u16CRC, highByte(registerAddress));
u16CRC = crc16_update(u16CRC, lowByte(registerAddress));
u16CRC = crc16_update(u16CRC, highByte(NewslaveAddr));
u16CRC = crc16_update(u16CRC, lowByte(NewslaveAddr));
preTransmission();                                                                                 /* trigger transmission mode*/
PZEMSerial.write(OldslaveAddr);                                                                       /* these whole process code sequence refer to manual*/
PZEMSerial.write(SlaveParameter);
PZEMSerial.write(highByte(registerAddress));
PZEMSerial.write(lowByte(registerAddress));
PZEMSerial.write(highByte(NewslaveAddr));
PZEMSerial.write(lowByte(NewslaveAddr));
PZEMSerial.write(lowByte(u16CRC));
PZEMSerial.write(highByte(u16CRC));
delay(10);
postTransmission();                                                                                /* trigger reception mode*/
delay(100);
}
