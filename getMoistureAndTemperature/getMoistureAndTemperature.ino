/*************************************************
File:             getMoistureAndTemperature.ino
Description:      Development board is connected with a single module 
                  to obtain the soil moisture detection value and temperature value of the module 
                  and display them on the serial port monitor
Note:               
**************************************************/
#include "BME34M101.h"

//BME34M101 mySoilMoistureSensor(5,4);//Please comment out this line of code if you don't use Serial
BME34M101 mySoilMoistureSensor(&Serial1);//Please comment out this line of code if you  use Serial1 on BMduino
//BME34M101 mySoilMoistureSensor(&Serial2);//Please comment out this line of code if you  use Serial2 on BMduino

void setup()
{
  Serial.begin(115200);
    
  mySoilMoistureSensor.begin();
  Serial.println("Check whether the module is connected,waiting...");  
  while(mySoilMoistureSensor.isConnected() == false)
  {
    delay(1000);
  }
  Serial.println("Module is connecting.");
}

void loop() 
{
  Serial.print("Get soil moisture value is: ");
  Serial.println(mySoilMoistureSensor.getMoisture());
  delay(100);

  Serial.print("Get module temperature value is: ");
  Serial.println(mySoilMoistureSensor.getTemperature());
  delay(1000);
}
