#include <Wire.h>

#define MPU6500_I2C_ADDRESS 0x68   // Default MPU-6050 address
#define MPU6500_WHO_AM_I 0x75      // R Who am I address

void setup()
{
  Wire.begin();        // Initiate wire library
  Serial.begin(115200);  // Initiate serial port 
  WhoAmI();            // Verifies identity of device
}
void loop()
{
  //blank
}

void WhoAmI(){
  uint8_t waiByte;                                    // Data will go here
  MPU6050_Read(MPU6500_WHO_AM_I, &waiByte);           // Get data
  Serial.print(F("Device WhoAmI reports as: 0x"));    // 
  Serial.println(waiByte,HEX);                        // Report WhoAmI data
  }

void MPU6050_Read(int address,uint8_t *data){            // Read from MPU6050. Needs register address, data array
  int size = sizeof(*data);                              //
  Wire.beginTransmission(MPU6500_I2C_ADDRESS);           // Begin talking to MPU6050
  Wire.write(address);                                   // Set register address
  Wire.endTransmission(false);                           // Hold the I2C-bus
  Wire.requestFrom(MPU6500_I2C_ADDRESS, size, true);     // Request bytes, release I2C-bus after data read
  int i = 0;                                             //
  while(Wire.available()){                               //
    data[i++]=Wire.read();                               // Add data to array
  }
}