void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);     
}

void loop() {
  float moisture_percentage;
  int sensor_analog;
  sensor_analog = analogRead(A0);
  Serial.print("Law Soil data:");
  Serial.println(sensor_analog);
  moisture_percentage = ( 100 - ( (sensor_analog/1024.00) * 100 ) );
  Serial.print("Moisture Percentage = ");
  Serial.print(moisture_percentage);
  Serial.print("%\n\n");
  delay(1000);
  
}
