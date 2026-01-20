/*****************************************************************************************************
File:             readLux.ino
Description:      This example demonstrates how to initialize the VEML7700 and then get the ambient light lux.
Note:             
******************************************************************************************************/
#include "BME82M131.h"

BME82M131 ALS(2, &Wire);     //intPin, Wire. Please comment out this line of code if you don't use Wire
// BME82M131 ALS(22, &Wire1); //Please uncomment out this line of code if you use Wire1 on BMduino
// BME82M131 ALS(25, &Wire1); //Please uncomment out this line of code if you use Wire2 on BMduino

void setup()
{
  Serial.begin(9600);
  ALS.begin();
  Serial.print(ALS.getNumber());
  Serial.print(" modules are ");
  Serial.println("Connected!");
}
void loop()
{
  Serial.print("Lux1:");
  Serial.println( ALS.readLux(1));     // Read the lux from the sensor1 and print it
  delay(1000);
  
}