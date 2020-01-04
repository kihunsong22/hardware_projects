#include <I2S.h>

// I2S mic reading for SAMD21 based boards
// WS connected to pin 0 (Zero) or pin 3 (MKR1000, MKRZero)
// CLK connected to pin 1 (Zero) or pin 2 (MKR1000, MKRZero)
// SD connected to pin 9 (Zero) or pin A6 (MKR1000, MKRZero)

void setup() {
  Serial.begin(115200);
  while (!Serial) {} // wait for serial port to connect. Needed for native USB port only

  // start I2S at 16 kHz with 32-bits per sample
  if (!I2S.begin(I2S_PHILIPS_MODE, 16000, 32)) {
    Serial.println("Failed to initialize I2S!");
    while (1); // do nothing
  }else{
		Serial.println("I2S initialized!");
	}
}

void loop() {
  int sample = I2S.read();

  if ((sample == 0) || (sample == -1) ) {
    return;
  }
  // convert to 18 bit signed
  sample >>= 14;

  // if it's non-zero print value to serial
  Serial.println(sample);
}