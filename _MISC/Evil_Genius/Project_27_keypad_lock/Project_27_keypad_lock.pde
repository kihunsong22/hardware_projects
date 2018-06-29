// Project 27 Keypad door lock

#include <Keypad.h>
#include <EEPROM.h>

char* secretCode = "1234";
int position = 0;
boolean locked = true;

const byte rows = 4; 
const byte cols = 3; 
char keys[rows][cols] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[rows] = {2, 7, 6, 4}; 
byte colPins[cols] = {3, 1, 5}; 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

int redPin = 9;
int greenPin = 8;
int solenoidPin = 10;

void setup()                    
{
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(solenoidPin, OUTPUT);
  loadCode();
  flash();
  updateOutputs();
}

void loop()                    
{
  char key = keypad.getKey();
  if (key == '*'  && ! locked)
  {
    // unlocked and * pressed so change code
    position = 0;
    getNewCode();
    updateOutputs();
  }
  if (key == '#')
  {
    locked = true;
    position = 0;
    updateOutputs();
  }
  if (key == secretCode[position])
  {
    position ++;
  }
  if (position == 4)
  {
    locked = false;
    updateOutputs();
  }
  delay(100);
}

void updateOutputs()
{
  if (locked)
  {
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);  
    digitalWrite(solenoidPin, HIGH);
  }
  else
  {
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH); 
    digitalWrite(solenoidPin, LOW);    
  }
}

void getNewCode()
{
  flash();
  for (int i = 0; i < 4; i++ )
  {
    char key;
    key = keypad.getKey();
    while (key == 0)
    {
      key = keypad.getKey();
    }
    flash();
    secretCode[i] = key;
  }
  saveCode();
  flash();flash();
}

void loadCode()
{
  if (EEPROM.read(0) == 1)
  {
    secretCode[0] = EEPROM.read(1);
    secretCode[1] = EEPROM.read(2);
    secretCode[2] = EEPROM.read(3);
    secretCode[3] = EEPROM.read(4);
  }
}

void saveCode()
{
  EEPROM.write(1, secretCode[0]);
  EEPROM.write(2, secretCode[1]);
  EEPROM.write(3, secretCode[2]);
  EEPROM.write(4, secretCode[3]);
  EEPROM.write(0, 1);  
}

void flash()
{
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);    
    delay(500);
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);    
}
