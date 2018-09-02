#include<Wire.h>
#define MPU 0x68

int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

void setup(){
    Serial.begin(115200);

    Wire.setClock(400000);
    Wire.begin();
    delay(50);
    Wire.beginTransmission(MPU);
    Wire.write(0x6B);  // PWR_MGMT_1 register
    Wire.write(0x00);  // set 0x6B to zero -> wakes up the MPU
    Wire.endTransmission(true);
}

void loop(){
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);  // register 0x3B (ACCEL_XOUT_H), 큐에 데이터 기록
    Wire.endTransmission(false);    //stay connected
    Wire.requestFrom(MPU,14,true);  //request data

    AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
    AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

    // Serial.print("AcX = "); Serial.print(AcX);
    // Serial.print(" | AcY = "); Serial.print(AcY);
    // Serial.print(" | AcZ = "); Serial.print(AcZ);
    // Serial.print(" | Tmp = "); Serial.print(Tmp/340.00+36.53);  
    // Serial.print(" | GyX = "); Serial.print(GyX);
    // Serial.print(" | GyY = "); Serial.print(GyY);
    // Serial.print(" | GyZ = "); Serial.println(GyZ);
    // Serial.println();
    Serial.print(AcX); Serial.print(" ");
    Serial.print(AcY); Serial.print(" ");
    // Serial.print(AcZ); Serial.print(" ");
    // Serial.print(GyX); Serial.print(" ");
    // Serial.print(GyY); Serial.print(" ");
    // Serial.print(GyZ); Serial.print(" ");
    Serial.println();
    delay(20);
}
