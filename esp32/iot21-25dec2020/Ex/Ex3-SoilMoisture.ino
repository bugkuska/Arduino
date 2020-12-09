#define INPUT_1 36
#define INPUT_2 39
#define INPUT_3 34
#define INPUT_4 33

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);     
}
void loop() {
  float moisture_percentage1;
  int sensor_analog1;
  sensor_analog1 = analogRead(INPUT_1);
  Serial.print("Law Soil data 1:");
  Serial.println(sensor_analog1);
  moisture_percentage1 = ( 100 - ( (sensor_analog1/4095.00) * 100 ) );
  
  Serial.print("Moisture Percentage 1 = ");
  Serial.print(moisture_percentage1);
  Serial.print("%\n\n");
  delay(1000);  


  float moisture_percentage2;
  int sensor_analog2;
  sensor_analog2 = analogRead(INPUT_2);
  Serial.print("Law Soil data 2:");
  Serial.println(sensor_analog2);
  moisture_percentage2 = ( 100 - ( (sensor_analog2/4095.00) * 100 ) );
  
  Serial.print("Moisture Percentage 2= ");
  Serial.print(moisture_percentage2);
  Serial.print("%\n\n");
  delay(1000);  

  float moisture_percentage3;
  int sensor_analog3;
  sensor_analog3 = analogRead(INPUT_3);
  Serial.print("Law Soil data 3:");
  Serial.println(sensor_analog3);
  moisture_percentage3 = ( 100 - ( (sensor_analog3/4095.00) * 100 ) );
  
  Serial.print("Moisture Percentage 3= ");
  Serial.print(moisture_percentage3);
  Serial.print("%\n\n");
  delay(1000);  

  float moisture_percentage4;
  int sensor_analog4;
  sensor_analog4 = analogRead(INPUT_4);
  Serial.print("Law Soil data 4:");
  Serial.println(sensor_analog4);
  moisture_percentage4 = ( 100 - ( (sensor_analog4/4095.00) * 100 ) );
  
  Serial.print("Moisture Percentage 4= ");
  Serial.print(moisture_percentage4);
  Serial.print("%\n\n");
  delay(1000);  
}
