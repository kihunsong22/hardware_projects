// Project 19 Tune Player

int dacPins[] = {2, 4, 7, 8};
int sin16[] = {7, 8, 10, 11, 12, 13, 14, 14, 15, 14, 14, 13, 12, 11, 
              10, 8, 7, 6, 4, 3, 2, 1, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6};

int lowToneDurations[] = {120, 105, 98, 89, 78, 74, 62};    
//                         A    B   C   D    E  F   G           
int highToneDurations[] = { 54, 45, 42, 36, 28, 26, 22 };
//                           a   b   c   d   e   f   g

// Scale
//char* song = "A B C D E F G a b c d e f g";

// Jingle Bells
//char* song = "E E EE E E EE E G C D EEEE F F F F F E E E E D D E DD GG E E EE E E EE E G C D EEEE F F F F F E E E G G F D CCCC";

// Jingle Bells - Higher
char* song = "e e ee e e ee e g c d eeee f f f f f e e e e d d e dd gg e e ee e e ee e g c d eeee f f f f f e e e g g f d cccc";


void setup()                 
{
  for (int i = 0; i < 4; i++)
  {
    pinMode(dacPins[i], OUTPUT);
  }
}

void loop()                    
{
  int i = 0;
  char ch = song[0];
  while (ch != 0)
  {
    if (ch == ' ')
    {
      delay(75);
    }
    else if (ch >= 'A' and ch <= 'G')
    {
      playNote(lowToneDurations[ch - 'A']);
    }
    else if (ch >= 'a' and ch <= 'g')
    {
      playNote(highToneDurations[ch - 'a']);
    }
    i++;
    ch = song[i]; 
  }
  
  delay(5000);
}

void setOutput(byte value)
{
    digitalWrite(dacPins[3], ((value & 8) > 0));
    digitalWrite(dacPins[2], ((value & 4) > 0));
    digitalWrite(dacPins[1], ((value & 2) > 0));
    digitalWrite(dacPins[0], ((value & 1) > 0));
}

void playNote(int pitchDelay)
{
  long numCycles = 5000 / pitchDelay + (pitchDelay / 4);
  for (int c = 0; c < numCycles; c++)
  {
    for (int i = 0; i < 32; i++)
    {
      setOutput(sin16[i]);
      delayMicroseconds(pitchDelay);
    }
  }
}
