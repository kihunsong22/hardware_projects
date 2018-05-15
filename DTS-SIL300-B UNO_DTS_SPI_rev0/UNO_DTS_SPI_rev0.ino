// 1. ChipSelectPin : 10           (SCE)
// 2. MOSI(Master Output) : 11     (SDO)
// 3. MISO(Master Input) : 12      (SDI)
// 4. SCK : 13                     (SCK)

#include<SPI.h>
#define TARGET_CMD	0xA0			// 대상 온도 커맨드
#define SENSOR_CMD	0xA1			// 센서 온도 커맨드

const int chipSelectPin  = 10;
unsigned char T_high_byte;
unsigned char T_low_byte;
int  iTARGET, iSENSOR;	// 부호 2byte 온도 저장 변수


int SEND_COMMAND(unsigned char cCMD);

void setup(){
  Serial.begin(115200);

  pinMode(MISO, INPUT);
  pinMode(chipSelectPin , OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);

  digitalWrite(chipSelectPin , HIGH);    // CS High Level
  SPI.setDataMode(SPI_MODE3);            // Setting SPI Mode
  SPI.setClockDivider(SPI_CLOCK_DIV16);  // 16MHz/16 = 1MHz
  SPI.setBitOrder(MSBFIRST);             // MSB First
  SPI.begin();                           // Initialize SPI
  delay(500);                             // wating for DTS setup time
}

void loop() {
  iTARGET = SEND_COMMAND(TARGET_CMD);			// 대상 온도 Read
  delay(50);					        // 50ms : 이 라인을 지우지 마세요

  iSENSOR = SEND_COMMAND(SENSOR_CMD);			// 센서 온도 Read
  delay(500);						// 500ms : 이 라인을 지우지 마세요.
  Serial.print("Target Temp : ");
  Serial.print(float(iTARGET)/100);
  Serial.print("     Ambient Temp : ");
  Serial.println(float(iSENSOR)/100);
}

int SEND_COMMAND(unsigned char cCMD){
    digitalWrite(chipSelectPin , LOW);  // CS Low Level
    delayMicroseconds(10);              // delay(10us)
    SPI.transfer(cCMD);                // Send 1st Byte
    delay(10);                          // delay(10ms)
    T_low_byte = SPI.transfer(0x22);   // Send 2nd Byte
    delay(10);                          //delay(10ms)
    T_high_byte = SPI.transfer(0x22);  // Send 3rd Byte
    digitalWrite(chipSelectPin , HIGH); // CS High Level

    return (T_high_byte<<8 | T_low_byte);	// 상위, 하위 바이트 연산
}
