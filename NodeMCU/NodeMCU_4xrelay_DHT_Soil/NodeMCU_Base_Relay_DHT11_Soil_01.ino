//*****Define MCU Digital Pin*******//
//Relay1_btn1         D1
//Relay2_btn2         D2
//Relay3_btn3         D3
//Relay4_btn4         D4
//DHT11               D5
//ว่าง                  D0
//ว่าง                  D6
//ว่าง                  D7
//*****Define MCU Digital Pin*******//

//*****Define Blynk Virtual Pin*****//
//V0  ค่าความชื้นในดิน
//V1  ไฟสถานะปุ่ม 1
//V2  ปุ่ม เปิด-ปิด 1
//V3  ไฟสถานะปุ่ม 2
//V4  ปุ่ม เปิด-ปิด 2
//V5  ไฟสถานะปุ่ม 3
//V6  ปุ่ม เปิด-ปิด 3
//V7  ไฟสถานะปุ่ม 4
//V8  ปุ่ม เปิด-ปิด 4
//V9  Temperature
//V10 Humidity
//V11 Auto&Manual btn1
//V12 Slider btn1
//V13 Auto&Manual btn2
//V14 Slider btn2
//*****Define Blynk Virtual Pin*****//

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
//#include <SimpleTimer.h>
BlynkTimer timer;
int blynkIsDownCount = 0;

//Slider for set Soil Moisture limit
//ตรวจสอบเงื่อนไขสำหรับเปิดปิดปุ่มที่ 1 ในโหมดการทำงานอัตโนมัติ
bool switchStatus1 = 0; // 0 = manual,1=auto
int SoilsensorLimit1 = 0;
bool manualSwitch1 = 0;

//DHT11
#include <DHT.h>
#define DHTPIN D5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
//Slider for set temperature limit
//ตรวจสอบเงื่อนไขสำหรับเปิดปิดปุ่มที่ 2 ในโหมดการทำงานอัตโนมัติ
bool switchStatus2 = 0; // 0 = manual,1=auto
int templimit = 0;
bool manualSwitch2 = 0;

//BTN1
#define Relay1_btn1   D1
#define Widget_LED_btn1 V1          //ไฟสถานะปุ่ม 1
#define Widget_Btn_btn1 V2          //ปุ่ม เปิด-ปิด 1
WidgetLED LedBlynkbtn1(Widget_LED_btn1);

//BTN2
#define Relay2_btn2   D2
#define Widget_LED_btn2 V3          //ไฟสถานะปุ่ม 2
#define Widget_Btn_btn2 V4          //ปุ่ม เปิด-ปิด 2
WidgetLED LedBlynkbtn2(Widget_LED_btn2);

//BTN3
#define Relay3_btn3   D3
#define Widget_LED_btn3 V5          //ไฟสถานะปุ่ม 3
#define Widget_Btn_btn3 V6          //ปุ่ม เปิด-ปิด 3
WidgetLED LedBlynkbtn3(Widget_LED_btn3);

//BTN4
#define Relay4_btn4   D4
#define Widget_LED_btn4 V7          //ไฟสถานะปุ่ม 4
#define Widget_Btn_btn4 V8          //ปุ่ม เปิด-ปิด 4
WidgetLED LedBlynkbtn4(Widget_LED_btn4);

//V9  Humidity
//V10 Temperature

//Wi-Fi and blynk credentials   
char auth[] = "";     //Blynk Token
char ssid[] = "";     //Wi-Fi                
char pass[] = "";     //Password เชื่อมต่อ Wi-Fi                         

//Setup Function
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
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
   
   //Start read DHT11
  dht.begin();  //เริ่มอ่านข้อมูล DHT Sensor
  
  Blynk.begin(ssid, pass,auth);
  timer.setInterval(30000L, reconnecting);  
  timer.setInterval(5000L, dhtSensorData);
  timer.setInterval(5000L, getsoilSensorData);
}

// Update switchStatus1 on Soil Moisture
BLYNK_WRITE(V11)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}

// Update Soil setting
BLYNK_WRITE(V12)
{   
  SoilsensorLimit1 = param.asInt(); // Get value as integer
}

// Update manualSwitch
BLYNK_WRITE(V2)
{
  manualSwitch1 = param.asInt();
}

void getsoilSensorData()
{
  float soil_percentage;
  int sensor_analog;
  sensor_analog = analogRead(A0);
  Serial.print("Law Soil data:");
  Serial.println(sensor_analog);
  soil_percentage = ( 100 - ( (sensor_analog/1024.00) * 100 ) );
  Blynk.virtualWrite(V0,(soil_percentage)); 
  Serial.print("Soil Percentage = ");
  Serial.print(soil_percentage);
  Serial.print("%\n\n");
  delay(1000);
    
if(switchStatus1)
  {
    // auto
    if(soil_percentage < SoilsensorLimit1)
    {
        digitalWrite(Relay1_btn1, LOW);  
        Blynk.virtualWrite(V2, 1);
                
        Blynk.setProperty(Widget_LED_btn1, "color", "#C70039");
        Blynk.setProperty(Widget_LED_btn1, "label", "เปิดปุ่มที่ 1");
        LedBlynkbtn1.on(); 
    }  
    else
    {
        digitalWrite(Relay1_btn1, HIGH);
        Blynk.virtualWrite(V2, 0);
        Blynk.virtualWrite(Widget_LED_btn1, 0);
        Blynk.setProperty(Widget_LED_btn1, "label", "ปิดปุ่มที่ 1");                       
        LedBlynkbtn1.off();  
    }
  }
  else
  {
    if(manualSwitch1)
    {
        digitalWrite(Relay1_btn1, LOW);        
        Blynk.setProperty(Widget_LED_btn1, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_btn1, "label", "เปิดปุ่มที่ 1");
        LedBlynkbtn1.on(); 
    }
    else
    {
        digitalWrite(Relay1_btn1, HIGH);
        Blynk.setProperty(Widget_LED_btn1, "label", "ปิดปุ่มที่ 1");                       
        LedBlynkbtn1.off();
    }
    // manaul
  }
}

// Update switchStatus2 on Temperature
BLYNK_WRITE(V13)
{   
  switchStatus2 = param.asInt(); // Get value as integer
}

// Update temperture setting
BLYNK_WRITE(V14)
{   
  templimit = param.asInt(); // Get value as integer
}

// Update manualSwitch2
BLYNK_WRITE(V4)
{
  manualSwitch2 = param.asInt();
}

//DHT11
void dhtSensorData(){
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)){ 
 Serial.println("Read from DHT Sensor 1");
  return;
  }
  Blynk.virtualWrite(V9, t);
  Blynk.virtualWrite(V10, h);
  
  if(switchStatus2)
  {
    // auto
    if(t > templimit)
    {
        digitalWrite(Relay2_btn2, LOW);  
        Blynk.virtualWrite(V4, 1);
        Blynk.setProperty(Widget_LED_btn2, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_btn2, "label", "เปิดปุ่มที่ 2");
        LedBlynkbtn2.on(); 

    }  
    else
    {
        digitalWrite(Relay2_btn2, HIGH);
        Blynk.virtualWrite(V4, 0);
        Blynk.virtualWrite(Widget_LED_btn2, 0);
        Blynk.setProperty(Widget_LED_btn2, "label", "ปิดปุ่มที่ 2");                       
        LedBlynkbtn2.off();  
    }
  }
  else
  {
    if(manualSwitch2)
    {
        digitalWrite(Relay2_btn2, LOW);
        Blynk.setProperty(Widget_LED_btn2, "color", "#00FF00");
        Blynk.setProperty(Widget_LED_btn2, "label", "เปิดปุ่มที่ 2");
        LedBlynkbtn2.on(); 
    }
    else
    {
        digitalWrite(Relay2_btn2, HIGH);
        Blynk.setProperty(Widget_LED_btn2, "label", "ปิดปุ่มที่ 2");                       
        LedBlynkbtn2.off();
    }
    // manaul
  }
}


//****BUTTON ON/OFF btn3****
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

//****BUTTON ON/OFF btn4****
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

//Blynk connected
BLYNK_CONNECTED()
{
    Serial.println(".");//per debug
    Blynk.syncAll();

 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
 }
}

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

void loop()
{
  if (Blynk.connected())
  {
    Blynk.run();
    Blynk.syncAll();
  }
   timer.run();  
}
