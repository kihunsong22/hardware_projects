#include <VirtualWire.h>
const int TX_DIO_Pin = 5;
int count = 0;

void setup()
{
  vw_set_tx_pin(TX_DIO_Pin); // Initialize TX pin
  vw_setup(1200); // Transfer speed : 2000 bits per sec
}

void loop()
{
  if(count%2)
    send("Trasnmitting value1");
  else
    send("Trasnmitting value2");
    
  count+=1;
  delay(500);
}

void send (char *message)
{
  vw_send((uint8_t *)message, strlen(message));
  vw_wait_tx(); // Wait until the whole message is gone
}
