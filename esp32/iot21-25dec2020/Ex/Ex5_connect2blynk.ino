#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
BlynkTimer timer;
int blynkIsDownCount=0;

// Your WiFi credentials.
char ssid[] = "";                     //ชื่อ SSID ที่เราต้องการเชื่อมต่อ        
char pass[] = "";                         //รหัสผ่าน WI-FI
char auth[] = ""; //Auth token from blynk app 

bool Connected2Blynk = false;


//Setup
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  //Serial.println();
  //Serial.println();
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
    
  // Blynk.config(blynk_token);////เริ่มการเชื่อมต่อ Blynk Server แบบปกติ
   Blynk.begin(auth, ssid, pass);
    while (Blynk.connect() == false) {
        // Wait until Blynk is connected
    }

  timer.setInterval(10000L, reconnectblynk);  //Function reconnect  
}

//blynk conneted
BLYNK_CONNECTED()
{
 Blynk.syncAll();
 
 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
 }
}

//Loop 
void loop() {    

 
  if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();//ให้เวลาของ Blynk ทำงาน
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
