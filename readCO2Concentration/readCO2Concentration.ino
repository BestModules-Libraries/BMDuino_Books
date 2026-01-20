/*************************************************
File: readCO2Concentration.ino
Description: Get the CO2 concentration, unit:ppm
Note: None
**************************************************/

#include <BM25S3321-1.h>

#define STA_PIN 22 // Input pin
// #define RX_PIN 2  // Simulate as RX pin
// #define TX_PIN 3  // Simulate as TX pin
uint16_t CO2Value = 0;

// BM25S3321_1 CO2(STA_PIN, RX_PIN, TX_PIN); // Softeware serial:8->Input pin 2->RX pin, 3->TX pin

/* BMduino-UNO */
// BM25S3321_1 CO2(STA_PIN, &Serial); // Hardware serial: Serial
BM25S3321_1 CO2(STA_PIN, &Serial1); // Hardware serial: Serial1
// BM25S3321_1 CO2(STA_PIN, &Serial2); // Hardware serial: Serial2
// BM25S3321_1 CO2(STA_PIN, &Serial3); // Hardware serial: Serial3
// BM25S3321_1 CO2(STA_PIN, &Serial4); // Hardware serial: Serial4

void setup()
{
  CO2.begin();        // Initialize module, baud rate: 9600bps
  Serial.begin(9600); // Initialize Serial, baud rate: 9600bps

  Serial.println("Module preheating...(about 60 second)");
  CO2.preheatCountdown(); // Wait for the End of module preheating.
  Serial.println("End of module preheating.");
  Serial.println();
  Serial.println("Perform initial setup.");
  // CO2.calibrateZeroPoint();
  CO2.setRangeMax(5000);
}

void loop()
{
  CO2Value = CO2.readCO2Value();
  Serial.print("CO2: ");
  Serial.print(CO2Value);
  Serial.println(" ppm");
  delay(2000);
}