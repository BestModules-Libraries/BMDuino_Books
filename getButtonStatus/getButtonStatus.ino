/*************************************************
File:             geyButtonStatus.ino
Description:      The development board is connected to a button,when receiving the interrupt signal, read the button trigger status whth IIC
Note:               
**************************************************/

#include <BMK22M131.h>
#include <Wire.h>
BMK22M131 myButton(2,&Wire);
uint8_t int_flag = 0;
uint8_t buttonStatus;


void setup() {
  Serial.begin(9600);   //Serial debug output
  attachInterrupt(digitalPinToInterrupt(2), ButtonInt, FALLING);
  myButton.begin();
  myButton.ledButtonMode(1);
  Serial.println("Check whether the module is connected,waiting...");
  if(myButton.isConnected()==true)
  {Serial.println("The module is connecting");}

}

void loop() {
  if(int_flag)
  {
    int_flag = 0;
    buttonStatus= myButton.getButtonStatus(1);
      if(buttonStatus == 1)
      {
        Serial.println("The button is short pressed.");
      }
      else
      {
        Serial.println("The button is long pressed.");
      }
    }
}


void ButtonInt(){
  int_flag = 1;
}
