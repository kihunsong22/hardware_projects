// #include <Arduino.h>
// #include <Wire.h>
// #include <SPI.h>
// #include <Adafruit_Sensor.h>
// #include "Adafruit_BME680.h"

// Adafruit_BME680 bme; // I2C

// #define SEALEVELPRESSURE_HPA (1013.25)

// void setup()
// {
//   delay(100);

//   Serial.begin(115200);
//   Serial.println();
//   Serial.println("ESP8266-BME680-sensor-board");

//   if (!bme.begin(0x76)) 
//   {
//     Serial.println("Could not find a valid BME680 sensor, check wiring!");
//     while (1);
//   }
  
//   bme.setTemperatureOversampling(BME680_OS_8X);
//   bme.setHumidityOversampling(BME680_OS_2X);
//   bme.setPressureOversampling(BME680_OS_4X);
//   bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
//   bme.setGasHeater(320, 150); // 320*C for 150 ms
// }

// void loop()
// {
//   if (! bme.performReading()) 
//   {
//     Serial.println("Failed to perform reading :(");
//     delay(2000);
//     return;
//   }
//   Serial.print("Temperature = ");
//   Serial.print(bme.temperature);
//   Serial.println(" *C");

//   Serial.print("Pressure = ");
//   Serial.print(bme.pressure / 100.0);
//   Serial.println(" hPa");

//   Serial.print("Humidity = ");
//   Serial.print(bme.humidity);
//   Serial.println(" %");

//   Serial.print("Gas = ");
//   Serial.print(bme.gas_resistance / 1000.0);
//   Serial.println(" KOhms");

//   Serial.print("Approx. Altitude = ");
//   Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
//   Serial.println(" m");

//   Serial.println();
//   delay(2000);
// }