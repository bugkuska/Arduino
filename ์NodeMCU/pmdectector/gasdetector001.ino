/*******************Demo for MQ-2 Gas Sensor Module V1.0*****************************/

#include <FS.h>                   //this needs to be first, or it all crashes and burns...
//#define BLYNK_DEBUG
#define BLYNK_PRINT Serial        // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <BlynkSimpleEsp8266.h>   //https://github.com/blynkkk/blynk-library
#include <DNSServer.h>            //https://github.com/esp8266/Arduino
#include <ESP8266WebServer.h>     //https://github.com/esp8266/Arduino
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <SimpleTimer.h>          //https://github.com/jfturcot/SimpleTimer
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

//Line Notify
#include <TridentTD_LineNotify.h>
//#define LINE_TOKEN  ""
#define LINE_TOKEN  ""

//DHT
#include <DHT.h>                  //https://github.com/adafruit/DHT-sensor-library
#define DHTPIN D7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

int blynkIsDownCount=0;

#include <SimpleTimer.h>
BlynkTimer timer;

//define your default values here, if there are different values in config.json, they are overwritten.
//char mqtt_server[40];
//char mqtt_port[6] = "8080";
char blynk_token[34] = "";

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

//Relay
const int Relay1 = D2;
const int Relay2 = D4;
const int Relay3 = D5;
const int Relay4 = D6;

//ldr
const int ldr_pin = D1;


//BlynkConnectted
const int ledblynk = D3;

//Led status
boolean stateled1=0;
boolean prevStateled1 = 0;
boolean stateled2=0;
boolean prevStateled2 = 0;
boolean stateled3=0;
boolean prevStateled3 = 0;
boolean stateled4=0;
boolean prevStateled4 = 0;
//bool swState = false;


/************************Hardware Related Macros************************************/
#define         MQ_PIN                       (A0)     //define which analog input channel you are going to use
#define         RL_VALUE                     (5)     //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR          (9.83)  //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
                                                     //which is derived from the chart in datasheet
/***********************Software Related Macros************************************/
#define         CALIBARAION_SAMPLE_TIMES     (50)    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (500)   //define the time interal(in milisecond) between each samples in the
                                                     //cablibration phase
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interal(in milisecond) between each samples in 
                                                     //normal operation
/**********************Application Related Macros**********************************/
#define         GAS_LPG                      (0)
#define         GAS_CO                       (1)
#define         GAS_SMOKE                    (2)
/*****************************Globals***********************************************/
float           LPGCurve[3]  =  {2.3,0.21,-0.47};   //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent"
                                                    //to the original curve. 
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.21), point2: (lg10000, -0.59) 
float           COCurve[3]  =  {2.3,0.72,-0.34};    //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent" 
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.72), point2: (lg10000,  0.15) 
float           SmokeCurve[3] ={2.3,0.53,-0.44};    //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent" 
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.53), point2: (lg10000,  -0.22)                                                     
float           Ro           =  10;                 //Ro is initialized to 10 kilo ohms

void setup()
{
  Serial.begin(9600);                               //UART setup, baudrate = 9600bps
  Serial.print("Calibrating...\n");                
  Ro = MQCalibration(MQ_PIN);                       //Calibrating the sensor. Please make sure the sensor is in clean air 
                                                    //when you perform the calibration                    
  Serial.print("Calibration is done...\n"); 
  Serial.print("Ro=");
  Serial.print(Ro);
  Serial.print("kohm");
  Serial.print("\n");

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
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read

  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 32);

  //WiFiManager
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  
  //add all your parameters here
  //wifiManager.addParameter(&custom_mqtt_server);
  //wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_blynk_token);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("pmDetecter_node02", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

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

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  
  //LineNotify
  LINE.setToken(LINE_TOKEN);  // กำหนด Line Token

  // Setup Pin Mode
  pinMode(ldr_pin,INPUT); //NODEMCU PIN D1
  pinMode(Relay1,OUTPUT); // NODEMCU PIN D2
  pinMode(Relay2,OUTPUT); // NODEMCU PIN D4
  pinMode(Relay3,OUTPUT); // NODEMCU PIN D5
  pinMode(Relay4,OUTPUT); // NODEMCU PIN D6
  pinMode(ledblynk,OUTPUT); // NODEMCU PIN D3

  // Set Defult Relay Status
  digitalWrite(Relay1, HIGH);
  digitalWrite(Relay2, HIGH);
  digitalWrite(Relay3, HIGH);
   digitalWrite(Relay4, HIGH);
  //digitalWrite(Relay4, HIGH);
  digitalWrite(ledblynk, HIGH);

  //Start read DHT11
  dht.begin();
  
  Blynk.config(blynk_token);
  timer.setInterval(1000L, checkledstate);
  timer.setInterval(30000L, reconnecting);  
  timer.setInterval(1000L, dht11Sensor); 
  timer.setInterval(1000L, ldrSensor); 
}

//Relay control
BLYNK_WRITE(V10) //Blynk Virtual Pin V10 to Button 1 Control Relay 1
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay1, LOW);
      Serial.println("Relay 1 On");   
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay1, HIGH);
      Serial.println("Relay 1 Off");    
    }
}
BLYNK_WRITE(V11) //Blynk Virtual Pin V11 to Button 2 Control Relay 2 
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay2, LOW);
      Serial.println("Relay 2 On");  
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay2, HIGH);
      Serial.println("Relay 2 Off");    
    }
}
BLYNK_WRITE(V12) //Blynk Virtual Pin V12 to Button 3 Control Relay 3
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay3, LOW);
      Serial.println("Relay 3 On");     
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay3, HIGH);
      Serial.println("Relay 3 Off");    
    }
}

BLYNK_WRITE(V13) //Blynk Virtual Pin V13 to Button 4 Control Relay 4
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay4, LOW);
      Serial.println("Relay 4 On");     
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay4, HIGH);
      Serial.println("Relay 4 Off");    
    }
}
void loop()
{


  if (Blynk.connected())
  {
    Blynk.run();
  }
   timer.run(); 
   
   Serial.print("LPG:"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG) );
   Serial.print( "ppm" );
   Serial.print("    ");   
   Serial.print("CO:"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CO) );
   Serial.print( "ppm" );
   Serial.print("    ");   
   Serial.print("SMOKE:"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE) );
   Serial.print( "ppm" );
   Serial.print("\n");
   delay(1000);

      Blynk.virtualWrite(V7, (MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG) ));      
      Blynk.virtualWrite(V8, (MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CO) )); 
      Blynk.virtualWrite(V9, (MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE) ));
     
}

/****************** MQResistanceCalculation ****************************************
Input:   raw_adc - raw value read from adc, which represents the voltage
Output:  the calculated sensor resistance
Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
         across the load resistor and its resistance, the resistance of the sensor
         could be derived.
************************************************************************************/ 
float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
}

/***************************** MQCalibration ****************************************
Input:   mq_pin - analog channel
Output:  Ro of the sensor
Remarks: This function assumes that the sensor is in clean air. It use  
         MQResistanceCalculation to calculates the sensor resistance in clean air 
         and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about 
         10, which differs slightly between different sensors.
************************************************************************************/ 
float MQCalibration(int mq_pin)
{
  int i;
  float val=0;

  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {            //take multiple samples
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBARAION_SAMPLE_TIMES;                   //calculate the average value

  val = val/RO_CLEAN_AIR_FACTOR;                        //divided by RO_CLEAN_AIR_FACTOR yields the Ro 
                                                        //according to the chart in the datasheet 

  return val; 
}
/*****************************  MQRead *********************************************
Input:   mq_pin - analog channel
Output:  Rs of the sensor
Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
         The Rs changes as the sensor is in the different consentration of the target
         gas. The sample times and the time interval between samples could be configured
         by changing the definition of the macros.
************************************************************************************/ 
float MQRead(int mq_pin)
{
  int i;
  float rs=0;

  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }

  rs = rs/READ_SAMPLE_TIMES;

  return rs;  
}

/*****************************  MQGetGasPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         gas_id      - target gas type
Output:  ppm of the target gas
Remarks: This function passes different curves to the MQGetPercentage function which 
         calculates the ppm (parts per million) of the target gas.
************************************************************************************/ 
int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
  if ( gas_id == GAS_LPG ) {
     return MQGetPercentage(rs_ro_ratio,LPGCurve);
  } else if ( gas_id == GAS_CO ) {
     return MQGetPercentage(rs_ro_ratio,COCurve);
  } else if ( gas_id == GAS_SMOKE ) {
     return MQGetPercentage(rs_ro_ratio,SmokeCurve);
  }    

  return 0;
}

/*****************************  MQGetPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         pcurve      - pointer to the curve of the target gas
Output:  ppm of the target gas
Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm) 
         of the line could be derived if y(rs_ro_ratio) is provided. As it is a 
         logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic 
         value.
************************************************************************************/ 
int  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}


BLYNK_CONNECTED()
{
  Serial.println(".");//per debug
  Blynk.syncVirtual(V10);
  Blynk.syncVirtual(V11);
  Blynk.syncVirtual(V12);
  Blynk.syncVirtual(V13);
  digitalWrite(ledblynk, LOW); //ledpin for check blynk connected
  Serial.println("Blynk Connected");
}

void reconnecting()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    Blynk.syncVirtual(V10);
    Blynk.syncVirtual(V11);
    Blynk.syncVirtual(V12);
    Blynk.syncVirtual(V13);
    digitalWrite(ledblynk, LOW); //ledpin for check blynk connected
  }
}

// Check Status LED Widget
void checkledstate()
{
stateled1=digitalRead(Relay1); // V10 Pin D2 Control Relay 1
  if (stateled1 != prevStateled1)
  {
    if (stateled1==0) Blynk.virtualWrite(V17,255); 
    if (stateled1==1) Blynk.virtualWrite(V17,0); 
  }  
  prevStateled1=stateled1;

stateled2=digitalRead(Relay2); // V11 Pin D4 Control Relay 2
  if (stateled2 != prevStateled2)
  {
    if (stateled2==0) Blynk.virtualWrite(V18,255); 
    if (stateled2==1) Blynk.virtualWrite(V18,0); 
  }  
  prevStateled2=stateled2;

stateled3=digitalRead(Relay3); // V12 Pin D5 Control Relay 3
  if (stateled3 != prevStateled3)
  {
    if (stateled3==0) Blynk.virtualWrite(V19,255); 
    if (stateled3==1) Blynk.virtualWrite(V19,0); 
  }  
  prevStateled3=stateled3;

  stateled4=digitalRead(Relay4); // V13 Pin D6 Control Relay 4
  if (stateled4 != prevStateled4)
  {
    if (stateled4==0) Blynk.virtualWrite(V20,255); 
    if (stateled4==1) Blynk.virtualWrite(V20,0); 
  }  
  prevStateled4=stateled3;
}

 void dht11Sensor(){
 float h = dht.readHumidity();
 float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)){ 
  Serial.println("Read from DHT Sensor");
  return;
  }
  Serial.print("Humidity :");
  Serial.println(h);
  Serial.print("Temperature :");
  Serial.println(t);
      
  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);
  if(t>35){
   //Blynk.virtualWrite(V15,0);
   //Blynk.virtualWrite(V17,255); 
   digitalWrite(Relay2, LOW);
   Serial.println("Relay 2 On");
   //LINE.notify("อุณหภูมิขณะนี้เกิน "+String(t)+" องศา"); 
   //Blynk.email("bugkuska@gmail.com", "ESP8622 Alert", "Alert");
   //Blynk.notify("ESP8266 Alert - แจ้งเตือน อุณภูมิเกิน 35 องศาเซลเซียล ");
  }
  if(t<35){
   //Blynk.virtualWrite(V15,1);
   //Blynk.virtualWrite(V17,0); 
   digitalWrite(Relay2, HIGH);
   Serial.println("Relay 2 Off");
   //Blynk.email("bugkuska@gmail.com", "ESP8622 Alert", "No Alert");
   //Blynk.notify("ESP8266 Alert - แจ้งเตือน อุณภูมิเกิน 35 องศาเซลเซียล ");
}
}

 void ldrSensor()
 
 {
 if( digitalRead( ldr_pin ) == 1){
      digitalWrite( Relay1,LOW);
   }
   else{
      digitalWrite( Relay1 , HIGH);
   }
   
   Serial.println( digitalRead( ldr_pin ));
   delay(100);
 }
