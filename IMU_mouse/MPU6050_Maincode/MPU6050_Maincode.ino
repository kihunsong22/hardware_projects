#include<Wire.h>
#define MPU 0x68

int16_t AcX, AcY, AcZ, offX=0, offY=0, offZ=0, calX, calY, calZ;
int32_t velX=0, velY=0;
int16_t posX=0, posY=0;
// int16_t Tmp,GyX,GyY,GyZ;

float EMA_a = 0.06;
int stableX=0, stableY=0;

void calibrate();
int EMA_function(float alpha, int16_t latest, int16_t stored);

void setup(){
    Serial.begin(115200);

    // Wire.setClock(400000);
    Wire.begin();
    delay(10);
    Wire.beginTransmission(MPU);
    Wire.write(0x6B);  // PWR_MGMT_1 register
    Wire.write(0x00);  // set 0x6B to zero -> wakes up the MPU
    Wire.endTransmission(true);

    calibrate();  //calibrate offset
}

void loop(){
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);  // register 0x3B (ACCEL_XOUT_H), 큐에 데이터 기록
    Wire.endTransmission(false);
    Wire.requestFrom(MPU,14,true);  //request data

    AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcX = AcX-offX;
    AcY = AcY-offY;

    // calX = EMA_function(EMA_a, AcX, calX);
    // calY = EMA_function(EMA_a, AcY, calY);

    if(AcX>600 || AcX<-600){
        velX += AcX;
        stableX=0;
    }else{
        stableX++;
    }
    velX = stableX>5 ? (velX/1.5) : velX;
    
    if(AcY>600 || AcY<-600){
        velY += AcY;
        stableY=0;
    }else{
        stableY++;
    }
    velY = stableY>5 ? (velY/1.5) : velY;

    posX += (velX/100);
    posY += (velY/100);

    // Serial.print(AcX); Serial.print(" ");
    // Serial.print(AcY); Serial.print(" ");
    // Serial.print(calX); Serial.print(" ");
    // Serial.print(calY); Serial.print(" ");
    
    Serial.print(velX); Serial.print(" ");
    Serial.print(velY); Serial.print(" ");

    Serial.print(posX); Serial.print(" ");
    Serial.print(posY); Serial.print(" ");

    Serial.println();
    delay(6);
}

void calibrate(){
    delay(200);
    for(int i=0; i<20; i++){
        Wire.beginTransmission(MPU);
        Wire.write(0x3B);
        Wire.endTransmission(false);
        Wire.requestFrom(MPU,14,true);

        offX += Wire.read()<<8|Wire.read();
        offY += Wire.read()<<8|Wire.read();
        offZ += Wire.read()<<8|Wire.read();
    }

    offX /= 20;
    offY /= 20;
    offZ /= 20;

    Serial.println("==========================================");
    Serial.print(offX); Serial.print(" ");
    Serial.print(offY); Serial.print(" ");
    Serial.print(offZ); Serial.println(" ");
    Serial.println("==========================================");

    delay(5);
}

int EMA_function(float alpha, int16_t latest, int16_t stored){
    return round( alpha * latest) + round((1-alpha) * stored );
}
