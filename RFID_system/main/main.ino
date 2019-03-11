#include <SPI.h>
#include <Wire.h>
#include <MFRC522.h>
#include <Adafruit_SSD1306.h>

#define RST_PIN 9
#define SS_PIN  10

MFRC522 mfrc(SS_PIN, RST_PIN);

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  pinMode(8, OUTPUT);

  Serial.begin(115200);
  SPI.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  mfrc.PCD_Init();
}

int energy = 50;
int o2 = 50;
int co2 = 50;
int h2o = 50;

bool state = 0;
bool soundCheck = false;
bool loopCheck = true;
bool warningCheck = false;
String lastuid = "";
bool check = true;
unsigned long prev_time = 0;
unsigned long prev_time2 = 0;
unsigned long prev_time3 = 0;
int buzzCnt = 0;
int buzzCnt2 = 0;
int speed = 1;

void loop() {
  if (1 <= energy && energy <= 30) {//나중에 15로 바꾸기
    soundCheck = true;
  } else {
    soundCheck = false;
    buzzCnt = 0;
  }
  if (energy <= 0) {
    gameEnd();
  }
  if (o2 <= 0 && co2 >= 100 && h2o <= 0) {
    speed = 4;
  }
  else if (o2 <= 0 && co2 >= 100) {
    speed = 3;
  }
  else if (h2o <= 0 && co2 >= 100) {
    speed = 3;
  }
  else if (o2 <= 0 && h2o <= 0) {
    speed = 3;
  }
  else if (o2 <= 0) {
    speed = 2;
  }
  else if (co2 >= 100) {
    speed = 2;
  }
  else if (h2o <= 0) {
    speed = 2;
  } else speed = 1;


  unsigned long current_time = millis();
  if (current_time - prev_time > 1000) {

    if (energy > 0) {
      energy -= speed;
    }
    //    if (o2 > 0) {
    //      o2 -= 1;
    //    }
    //    if (co2 > 0) {
    //      co2 -= 1;
    //    }
    //    if (h2o < 100) {
    //      h2o += 1;
    //    }
    prev_time = millis();
  }

  unsigned long current_time2 = millis();
  if (soundCheck == true) {
    if (current_time2 - prev_time2 > 200) {
      if (buzzCnt < 16) {
        digitalWrite(8, loopCheck);
        loopCheck = !loopCheck;
        buzzCnt++;
      }
      if (buzzCnt == 24) digitalWrite(8, LOW);
      prev_time2 = millis();
    }
  }

  unsigned long current_time3 = millis();
  if (warningCheck == true) {
    if (current_time3 - prev_time3 > 1000) {
      if (buzzCnt2 < 3) {
        digitalWrite(8, HIGH);
        buzzCnt2++;
      } else {
        warningCheck = false;
        digitalWrite(8, LOW);
      }
      prev_time3 = millis();
    }
  }

  mfrc.PICC_ReadCardSerial();
  if (( mfrc.PICC_IsNewCardPresent() || mfrc.PICC_ReadCardSerial()) && (mfrc.uid.uidByte[0] != 0)) {//쓰레기값 처리
    mfrc.PICC_ReadCardSerial();
    Serial.print("Card UID:");
    String uid = "";
    for (byte i = 0; i < 4; i++) {
      Serial.print(mfrc.uid.uidByte[i]);
      uid += mfrc.uid.uidByte[i];
    }
    Serial.println();

    if (lastuid == uid) {//이전 입력받은 데이터랑 비교
      check = false;//같으면 미행동 선언
    }
    if (check) {
      doSomething(uid);//행동
    }
    check = true;
    lastuid = uid;//이전 데이터로 옮김
    delay(50);
  }
  displayAll();
}

void doSomething(String uid) {
  buzzCnt2 = 0;
  if (uid == "16924020386") {
    o2 -= 50;
    h2o -= 50;
    warningCheck = true;
  }
  if (uid == "18924555127") {
    co2 += 50;
    warningCheck = true;
  }
  if (uid == "59701060") {
    warningCheck = true;
  }
}
void gameEnd() {
  digitalWrite(8, HIGH);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(49, 0);
  display.setTextSize(6);
  display.println("+");
  display.setCursor(19, 48);
  display.setTextSize(2);
  display.println("HOSPITAL");
  display.display();
  delay(5000);
  digitalWrite(8, LOW);
  while (1);
}
void displayAll() {
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(2, 10);
  display.println(pad(energy));
  display.setCursor(74, 10);
  display.println(pad(o2));
  display.setCursor(2, 43);
  display.println(pad(co2));
  display.setCursor(74, 43);
  display.println(pad(h2o));

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(11, 0);
  display.println(F("Energy"));
  display.setCursor(94, 0);
  display.println(F("O2"));
  display.setCursor(19, 33);
  display.println(F("Co2"));
  display.setCursor(91, 33);
  display.println(F("H2O"));
  display.display();
}
String pad(int n) {
  String final = "";
  if (n < 100) final += '0';
  if (n < 10) final += '0';
  final += n;
  return final;
}