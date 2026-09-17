/*************************************************
File:         readGasValue.ino
Description:  Receive the information automatically output by the module every second,
              and print part of the information to the serial port monitor
Note:
**************************************************/
#include <BM22S3031-1.h>

uint8_t moduleInfo[34] = {0};
uint16_t ADValue, gasValue, gasAlarmThreshold;

BM22S3031_1 gas(8, 2, 3); // Softeware serial: 8->STATUS, 2->RX, 3->TX
// BM22S3031_1 gas(8, &Serial); //Please uncomment out this line of code if you use HW Serial on BMduino
// BM22S3031_1 gas(STATUS1, &Serial1); //Please uncomment out this line of code if you use HW Serial1 on BMduino
// BM22S3031_1 gas(STATUS2, &Serial2); //Please uncomment out this line of code if you use HW Serial2 on BMduino
void setup()
{
  gas.begin(); // Initialize Softeware serial, baud rate 9600bps, Set pin 8 as input mode

  Serial.begin(9600); // Initialize Serial, baud rate 9600bps

  Serial.println("Module preheating...(about 3 mins)");
  gas.preheatCountdown(); // Wait for the End of module preheating.
  Serial.println("End of module preheating.");
  Serial.println();
  delay(1200);
}
void loop()
{
  /* Scaning the serial port received buffer to receive the information sent by the module */
  if (gas.isInfoAvailable() == true)
  {
    gas.readInfoPackage(moduleInfo);
    printInfo(); // Print some information of the module
  }
}

void printInfo()
{
  /*Print gas alarm threshold (PPM)*/
  Serial.print("Gas alarm threshold: ");
  gasAlarmThreshold = (moduleInfo[23] << 8) + moduleInfo[24];
  Serial.print(gasAlarmThreshold);
  Serial.println(" PPM");

  /*Print current gas concentration (PPM)*/
  Serial.print("Gas concentration: ");
  gasValue = (moduleInfo[9] << 8) + moduleInfo[10];
  Serial.print(gasValue);
  Serial.println(" PPM");

  /*Print Gas AD Value*/
  Serial.print("Gas AD Value: ");
  ADValue = (moduleInfo[5] << 8) + moduleInfo[6];
  Serial.println(ADValue);
  Serial.println();
}
