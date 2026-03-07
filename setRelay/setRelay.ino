/*************************************************
File:             controlRelay.ino
Description:      A relay module is controlled by IIC as an LED switch to control the switching of the LED once per second.
Note:               
**************************************************/

#include <BMP75M131.h>
//BMP75M131     myRelay(&Wire);//intPin,Please comment out this line of code if you don't use Wire
BMP75M131 myRelay(&Wire1);//Please uncomment out this line of code if you use Wire1 on BMduino
//BMP75M131     myRelay(&Wire2);//intPin,Please comment out this line of code if you don't use Wire

void setup() {
  Serial.begin(9600);   //Serial debug output
  
  myRelay.begin();
}

void loop() 
{
  uint8_t  relyaStatus;

  myRelay.setRelaySwitch(1, 1);
  Serial.println("Close the relay switch");
  relyaStatus = myRelay.getRelayStatus(1);  
  Serial.print("Get the relay switch status valus is: ");
  Serial.println(relyaStatus);
   
  delay(1000);
  
  myRelay.setRelaySwitch(1, 0);
  Serial.println("Open the relay switch");
  relyaStatus = myRelay.getRelayStatus(1);
  Serial.print("Get the relay switch status valus is: ");
  Serial.println(relyaStatus);
  
  delay(1000);
}
