#include<Wire.h>
#include<LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);
void setup() {
 lcd.begin();
 //lcd.backlight();
 lcd.setCursor(0,0);//กำหนด เคอร์เซอร์อยู่ตัวอักษรตำแหน่งที่0 แถวที่1 เตรียมพิมพ์ข้อความ
lcd.print("LCD1602 l2c Test");//พิมพ์ข้อความ"LCD1602 l2c Test"
 lcd.setCursor(2,1);//กำหนด เคอร์เซอร์อยู่ตัวอักษรตำแหน่งที่1 แถวที่2 เตรียมพิมพ์ข้อความ
 lcd.print("myarduino.net");//พิมพ์ข้อความ"myarduino.net"
}

void loop() {
 

}
