#include <WiFi.h>

char ssid[] = ""; //ชื่อ SSID ที่เราต้องการเชื่อมต่อ        
char pass[] = ""; //รหัสผ่าน WI-FI
 
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
 
 }

void loop()
{
 //run program here 
}
