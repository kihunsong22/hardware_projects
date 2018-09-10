#include<Wire.h>
#include<Mouse.h>
#define MPU 0x68

int16_t AcX, AcY, AcZ, offX=0, offY=0, offZ=0, calX, calY, calZ;
int32_t velX=0, velY=0;
int16_t posX=0, posY=0;
// int16_t Tmp,GyX,GyY,GyZ;

float EMA_a = 0.06, mouse_sen=0.0006;
int stableX=0, stableY=0;
int buttonL=0, buttonR=0, bstateL=0, bstateR=0;  //button left, right

void calibrate();  //calibrate initial offset
int EMA_function(float alpha, int16_t latest, int16_t stored){
    return round( alpha * latest) + round((1-alpha) * stored );
}

void setup(){
    Wire.setClock(400000);
    Wire.begin();
    delay(10);
    Wire.beginTransmission(MPU);
    Wire.write(0x6B);  // PWR_MGMT_1 register
    Wire.write(0x00);  // set 0x6B to zero -> wakes up the MPU
    Wire.endTransmission(true);

    calibrate();
}

void loop(){
    buttonL = digitalRead(5);
    buttonR = digitalRead(6);
    if(buttonL != bstateL){
        if(buttonL){
            Mouse.press(MOUSE_LEFT);
        }else{
            Mouse.release(MOUSE_LEFT);
        }
    }
    if(buttonR != bstateR){
        if(buttonR){
            Mouse.press(MOUSE_RIGHT);
        }else{
            Mouse.release(MOUSE_RIGHT);
        }
    }
    bstateL = buttonL;
    bstateR = buttonR;

    Wire.beginTransmission(MPU);
    Wire.write(0x3B);  // register 0x3B (ACCEL_XOUT_H), save data to queue
    Wire.endTransmission(false);
    Wire.requestFrom(MPU,14,true);  //request data from MPU

    AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcX = AcX-offX;
    AcY = AcY-offY;

    // calX = EMA_function(EMA_a, AcX, calX);
    // calY = EMA_function(EMA_a, AcY, calY);

    if(AcX>600 || AcX<-600){  //X acceleration
        velX += AcX;
        stableX=0;
    }else{
        stableX++;
    }
    velX = stableX>5 ? (velX/1.5) : velX;
    
    if(AcY>600 || AcY<-600){  //Y acceleration
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
    
    // Serial.print(velX); Serial.print(" ");
    // Serial.print(velY); Serial.print(" ");
    // Serial.print(posX); Serial.print(" ");
    // Serial.print(posY); Serial.print(" ");
    // Serial.println();

    Mouse.move(-velX*mouse_sen, velY*mouse_sen*1.6, 0);

    delay(4);
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

    // Serial.println("==========================================");  //Offset data
    // Serial.print(offX); Serial.print(" ");
    // Serial.print(offY); Serial.print(" ");
    // Serial.print(offZ); Serial.println(" ");
    // Serial.println("==========================================");

    RXLED0;
    TXLED0;
    for(int i=0; i<10; i++){
        TXLED1;
        RXLED0;
        delay(80);
        TXLED0;
        RXLED1;
        delay(80);
    }
    RXLED0;
    TXLED0;
}
