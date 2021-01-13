
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SoftwareSerial.h> // Arduino IDE <1.6.6
#include <PZEM004T.h>

BlynkTimer timer;

PZEM004T pzem(D5,D6);  // (RX,TX) connect to TX,RX of PZEM
IPAddress ip(192,168,1,1);

float voltage_blynk=0;
float current_blynk=0;
float power_blynk=0;
float energy_blynk=0;

#include <DHT.h>
#define DHTPIN D0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#include <SimpleTimer.h>

int blynkIsDownCount=0;

// You should get Auth Token in the Blynk App.
char auth[] = "";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "";
char pass[] = "";

unsigned long lastMillis = 0;

//ประกาศตัวแปร
const int Relay1 = D1;
const int Relay2 = D2;
const int Relay3 = D4;
const int Relay4 = D7;

boolean stateled = 0;
boolean prevStateled = 0;
boolean stateled2=0;
boolean prevStateled2 = 0;
boolean stateled3=0;
boolean prevStateled3 = 0;
boolean stateled4=0;
boolean prevStateled4 = 0;

void setup()
{
  Serial.begin(115200);
  pzem.setAddress(ip);

  dht.begin();  //เริ่มอ่านข้อมูล DHT Sensor
  
  //Blynk.begin(auth, ssid, pass);
  Blynk.begin(auth, ssid, pass,"blynkfree.info", 8080);
  timer.setInterval(1000L, ReadPzem);
  timer.setInterval(1000L, sendSensor); //Send sensor data to display on app blynk
  timer.setInterval(30000L, reconnecting);  //Function reconnect

  // Setup Pin Mode
  pinMode(Relay1,OUTPUT); // NODEMCU PIN D1
  pinMode(Relay2,OUTPUT); // NODEMCU PIN D2
  pinMode(Relay3,OUTPUT); // NODEMCU PIN D4
  pinMode(Relay4,OUTPUT); // NODEMCU PIN D7
   
  // Set Defult Relay Status
  digitalWrite(Relay1, HIGH);
  digitalWrite(Relay2, HIGH);
  digitalWrite(Relay3, HIGH);
  digitalWrite(Relay4, HIGH);
}

BLYNK_WRITE(V10) //Blynk Virtual Pin V10 to Button 1 Control Relay 1
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay1, LOW);
      Serial.println("Relay 1 On");   
      Blynk.virtualWrite(V13,255); 
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay1, HIGH);
      Serial.println("Relay 1 Off");    
       Blynk.virtualWrite(V13,0); 
    }
}
BLYNK_WRITE(V11) //Blynk Virtual Pin V11 to Button 2 Control Relay 2 
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay2, LOW);
      Serial.println("Relay 2 On");  
      Blynk.virtualWrite(V14,255);
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay2, HIGH);
      Serial.println("Relay 2 Off");    
         Blynk.virtualWrite(V14,0);
    }
}
BLYNK_WRITE(V15) //Blynk Virtual Pin V15 to Button 3 Control Relay 3
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay3, LOW);
      Serial.println("Relay 3 On");  
      Blynk.virtualWrite(V17,255);   
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay3, HIGH);
      Serial.println("Relay 3 Off");    
      Blynk.virtualWrite(V17,0);
    }
}

BLYNK_WRITE(V16) //Blynk Virtual Pin V16 to Button 4 Control Relay 4
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay4, LOW);
      Serial.println("Relay 4 On");    
      Blynk.virtualWrite(V18,255);
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay4, HIGH);
      Serial.println("Relay 4 Off");    
      Blynk.virtualWrite(V18,0); 
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

BLYNK_CONNECTED()
{
  //Blynk.syncAll();
  Serial.println(".");//per debug
  Blynk.syncVirtual(V10);
  Blynk.syncVirtual(V11);
  Blynk.syncVirtual(V15);
  Blynk.syncVirtual(V16);
 
}

void reconnecting()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    //Blynk.syncAll();

    Blynk.syncVirtual(V10);
    Blynk.syncVirtual(V11);
    Blynk.syncVirtual(V15);
    Blynk.syncVirtual(V16);
  }
}
void sendSensor(){
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)){ 
  Serial.println("Read from DHT Sensor");
  return;
  }
  Serial.print("Temperature");
  Serial.println(t);
   Serial.print("Humidity");
  Serial.println(h);
  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);
  if(t>40){
   Blynk.notify("ESP8266 Alert - Temperature over 40 C ");
  }
}

void ReadPzem() {
/// Read meter PZEM 
    float v = pzem.voltage(ip);    
    if (v < 0.0) v = 0.0;
    Serial.print(v);Serial.print("V; ");
    if(v >= 0.0){   voltage_blynk =v; } //V

  
    float i = pzem.current(ip);
    if(i >= 0.0){ Serial.print(i);Serial.print("A; "); } 
    if(i >= 0.0){ current_blynk=i;    }  //A     
                                                                                                                 
    
    float p = pzem.power(ip);
    if(p >= 0.0){ Serial.print(p);Serial.print("W; "); } 
    if(p >= 0.0){power_blynk=p;       } //kW
 
    
    float e = pzem.energy(ip);      
    if(e >= 0.0){ Serial.print(e);Serial.print("Wh; "); }     
    if(e >= 0.0){  energy_blynk =e;  } ///kWh

    Serial.println();
    
    delay(1000);

      //Publish data every 10 seconds (10000 milliseconds). Change this value to publish at a different interval.
          if (millis() - lastMillis > 10000) {
            lastMillis = millis();           
            Blynk.virtualWrite(V51, voltage_blynk);
            Blynk.virtualWrite(V52, current_blynk  );            
            Blynk.virtualWrite(V53, power_blynk);
            Blynk.virtualWrite(V54, energy_blynk  );
            Blynk.virtualWrite(V55, lastMillis  );      
          }         
}

// Check Status LED Widget
void checkledstate()
{
  stateled=digitalRead(Relay1); // V10 Pin D1 Control Relay 1
  if (stateled != prevStateled)
  {
    if (stateled==0) Blynk.virtualWrite(V13,255);
    if (stateled==1) Blynk.virtualWrite(V13,0);
  }
  prevStateled=stateled;
  
stateled2=digitalRead(Relay2); // V11 Pin D2 Control Relay 2
  if (stateled2 != prevStateled2)
  {
    if (stateled2==0) Blynk.virtualWrite(V14,255); 
    if (stateled2==1) Blynk.virtualWrite(V14,0); 
  }  
  prevStateled2=stateled2;

stateled3=digitalRead(Relay3); // V15 Pin D3 Control Relay 3
  if (stateled3 != prevStateled3)
  {
    if (stateled3==0) Blynk.virtualWrite(V17,255); 
    if (stateled3==1) Blynk.virtualWrite(V17,0); 
  }  
  prevStateled3=stateled3;

stateled4=digitalRead(Relay4); // V16 Pin D4 Control Relay 4
  if (stateled4 != prevStateled4)
  {
    if (stateled4==0) Blynk.virtualWrite(V18,255); 
    if (stateled4==1) Blynk.virtualWrite(V18,0); 
  }  
  prevStateled4=stateled4;
}
