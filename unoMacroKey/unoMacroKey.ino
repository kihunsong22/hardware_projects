/* Arduino USB Keyboard HID demo */
/* Send "hello world" to computer through USB every 5 seconds */

#define KEY_LEFT_CTRL  0x01
#define KEY_LEFT_SHIFT  0x02
#define KEY_RIGHT_CTRL  0x10
#define KEY_RIGHT_SHIFT 0x20

uint8_t buf[8] = { 0 }; /* Keyboard report buffer */

/* set end */

const int PSWW = 5;
const int PINN = 6;
const int LED = 13;

void setup() 
{
  Serial.begin(9600);
  pinMode(PINN, INPUT_PULLUP);
  pinMode(PSWW, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
}

char *psw = "password|";
char *piw = "pin|";

void loop() 
{
  if (digitalRead(PSWW) == LOW){
    char *chp = psw;
    
    buf[0] = 0;
    buf[2] = 88;
    Serial.write(buf, 8);
    buf[0] = 0;
    buf[2] = 0;
    Serial.write(buf, 8); // Release key
    delay(500);
    
    while (*chp) {      
      if ((*chp >= 'a') && (*chp <= 'z')) {
        buf[2] = *chp - 'a' + 4;
      } else if ((*chp >= '1') && (*chp <= '9')) {
        buf[2] = *chp - '1' + 30;
      } else {
        switch (*chp) {
          case '|':
            buf[2] = 0x28; /* ENTER use '|' */
            break;
          case '+':
            buf[0] = KEY_LEFT_SHIFT;  /* Caps */
            buf[2] = 0x2E;  // '='
            break;
        }
    }

    Serial.write(buf, 8); // Send keystroke
    buf[0] = 0;
    buf[2] = 0;
    Serial.write(buf, 8); // Release key
    chp++;
    }
    digitalWrite(LED, HIGH);
    delay(100);
  }


  
  if (digitalRead(PINN) == LOW){
    char *cwp = piw;

    buf[0] = 0;
    buf[2] = 88;
    Serial.write(buf, 8);
    buf[0] = 0;
    buf[2] = 0;
    Serial.write(buf, 8); // Release key
    delay(500);
    
    while (*cwp) {
      if ((*cwp >= '1') && (*cwp <= '9')) {
        buf[2] = *cwp - '1' + 30;
      } else {
        switch (*cwp) {
          case '0':
            buf[2] = 0x27;
            break;
        }
    }

    Serial.write(buf, 8); // Send keystroke
    buf[0] = 0;
    buf[2] = 0;
    Serial.write(buf, 8); // Release key
    cwp++;
    }
    digitalWrite(LED, HIGH);
    delay(100);
  }
  digitalWrite(LED, LOW);
}
