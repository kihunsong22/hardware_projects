// Project 16

int clockPin = 18;
int resetPin = 19; 

//int greenPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};
//int redPins[8] = {10, 11, 12, 13, 14, 15, 16, 17};

int greenPins[8] = {2, 3, 4, 5, 9, 8, 7, 6};
int redPins[8] = {10, 11, 12, 13, 17, 16, 15, 16};


int row = 0;
int col = 0;

// colors off = 0, green = 1, red = 2, orange = 3
byte pixels[8][8] = {
{1, 1, 1, 1, 1, 1, 1, 1}, 
{1, 2, 2, 2, 2, 2, 2, 1}, 
{1, 2, 3, 3, 3, 3, 2, 1}, 
{1, 2, 3, 3, 3, 3, 2, 1},
{1, 2, 3, 3, 3, 3, 2, 1}, 
{1, 2, 3, 3, 3, 3, 2, 1}, 
{1, 2, 2, 2, 2, 2, 2, 1}, 
{1, 1, 1, 1, 1, 1, 1, 1}
};

void setup() 
{ 
  pinMode(clockPin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  for (int i = 0; i < 8; i++)
  {
    pinMode(greenPins[i], OUTPUT);
    pinMode(redPins[i], OUTPUT);
  }
  Serial.begin(9600);
} 

void loop() 
{
  if (Serial.available())
  {
    char ch = Serial.read();
    if (ch == 'x')
    {
      clear();
    }
    if (ch >= 'a' and ch <= 'g')
    {
      col = 0;
      row = ch - 'a';
    }
    else if (ch >= '0' and ch <= '3')
    {
      byte pixel = ch - '0';
      pixels[row][col] = pixel;
      col++;
    }
  }
  refresh(); 
} 

void refresh()
{
  pulse(resetPin);
  delayMicroseconds(2000); 
  for (int row = 0; row < 8; row++)
  {
    for (int col = 0; col < 8; col++)
    {
      int redPixel = pixels[col][row] & 2;
      int greenPixel = pixels[col][row] & 1;
      digitalWrite(greenPins[col], greenPixel);
      digitalWrite(redPins[col], redPixel);     
    }
    pulse(clockPin);
    delayMicroseconds(1500); 
  }
}

void clear()
{
  for (int row = 0; row < 8; row++)
  {
    for (int col = 0; col < 8; col++)
    {
      pixels[row][col] = 0;
    }
  }
}


void pulse(int pin)
{
  delayMicroseconds(20);
  digitalWrite(pin, HIGH);
  delayMicroseconds(50);
  digitalWrite(pin, LOW);
  delayMicroseconds(50);
}
