/*************************************************
File:             readDistance.ino
Description:      1.The Wire is used for IIC communication with the BML36M001.
                  2.Use a hardware serial port (BAUDRATE 115200) to communicate with the serial monitor.
                  When the distance data is ready, the distance data is read and displayed on the serial monitor.
connection method： i2cPort:Wire  intPin:D2         
**************************************************/
#include "BML36M001.h"

BML36M001 BML36(2,&Wire);                            //i2cPort,intPin,Please comment out this line of code if you don't use HW Wire
//BML36M001 BML36(22,&Wire1);                           //Please uncomment out this line of code if you use HW Wire1 on BMduino
//BML36M001 BML36(25,&Wire2);                           //Please uncomment out this line of code if you use HW Wire1 on BMduino
void setup(void)
{
  Serial.begin(115200);                              //Set the communication rate of the serial monitor to 9600 baud
  BML36.begin();                                    //Initialize BML36M001   
  BML36.setDistanceModeLong();                         //Set the probe mode to be long-distance.  Detection range:40~4000(max)（Unit: mm）                                                
  BML36.startRanging();                                //Start measuring the distance         
}

void loop(void)
{    
if (BML36.getINT())                               //Ask if the data is ready  0：data is not ready 1：data is ready
  {
   Serial.print("Distance(mm):");                               //Print string:“Distance(mm):”        
   Serial.println(BML36.readDistance());                        //Gets distance data（Unit: mm）
   BML36.clearInterrupt();                                             //Clear the BML36M001 interrupt flag bit   
 } 
}
