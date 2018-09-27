//#include <math.h>
#include <avr/pgmspace.h>

#include <SPI.h>
#include <Wire.h>
//#include <SoftwareSerial.h>

#include "ASCFont.h"
#include "KSFont.h"
//#include "bitmap.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//SoftwareSerial BTSerial(0, 1);

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

int XPOS = 0;
int YPOS = 0;

int buffPos;
String inputString;

boolean stringComplete = false;

uint32_t RedImage[16] = {0, }; 
uint32_t GrnImage[16] = {0, };
byte HANFontImage[32] = {0, };
byte RowAddress = 0;

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup()   {                
  //BTSerial.begin(9600);
  Serial.begin(9600);
  Serial1.begin(9600);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  //display.display();
  //delay(200);
  display.clearDisplay();
  //display.drawBitmap(2, 16, IMG_logo_128x64, 120, 44, 1); 
  //display.display();
  //delay(2000);
  //display.clearDisplay(); 
//inputString="회의시간이 종료되었습니다. "   ;
inputString="알림서비스  " ;
//ED 9A 8C EC 9D 98 EC 8B 9C EA B0 84 EC 9D B4 
//EB A3 8C 
//EB 90 98 EC 97 88 EC 8A B5 8B 88 EB 8B 88 EB 8B A4 2E 20 0D
//0A


//Serial.println(utf8ascii(inputString) );

/*Serial.print(inputString[0],DEC);Serial.print(" ");
Serial.print(inputString[1],DEC);Serial.print(" ");
Serial.print(inputString[2],DEC);Serial.print(" ");
Serial.print(inputString[3],DEC);Serial.print(" ");
Serial.print(inputString[4],DEC);Serial.print(" ");
Serial.print(inputString[5],DEC);Serial.print(" ");
*/

/*Serial.println("UTF8-decoder Test");
  Serial.print("Original: ");
  Serial.println(inputString);
  utf8ascii(inputString);
  Serial.print("Extended ASCII-Version ");
  Serial.println(inputString);
*/

stringComplete = true;
}

long newTime;
long oldTime=0;//millis();
int aliveCnt=0;

void loop() {  
  newTime=millis();
  if( (newTime - oldTime) > 4000 )
  {
    Serial1.print("블루투스신호 :");
    Serial1.println(aliveCnt++);// Serial1.print(" ");
    oldTime = newTime;
  }
  while(Serial1.available()){
    buffPos++;
    char data = (char)Serial1.read();
    //Serial.write(data);
    if(data == '/'){
      stringComplete = true;
      Serial1.println();
    }else{
      inputString += data;
  
    }
  }  
  if(stringComplete == true){
    display.clearDisplay();
    stringComplete =false;
    //Serial.println(inputString);
    char paramChar[inputString.length()+1];
    inputString.toCharArray(paramChar,inputString.length()+1);

    matrixPrint(XPOS,YPOS,paramChar);
    display.display();
    buffPos = 0;
    inputString = "";
   
  }
}

void matrixPrint(int XPOS,int YPOS,char *pChar){
  byte rg = 3;   //<b1> red, <b0> green
  byte *pFs;
  byte i, b;
  byte c, c2, c3;

  while(*pChar){ 
    c = *(byte*)pChar++;

    //---------- 한글 ---------
    if(c >= 0x80){
      c2 = *(byte*)pChar++;
      c3 = *(byte*)pChar++;
      //Serial.print(c,DEC);
      //Serial.print(c2,DEC);
      //Serial.print(c3,DEC);      
      pFs = getHAN_font(c, c2, c3);
      display.drawBitmap(XPOS, YPOS,  pFs, 16, 16, 1);
      XPOS = XPOS+16;
      if(XPOS > 128){
        XPOS = 0;
        YPOS = YPOS+16;
      }
    }
    //---------- ASCII ---------
    else{
      pFs = (byte*)ASCfontSet + ((byte)c - 0x20) * 16;
      display.drawBitmap(XPOS, YPOS,  pFs, 8, 16, 1);
      XPOS = XPOS+8;
      if(XPOS > 128){
        XPOS = 0;
        YPOS = YPOS+16;
      }   
    }   
  }  
}  


/*=============================================================================
      한글 font 처리부분 
  =============================================================================*/

byte *getHAN_font(byte HAN1, byte HAN2, byte HAN3){

  const byte cho[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 1, 2, 4, 4, 4, 2, 1, 3, 0 };
  const byte cho2[] = { 0, 5, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 6, 6, 7, 7, 7, 6, 6, 7, 5 };
  const byte jong[] = { 0, 0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1 };

  uint16_t utf16;
  byte first, mid, last;
  byte firstType, midType, lastType;
  byte i;
  byte *pB, *pF;

  /*------------------------------
    UTF-8 을 UTF-16으로 변환한다.

    UTF-8 1110xxxx 10xxxxxx 10xxxxxx
  */------------------------------
  utf16 = (HAN1 & 0x0f) << 12 | (HAN2 & 0x3f) << 6 | HAN3 & 0x3f;

  /*------------------------------
    초,중,종성 코드를 분리해 낸다.

    unicode = {[(초성 * 21) + 중성] * 28}+ 종성 + 0xAC00

          0   1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 
    초성 ㄱ   ㄲ ㄴ ㄷ ㄸ ㄹ ㅁ ㅂ ㅃ ㅅ ㅆ ㅇ ㅈ ㅉ ㅊ ㅋ ㅌ ㅍ ㅎ
    중성 ㅏ   ㅐ ㅑ ㅒ ㅓ ㅔ ㅕ ㅖ ㅗ ㅘ ㅙ ㅚ ㅛ ㅜ ㅝ ㅞ ㅟ ㅠ ㅡ ㅢ ㅣ  
    종성 없음 ㄱ ㄲ ㄳ ㄴ ㄵ ㄶ ㄷ ㄹ ㄺ ㄻ ㄼ ㄽ ㄾ ㄿ ㅀ ㅁ ㅂ ㅄ ㅅ ㅆ ㅇ ㅈ ㅊ ㅋ ㅌ ㅍ ㅎ
  ------------------------------*/
  utf16 -= 0xac00;
  last = utf16 % 28;
  utf16 /= 28;
  mid = utf16 % 21;
  first = utf16 / 21;

  first++;
  mid++;

  /*------------------------------
    초,중,종성 해당 폰트 타입(벌)을 결정한다.
  ------------------------------*/

  /*
   초성 19자:ㄱㄲㄴㄷㄸㄹㅁㅂㅃㅅㅆㅇㅈㅉㅊㅋㅌㅍㅎ
   중성 21자:ㅏㅐㅑㅒㅓㅔㅕㅖㅗㅘㅙㅚㅛㅜㅝㅞㅟㅠㅡㅢㅣ
   종성 27자:ㄱㄲㄳㄴㄵㄶㄷㄹㄺㄻㄼㄽㄾㄿㅀㅁㅂㅄㅆㅇㅈㅊㅋㅌㅍㅎ

   초성
      초성 1벌 : 받침없는 'ㅏㅐㅑㅒㅓㅔㅕㅖㅣ' 와 결합
      초성 2벌 : 받침없는 'ㅗㅛㅡ'
      초성 3벌 : 받침없는 'ㅜㅠ'
      초성 4벌 : 받침없는 'ㅘㅙㅚㅢ'
      초성 5벌 : 받침없는 'ㅝㅞㅟ'
      초성 6벌 : 받침있는 'ㅏㅐㅑㅒㅓㅔㅕㅖㅣ' 와 결합
      초성 7벌 : 받침있는 'ㅗㅛㅜㅠㅡ'
      초성 8벌 : 받침있는 'ㅘㅙㅚㅢㅝㅞㅟ'

   중성
      중성 1벌 : 받침없는 'ㄱㅋ' 와 결합
      중성 2벌 : 받침없는 'ㄱㅋ' 이외의 자음
      중성 3벌 : 받침있는 'ㄱㅋ' 와 결합
      중성 4벌 : 받침있는 'ㄱㅋ' 이외의 자음

   종성
      종성 1벌 : 중성 'ㅏㅑㅘ' 와 결합
      종성 2벌 : 중성 'ㅓㅕㅚㅝㅟㅢㅣ'
      종성 3벌 : 중성 'ㅐㅒㅔㅖㅙㅞ'
      종성 4벌 : 중성 'ㅗㅛㅜㅠㅡ'

  */
  if(!last){  //받침 없는 경우
    firstType = cho[mid];
    if(first == 1 || first == 24) midType = 0;
    else midType = 1;
  }
  else{       //받침 있는 경우
    firstType = cho2[mid];
    if(first == 1 || first == 24) midType = 2;
    else midType = 3;
    lastType = jong[mid];
  }
  memset(HANFontImage, 0, 32);

  //초성 
  pB = HANFontImage;
  pF = (byte*)KSFont + (firstType*20 + first)*32;
  i = 32;
  while(i--) *pB++ = pgm_read_byte(pF++); 

  //중성
  pB = HANFontImage;
  pF = (byte*)KSFont + (8*20 + midType*22 + mid)*32;
  i = 32;
  while(i--) *pB++ |= pgm_read_byte(pF++); 

  //종성
  if(last){
    pB = HANFontImage;
    pF = (byte*)KSFont + (8*20 + 4*22 + lastType*28 + last)*32;
    i = 32;
    while(i--) *pB++ |= pgm_read_byte(pF++); 
  }

  return HANFontImage;
}


static byte c1;  // Last character buffer
byte utf8ascii(byte ascii) {
    if ( ascii<128 )   // Standard ASCII-set 0..0x7F handling  
    {   c1=0;
        return( ascii );
    }

    // get previous input
    byte last = c1;   // get last char
    c1=ascii;         // remember actual character

    switch (last)     // conversion depnding on first UTF8-character
    {   case 0xC2: return  (ascii);  break;
        case 0xC3: return  (ascii | 0xC0);  break;
        case 0x82: if(ascii==0xAC) return(0x80);       // special case Euro-symbol
    }

    return  (0);                                     // otherwise: return zero, if character has to be ignored
}

String utf8ascii(String s)
{      
        String r="";
        char c;
        for (int i=0; i<s.length(); i++)
        {
                c = utf8ascii(s.charAt(i));
                if (c!=0) r+=c;
        }
        return r;
}
void utf8ascii(char* s)
{      
        int k=0;
        char c;
        for (int i=0; i<strlen(s); i++)
        {
                c = utf8ascii(s[i]);
                if (c!=0)
                        s[k++]=c;
        }
        s[k]=0;
}
/*=============================================================================

  =============================================================================*/
