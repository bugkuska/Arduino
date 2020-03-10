// NodeMCU + Blynk App Control Relay 4 Channel
#define BLYNK_PRINT Serial  

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = ""; //ชื่อ SSID ที่เราต้องการเชื่อมต่อ        
char pass[] = ""; //รหัสผ่าน WI-FI
char auth[] = ""; //Auth token from blynk app
          
//ประกาศตัวแปร
const int Relay1 = D0;
const int Relay2 = D4;
const int Relay3 = D5;
const int Relay4 = D6;

void setup()
{

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
  pinMode(Relay1,OUTPUT); // NODEMCU PIN D0
  pinMode(Relay2,OUTPUT); // NODEMCU PIN D4
  pinMode(Relay3,OUTPUT); // NODEMCU PIN D5
  pinMode(Relay4,OUTPUT); // NODEMCU PIN D6

//Connect to Blynk Server
  Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
 
// Set Defult Relay Status
  digitalWrite(Relay1, HIGH);
  digitalWrite(Relay2, HIGH);
  digitalWrite(Relay3, HIGH);
   digitalWrite(Relay4, HIGH);
}

//Relay1
BLYNK_WRITE(V10) //Blynk Virtual Pin V10 to Button 1 Control Relay 1
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay1, LOW);
      Serial.println("Relay 1  On");
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay1, HIGH);
      Serial.println("Relay 1  Off");
    }
}

//Relay2
BLYNK_WRITE(V11) //Blynk Virtual Pin V11 to Button 2 Control Relay 2 
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay2, LOW);
      Serial.println("Relay 2  On");
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay2, HIGH);
      Serial.println("Relay 2  Off");
    }
}

//Relay3
BLYNK_WRITE(V12) //Blynk Virtual Pin V12 to Button 3 Control Relay 3
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay3, LOW);
      Serial.println("Relay 3  On");
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay3, HIGH);
      Serial.println("Relay 3  Off");
    }
}

//Relay4
BLYNK_WRITE(V13) //Blynk Virtual Pin V13 to Button 4 Control Relay 4
{
    if (param.asInt() == 0)
    {
      digitalWrite(Relay4, LOW);
      Serial.println("Relay 4  On");
    }
    if (param.asInt() == 1)
    {
      digitalWrite(Relay4, HIGH);
      Serial.println("Relay 4  Off");
    }
}
//End Relay control

void loop()
{
      Blynk.run();
}

BLYNK_CONNECTED()
{
  Blynk.syncAll();  
}
