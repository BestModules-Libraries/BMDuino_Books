/*****************************************************************
File:         readSlideAndKeyValue.ino
Description:  Connect the IIC interface of BMK54T004 to Wire of Arduino
              and connect the INT interface to PIN2 at the same time. 
              When a button is pressed, Arduino will output the 
              Key+Slider status through the Serial monitor
******************************************************************/
#include "BMK54T004.h"
BMK54T004     BMK54(2,&Wire);//intPin,Please comment out this line of code if you don't use Wire
//BMK54T004     BMK54(22,&Wire1);//Please uncomment out this line of code if you use Wire1 on BMduino
//BMK54T004     BMK54(25,&Wire2);//Please uncomment out this line of code if you use Wire2 on BMduino

void setup() 
{
  Serial.begin(9600);
  BMK54.begin(); 
  BMK54.setAllThresholdLevel(2);
} 

void loop() 
{
    if(BMK54.getINT() == 0)
    {          
        Serial.print("SlideValue:");
        Serial.println(BMK54.readSlideValue());             

        Serial.print("KeyValue:");
        Serial.println(BMK54.readKeyValue());      
    }  
}
