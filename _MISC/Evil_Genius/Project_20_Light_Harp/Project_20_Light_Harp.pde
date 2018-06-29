// Project 20 Light Harp

int pitchInputPin = 0;
int volumeInputPin = 1;
int volumeOutputPin = 6;

int dacPins[] = {2, 4, 7, 8};
int sin16[] = {7, 8, 10, 11, 12, 13, 14, 14, 15, 14, 14, 13, 12, 11, 
              10, 8, 7, 6, 4, 3, 2, 1, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6};

int count = 0;

void setup()                 
{
  for (int i = 0; i < 4; i++)
  {
    pinMode(dacPins[i], OUTPUT);
  }
  pinMode(volumeOutputPin, OUTPUT);
}

void loop()                    
{
  int pitchDelay = map(analogRead(pitchInputPin), 0, 1023, 10, 60);
  int volume = map(analogRead(volumeInputPin), 0, 1023, 10, 70);
  for (int i = 0; i < 32; i++)
  {
    setOutput(sin16[i]);
    delayMicroseconds(pitchDelay);
  }
  if (count == 10)
  {
    analogWrite(volumeOutputPin, volume);
    count = 0;
  }
  count++;
}

void setOutput(byte value)
{
    digitalWrite(dacPins[3], ((value & 8) > 0));
    digitalWrite(dacPins[2], ((value & 4) > 0));
    digitalWrite(dacPins[1], ((value & 2) > 0));
    digitalWrite(dacPins[0], ((value & 1) > 0));
}

