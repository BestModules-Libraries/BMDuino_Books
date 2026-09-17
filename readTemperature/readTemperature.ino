/*****************************************************************
File:         readTemperature.ino
Description:  1.I2C(100k) is used to communicate with BMH63K203.
              2.hardware Serial (BAUDRATE 9600) is used to communicate with Serial port monitor.            
Note:
******************************************************************/
#include <BMH06203.h>

//BMH06203 mytherm(&Wire); //Please uncomment out this line of code if you use Wire on BMduino
BMH06203 mytherm(&Wire1);//Please uncomment out this line of code if you use Wire1 on BMduino
// BMH06203 mytherm(&Wire2);//Please uncomment out this line of code if you use Wire2 on BMduino

void setup() 
{
  /*Please use it in IIC mode */
   mytherm.begin();      
   Serial.begin(9600);     // start serial for output
}

void loop()
{
//   Get OBJtemp data
   Serial.print("The OBJTemp is:");
   Serial.print(mytherm.readTemperature(OBJ_TEMP));
   Serial.println("℃");
}
