#include <SoftwareSerial.h>
#include <TinyGPS.h>

bool newdata;
uint16_t start;

SoftwareSerial GPS(D5, D4); // RX, TX
TinyGPS tinyGPS;

void tinyGPSdump(TinyGPS &tinyGPS);
void printFloat(double f, int digits = 2);

void setup(){
	Serial.begin(115200);
	GPS.begin(9600);

	ESP.wdtDisable();  // GPS.read() takes forever and triggers WDT reset on ESP
	ESP.wdtFeed();

	delay(100);
	Serial.println();
	Serial.println();
	Serial.println("uBlox Neo 6M");
	Serial.print("Testing TinyGPS library v."); Serial.println(TinyGPS::library_version());
	Serial.print("Sizeof(tinyGPSobject) = "); Serial.println(sizeof(TinyGPS));
	Serial.println();
}

void loop(){
	newdata = false;
	start = millis();

	while (millis() - start < 5000) {
		ESP.wdtFeed();  // Feed WDT on ESP chips since GPS.read triggers WDT reset
		if (GPS.available()) {
			char c = GPS.read();
			// Serial.print(c);  // uncomment to see raw GPS data
			if (tinyGPS.encode(c)) {
				newdata = true;
				// break;  // uncomment to print new data immediately!
			}
		}
	}

	if (newdata) {
		Serial.println("Acquired Data");
		Serial.println("-------------");
		tinyGPSdump(tinyGPS);
		Serial.println("-------------");
		Serial.println();
	}
 
}

void tinyGPSdump(TinyGPS &tinyGPS){
	long lat, lon;
	float flat, flon;
	unsigned long age, date, time, chars;
	int year;
	byte month, day, hour, minute, second, hundredths;
	unsigned short sentences, failed;
 
	tinyGPS.get_position(&lat, &lon, &age);
	Serial.print("Lat/Long(10^-5 deg): "); Serial.print(lat); Serial.print(", "); Serial.print(lon); 
	Serial.print(" Fix age: "); Serial.print(age); Serial.println("ms.");
 
	tinyGPS.f_get_position(&flat, &flon, &age);
	Serial.print("Lat/Long(float): "); printFloat(flat, 5); Serial.print(", "); printFloat(flon, 5);
	Serial.print(" Fix age: "); Serial.print(age); Serial.println("ms.");
 
	tinyGPS.get_datetime(&date, &time, &age);
	Serial.print("Date(ddmmyy): "); Serial.print(date); Serial.print(" Time(hhmmsscc): ");
		Serial.print(time);
	Serial.print(" Fix age: "); Serial.print(age); Serial.println("ms.");
 
	tinyGPS.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
	Serial.print("Date: "); Serial.print(static_cast<int>(month)); Serial.print("/"); 
		Serial.print(static_cast<int>(day)); Serial.print("/"); Serial.print(year);
	Serial.print("	Time: "); Serial.print(static_cast<int>(hour)); Serial.print(":"); 
		Serial.print(static_cast<int>(minute)); Serial.print(":"); Serial.print(static_cast<int>(second));
		Serial.print("."); Serial.print(static_cast<int>(hundredths));
	Serial.print("	Fix age: ");	Serial.print(age); Serial.println("ms.");
 
	Serial.print("Alt(cm): "); Serial.print(tinyGPS.altitude()); Serial.print(" Course(10^-2 deg): ");
		Serial.print(tinyGPS.course()); Serial.print(" Speed(10^-2 knots): "); Serial.println(tinyGPS.speed());
	Serial.print("Alt(float): "); printFloat(tinyGPS.f_altitude()); Serial.print(" Course(float): ");
		printFloat(tinyGPS.f_course()); Serial.println();
	Serial.print("Speed(knots): "); printFloat(tinyGPS.f_speed_knots()); Serial.print(" (mph): ");
		printFloat(tinyGPS.f_speed_mph());
	Serial.print(" (mps): "); printFloat(tinyGPS.f_speed_mps()); Serial.print(" (kmph): ");
		printFloat(tinyGPS.f_speed_kmph()); Serial.println();
 
	tinyGPS.stats(&chars, &sentences, &failed);
	Serial.print("Stats: characters: "); Serial.print(chars); Serial.print(" sentences: ");
		Serial.print(sentences); Serial.print(" failed checksum: "); Serial.println(failed);
}

void printFloat(double number, int digits){
	// Handle negative numbers
	if (number < 0.0) {
		 Serial.print('-');
		 number = -number;
	}
 
	// Round correctly so that print(1.999, 2) prints as "2.00"
	double rounding = 0.5;
	for (uint8_t i=0; i<digits; ++i)
		rounding /= 10.0;
 
	number += rounding;
 
	// Extract the integer part of the number and print it
	unsigned long int_part = (unsigned long)number;
	double remainder = number - (double)int_part;
	Serial.print(int_part);
 
	// Print the decimal point, but only if there are digits beyond
	if (digits > 0)
		Serial.print("."); 
 
	// Extract digits from the remainder one at a time
	while (digits-- > 0) {
		remainder *= 10.0;
		int toPrint = int(remainder);
		Serial.print(toPrint);
		remainder -= toPrint;
	}
}
