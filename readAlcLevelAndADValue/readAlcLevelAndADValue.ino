/*************************************************
File:         readAlcLevelAndADValue
Description:  Receive the information automatically output by the module every second,
               and prints the alcohol level and alcohol A/D value to the serial port monitor
Note:
**************************************************/
#include <BM22S3421-1.h>

uint8_t moduleInfo[14] = {0};
uint16_t ADValue, AlcLevel, ADValueH, ADValueL;

BM22S3421_1 Alc(8, 2, 3); // 8:STATUS, Softeware serial: 2:RX, 3:TX
// BM22S3421_1 Alc(8, &Serial); // 8: STATUS, Hardware serial：Serial
// BM22S3421_1 Alc(STATUS1, &Serial1); // 22(STATUS1): STATUS, Hardware serial: Serial1 (BMduino-UNO)
// BM22S3421_1 Alc(STATUS2, &Serial2); // 25(STATUS2): STATUS, Hardware serial: Serial2 (BMduino-UNO)
void setup()
{
  Alc.begin();        // Initialize SW/HW serial, BR: 9600bps, STATUS pin: input mode
  Serial.begin(9600); // Initialize Serial, BR: 9600bps
  Serial.println("Module power on preheating...(about 3 mins)");
  preheatCountdown();// Wait for the module to warm up
  Serial.println("End of preheating.");
  Serial.println();
  // alc.writeCommand(0xe0, 0x1e, AUTO_MODE); // Set the module to automatically output data(default)
}
void loop()
{
  if (Alc.isInfoAvailable() == true) // Scaning the serial port received buffer to receive the information sent by the module
  {
    Alc.readInfoPackage(moduleInfo);
    printInfo(); // Print some information of the module
  }
}
void printInfo()
{
  /*Print alcohol level*/
  Serial.print("Alc level: ");
  AlcLevel = moduleInfo[7];
  Serial.println(AlcLevel);

  /*Print alcohol A/D Value*/
  Serial.print("Alc A/D Value: ");
  ADValue = ((uint16_t)moduleInfo[5] << 8) + moduleInfo[6];
  Serial.println(ADValue);
  Serial.println();
}
void preheatCountdown()
{
  int16_t time = 180;
  delay(1200);
  if (Alc.isInfoAvailable() == true)
  {
    while (time > 0) // AUTO_MODE
    {
      if (Alc.isInfoAvailable() == true)
      {
        Alc.readInfoPackage(moduleInfo);
        time = moduleInfo[10];
        Serial.print("time:");
        Serial.println(time);
      }
    }
  }
  else
  {
    while (time > 0) // CMD_MODE
    {
      time--;
      delay(1030);
    }
  }
}