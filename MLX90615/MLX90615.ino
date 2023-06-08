unsigned char Re_buf[11], counter = 0;
unsigned char sign = 0;
unsigned char i = 0, sum = 0;
float TO = 0, TA = 0;

void serialEvent();

void setup()
{
  pinMode(13, OUTPUT);

  Serial.begin(115200);
  delay(10);

  Serial.write(0XA5);
  Serial.write(0X45);
  Serial.write(0XEA);
}

void loop()
{
  i = 0;
  sum = 0;

  if (sign)
  {
    sign = 0;
    for (i = 0; i < 8; i++)
    {
      sum += Re_buf[i];
    }
    if (sum == Re_buf[i])
    {
      TO = (float)(Re_buf[4] << 8 | Re_buf[5]) / 100; // 출력된 값으로 온도를 계산합니다.
      Serial.print("Temp:");
      Serial.println(TO);

      if (TO > 30)
      {
        digitalWrite(13, HIGH);
      }
      else
      {
        digitalWrite(13, LOW);
      }
    }
  }
}

void serialEvent()
{
  while (Serial.available())
  {
    Re_buf[counter] = (unsigned char)Serial.read();

    if (counter == 0 && Re_buf[0] != 0x5A)
    {
      return;
    }

    counter++;
    if (counter == 9)
    {
      counter = 0;
      sign = 1;
    }
  }
}