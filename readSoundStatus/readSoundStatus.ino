/*****************************************************************
File:        readSoundStatus.ino 
Description:  1.  The Sound Detector is a small board that combines a microphone and some processing circuitry.
                  It provides not only an audio output, but also a binary indication of the presence of sound,
                  an analog representation of it's amplitude and I2C interface.  
              2.hardware Serial (BAUDRATE 115200) is used to communicate with Serial port monitor.When there is a sound, 
              read the sensor data displayed on the serial port monitor.
              3.This project introduces the use of I2C interface.
          
note:connection method(Sound Detector:Arduino pin):GND:GND VDD:3V3 SDA:SDA1 SCL:SCL1 STA:STATUS1
******************************************************************/
#include <BMV23M001.h>

//BMV23M001 soundDetector(2,&Wire);//Please comment out this line of code if you don't use Wire
BMV23M001 soundDetector(22,&Wire1);//Please uncomment out this line of code if you use Wire1 on BMduino
//BMV23M001 soundDetector(25,&Wire2);//Please uncomment out this line of code if you use Wire2 on BMduino

void setup() 
{
  soundDetector.begin();   //IIC rate 100kHz
  Serial.begin(115200);  // start serial for Debug
}

void loop()
{
     if(soundDetector.readSoundStatus()==StatusFAIL)  //read sound status
     {
       Serial.println("Communication fail,please check your connection!");
     }
     else
     {

       Serial.print("Sound Status: ");
        if(soundDetector.readSoundStatus())
        {
          Serial.println("Sound!");
        }
        else
        { 
          Serial.println("Quiet!");
        }
     }
    // pause for 10 ms
}
