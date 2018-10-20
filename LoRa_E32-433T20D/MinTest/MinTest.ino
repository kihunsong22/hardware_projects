#include <SoftwareSerial.h>
// #include "E32-TTL-100.h"

#define dev_num 1  // 1: TX, 2: RX
#define MAX_TX_SIZE 58

// Broadcast TX/RX Addr: 0xFFFF / 0x0000, channel 0x04
#define BCTX_ADDR 0xFFFF
#define BCRX_ADDR 0x0000
#define DEVICE_A_ADDR_H 0x05
#define DEVICE_A_ADDR_L 0x01
#define DEVICE_B_ADDR_H 0x05
#define DEVICE_B_ADDR_L 0x02

const uint8_t M0_PIN = 7;
const uint8_t M1_PIN = 8;
const uint8_t AUX_PIN = A0;
const uint8_t SOFT_RX = 10;
const uint8_t SOFT_TX = 11;

char RX_buf[514];
String dataStr = "";

SoftwareSerial E32(SOFT_RX, SOFT_TX);  // RX, TX

bool ReadAUX();  // read AUX logic level
int8_t WaitAUX_H();  // wait till AUX goes high and wait a few millis
void SwitchMode(uint8_t mode);  // change mode to mode
void triple_cmd(uint8_t Tcmd);  // send 3x Tcmd and check for response
void Reset_module();  // send RST
void ReceiveMsg();
int8_t SendMsg(String msg);

// switch(dev_num){
// 	case 1:
// 		break;
// 	case 2:
// 		break;
// 	default:
// 		break;
// }

//=== SETUP =========================================+
void setup(){
	Serial.begin(115200);
	E32.begin(9600);

	Serial.println("\n\n\n Initializing...");
	switch(dev_num){
		case 1:
			Serial.println("Device: 1");
			break;
		case 2:
			Serial.println("Device: 2");
			break;

		default:
			Serial.println("Device: UNDEFINED");
			break;
	}

	pinMode(M0_PIN, OUTPUT);
	pinMode(M1_PIN, OUTPUT);
	pinMode(AUX_PIN, INPUT);
	pinMode(LED_BUILTIN, OUTPUT);

	Reset_module();
	SwitchMode(0);
	WaitAUX_H();
	delay(10);

	Serial.println("Init complete");
}
//=== SETUP =========================================-

//=== LOOP ==========================================+
void loop(){
	switch(dev_num){
		case 1:
			dataStr = "test data";
			if(SendMsg(dataStr) == 0){
				blinkLED();
			}
			break;

		case 2:
			ReceiveMsg();
			blinkLED();
			break;

		default:
			Serial.println("DEVICE NUM ERROR");
			break;
	}
	
	delay(500);
}
//=== LOOP ==========================================-


void blinkLED(){
	digitalWrite(LED_BUILTIN, HIGH);
	delay(75);
	digitalWrite(LED_BUILTIN, LOW);
	delay(75);
}

//=== AUX ===========================================+
bool ReadAUX(){
	int val = analogRead(AUX_PIN);

	if(val<50){
		return LOW;
	}else{
		return HIGH;
	}
}

int8_t WaitAUX_H(){
	uint8_t cnt = 0;

	while((ReadAUX()==LOW) && (cnt++<100)){
		Serial.print(".");
		delay(100);
	}

	if(cnt>=100){
		Serial.println("  AUX-TimeOut");
		return -1;
	}
	return 0;
}
//=== AUX ===========================================-

//=== Mode Select ===================================+
void SwitchMode(uint8_t mode){
	WaitAUX_H();

	switch (mode){
		case 0:
			// Mode 0 | normal operation
			digitalWrite(M0_PIN, LOW);
			digitalWrite(M1_PIN, LOW);
			break;
		case 1:
			// Mode 1 | wake-up
			digitalWrite(M0_PIN, HIGH);
			digitalWrite(M1_PIN, LOW);
			break;
		case 2:
			// Mode 2 | power save
			digitalWrite(M0_PIN, LOW);
			digitalWrite(M1_PIN, HIGH);
			break;
		case 3:
			// Mode 3 | setting operation
			digitalWrite(M0_PIN, HIGH);
			digitalWrite(M1_PIN, HIGH);
			break;
		default:
			return;
	}

	WaitAUX_H();
	delay(10);
}
//=== Mode Select ===================================-

void triple_cmd(uint8_t Tcmd){
	uint8_t CMD[3] = {Tcmd, Tcmd, Tcmd};
	E32.write(CMD, 3);
	Serial.print("Command: ");
	Serial.print(Tcmd, HEX);
	Serial.print(Tcmd, HEX);
	Serial.print(Tcmd, HEX);
	Serial.println();
	delay(50);
}

void Reset_module(){
	triple_cmd(0xC4);  // 0xC4: reset
	WaitAUX_H();
	delay(1000);
}

void ReceiveMsg(){
	SwitchMode(0);
	WaitAUX_H();

	uint8_t idx;
	uint8_t data_len = E32.available();

	Serial.print("LoRa Received: [");
	Serial.print(String(data_len));
	Serial.println("] bytes.");

	for(idx=0;idx<data_len;idx++){
		RX_buf[idx] = E32.read();
	}
	RX_buf[data_len] = "\0";  // NULL terminate array

	// for(idx=0;idx<data_len;idx++){
	// 	Serial.print(" 0x");
	// 	Serial.print(0xFF & RX_buf, HEX);  // print as an ASCII-encoded hexadecimal
	// }

	Serial.print("data: [");
	Serial.print(RX_buf);
	Serial.println("]");
	Serial.println();

	return;
}

int8_t SendMsg(String msg){
	Serial.print("LoRa transmitting [");
	Serial.print(String(msg.length()));
	Serial.println("] bytes");
	Serial.print("data: [");
	Serial.print(msg);
	Serial.println("]");

	char text[60];
	if(msg.length()>=58){
		msg.toCharArray(text, 58);
	}else{
		msg.toCharArray(text, msg.length());
	}

	SwitchMode(1);
	WaitAUX_H();

	E32.write(text);
	WaitAUX_H();
	delay(10);
	
	SwitchMode(0);
	WaitAUX_H();

	return 0;
}
