/*****************************************************************
File:         readStatusPin     
Description:  LED will have different actions for different states of the module.
              a. The module is normal: flag 0. There are no intrusion alerts. Led13 lights up for 100 milliseconds.
              b. Module alarm: sign 1, object/person passing is detected, and led 13 keeps 3S
              When a module status send change is detected, the status information will be printed on the serial port monitor.
******************************************************************/ 
#include "BM22S4221-1.h"
uint8_t flag;
uint8_t STATUS=5;//Change status pin
BM22S4221_1 PIR(STATUS,6,7);//intPin 5,rxPin 6 , txPin 7, Please comment out the line of code if you don't use software Serial
//BM22S4221_1 PIR(22,&Serial1);//Please uncomment out the line of code if you use HW Serial1 on BMduino
//BM22S4221_1 PIR(29,&Serial2);//Please uncomment out the line of code if you use HW Serial2 on BMduino
//BM22S4221_1 PIR(STATUS,&Serial3);//Please uncomment out the line of code if you use HW Serial3 on BMduino
//BM22S4221_1 PIR(STATUS,&Serial4);//Please uncomment out the line of code if you use HW Serial4 on BMduino
void setup() {
  Serial.begin(9600);
  PIR.begin();
  pinMode(STATUS,INPUT);
  pinMode(13,OUTPUT);
}
void loop() {
  if(PIR.getSTATUS()==HIGH&&flag!=1)
  { 
    Serial.println("Alarm! an object passes by"); 
    flag=1;
   }
  if(flag!=0&&PIR.getSTATUS()==LOW)
  {
    flag=0;
    Serial.println("Module normal;no alarm"); 
  }
  switch(flag)
  {
    case 0:    
    digitalWrite(13,1);
    delay(100);
    digitalWrite(13,LOW);
    delay(900);
    break;
    case 1:    
    digitalWrite(13,1);
    delay(1000);
    break;
  }
}
