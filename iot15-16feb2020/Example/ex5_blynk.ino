// NodeMCU + Blynk

#define BLYNK_PRINT Serial  
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = ""; //ชื่อ SSID ที่เราต้องการเชื่อมต่อ        
char pass[] = ""; //รหัสผ่าน WI-FI
char auth[] = ""; //Auth token from blynk app 
     
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

   
 //Connect to Blynk Server
  Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
}
void loop()
{
    if (Blynk.connected())
    {
      Blynk.run();
    }
}

BLYNK_CONNECTED()
{
  Blynk.syncAll();  
}
