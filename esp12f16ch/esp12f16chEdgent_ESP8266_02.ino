// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG
#define USE_NODE_MCU_BOARD

#include "BlynkEdgent.h"

// Variables for 74HC595 code
#define SER_Pin 14 // Serial Input pin on 74HC595 No 1 (74HC595 Pin 14). Otherwise known as DS
#define RCLK_Pin 12 // Shift Register Clock Pin on both 74HC595s (74HC595 Pin 12). Otherwise known as ST_CP
#define SRCLK_Pin 13 // Storage Register Clock Pin on both 74HC595s (74HC595 Pin 11). Otherwise known as SH_CP
#define outout_enablePin 5  // Output Enable pin on both 74HC595s (74HC595 Pin 13). Must be pulled LOW to enable the outputs

boolean registers[16]; // Zero-indexed array (0-15) which holds the state of the 16 relays

boolean just_restarted = true;  // flag to aoid turning all relays off at startup

void setup()
{
  Serial.begin(9600);
  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);
  pinMode(outout_enablePin, OUTPUT);
  digitalWrite(outout_enablePin, LOW);

  BlynkEdgent.begin();
}

void loop()
{
  BlynkEdgent.run();
}


BLYNK_CONNECTED()
{
  for (int loop = 0; loop <=  15; loop++)
  {
    Blynk.syncVirtual(loop);
  }
  just_restarted = false;
  writeRegisters();
}

BLYNK_WRITE_DEFAULT()
{
  int widget_pin = request.pin;      // Which virtual pin triggered this BLYNK_WRITE_DEFAULT callback?
  int widget_value = param.asInt();  // Get the value from the virtual pin (O = off, 1 = on)

  setRegisterPin(widget_pin, widget_value); // Set the correct register to the correct value (HIGH/LOW for on/off)
  if (!just_restarted)
  {
    writeRegisters();               // Write the stored register values out to the controller
  }
}

BLYNK_WRITE(V16) // Button labelled All OFF
{
  if (param.asInt())
  {
    for (int loop = 0; loop <=  15; loop++)
    {
      registers[loop] = HIGH;
      Blynk.virtualWrite(loop, 1);
    }
    writeRegisters();                         // Write the stored register values out to the controller
  }
  else
  {
    for (int loop = 0; loop <=  15; loop++)
    {
      registers[loop] = LOW;
      Blynk.virtualWrite(loop, 0);
    }
    writeRegisters();                         // Write the stored register values out to the controller
  }
}


void clearRegisters() // Clear registers variables
{
  for (int i = 15; i >=  0; i--)
  {
    registers[i] = LOW;;
  }
}

void writeRegisters() // Write the contents of the registers array out to the relays
{
  digitalWrite(RCLK_Pin, LOW);
  for (int i = 15; i >=  0; i--)
  {
    digitalWrite(SRCLK_Pin, LOW);
    int val = registers[i];
    digitalWrite(SER_Pin, val);
    digitalWrite(SRCLK_Pin, HIGH);
  }
  digitalWrite(RCLK_Pin, HIGH);
}

void setRegisterPin(int index, int value) //Set register variable to HIGH or LOW
{
  if (index >= 0 && index <= 15)
  {
    if (value == 0 || value == 1)
    {
      registers[index] = value; // we make it here if the index is 1-15 and thge value is 0 or 1
    }
    else
    {
      Serial.println("value must be either 0 or 1");
    }
  }
  else
  {
    Serial.println("index must be between 0 and 15");
  }
}
