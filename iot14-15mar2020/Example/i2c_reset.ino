#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27 , 16, 2);
const int reset_LCD = 2; 

void setup()
{
  pinMode(reset_LCD, INPUT);
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0); 
  lcd.print("LCD1602 I2c Test"); 
  lcd.setCursor(2, 1); 
  lcd.print("myarduino.net"); 
}
void loop() {
  if (digitalRead(reset_LCD) == HIGH) {
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0); 
  lcd.print("reset.");     

  }
  delay(1000);
  lcd.setCursor(0, 0); 
  lcd.print("mmm."); 
  lcd.setCursor(2, 1); 
  lcd.print("md.sun "); 
}
