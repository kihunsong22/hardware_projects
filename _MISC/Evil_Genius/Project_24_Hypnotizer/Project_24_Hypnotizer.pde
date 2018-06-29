// Project 24 - Hypnotizer

int t1Pin = 5;
int t3Pin = 6;

int speeds[] = {20, 40, 80, 120, 160, 180, 160, 120, 80, 40, 20, 
                -20, -40, -80, -120, -160, -180, -160, -120, -80, -40, -20};
int i = 0;

void setup()                   
{
  pinMode(t1Pin, OUTPUT);
  digitalWrite(t1Pin, LOW);
  pinMode(t3Pin, OUTPUT);
  digitalWrite(t3Pin, LOW);
}

void loop()                     
{
  int speed = speeds[i];
  i++;
  if (i == 22)
  {
    i = 0;
  }
  drive(speed);
  delay(1500);
}

void allOff()
{
  digitalWrite(t1Pin, LOW);
  digitalWrite(t3Pin, LOW);
  delay(1);
}

void drive(int speed)
{
  allOff();
  if (speed > 0)
  {
    analogWrite(t1Pin, speed);    
  } 
  else if (speed < 0)
  {
    analogWrite(t3Pin,  -speed);
  }
}
