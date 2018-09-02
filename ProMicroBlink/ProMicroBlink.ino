int RXLED = 17; // The RX LED has a defined Arduino pin
        // The TX LED was not so lucky, we'll need to use pre-defined
        // macros (TXLED1, TXLED0) to control that.
        // (We could use the same macros for the RX LED too -- RXLED1,
        //  and RXLED0.)
 
void setup()
{
 pinMode(RXLED, OUTPUT);// Set RX LED as an output, TX LED is set as an output behind the scenes
 
 Serial.begin(115200);     //This pipes to the serial monitor
 Serial1.begin(115200);     //This is the UART, pipes to sensors attached to board
 TXLED0;
}

void loop()
{
 Serial.println("Serial_TEST");
 Serial1.println("Serial1_TEST");
 
 digitalWrite(RXLED, LOW);
//  TXLED0;             //TX LED is not tied to a normally controlled pin
 delay(250);            // wait for a second
 digitalWrite(RXLED, HIGH);    // set the LED off
//  TXLED1;
 delay(250);            // wait for a second
}