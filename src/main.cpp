#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>

// Definitions
#define LED PIN_015
#define OLED_RESET -1
#define gpsSerial Serial1
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

// Declarations Functions
void setup();
void loop();

// Declarations Objects
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);
uint8_t buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];
TinyGPSPlus gps;

void setup(){
	pinMode(LED, OUTPUT); //set output mode
	digitalWrite(LED, LOW);
	
	gpsSerial.begin(9600);

	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { //if error then builtin led light up
	  digitalWrite(LED, HIGH); //Set the LED to high
	  while (true);
	}
	
	// display boot
	memset(buffer, 0, sizeof(buffer)); // set loop display buffer with zeros
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(0, 0);
	display.println("Booting or smth...");
	display.display();
	delay(2000);
	display.clearDisplay();
}
void loop(){
	while(gpsSerial.available() > 0){ // checks getting serial from gps
		gps.encode(gpsSerial.read());
	}

	if (gps.location.isUpdated() || gps.date.isUpdated()){display.clearDisplay();}

	if(gps.location.isUpdated()){
		Serial.print(F("LOCATION   Fix Age="));
		Serial.print(gps.location.age());
		Serial.print(F("ms Raw Lat="));
		Serial.print(gps.location.rawLat().negative ? "-" : "+");
		Serial.print(gps.location.rawLat().deg);
		Serial.print("[+");
		Serial.print(gps.location.rawLat().billionths);
		Serial.print(F(" billionths],  Raw Long="));
		Serial.print(gps.location.rawLng().negative ? "-" : "+");
		Serial.print(gps.location.rawLng().deg);
		Serial.print("[+");
		Serial.print(gps.location.rawLng().billionths);
		Serial.print(F(" billionths],  Lat="));
		Serial.print(gps.location.lat(), 6);
		Serial.print(F(" Long="));
		Serial.println(gps.location.lng(), 6);
		display.setCursor(0,0);
		display.printf("%3.6f,\n%3.6f", gps.location.lat(), gps.location.lng());
	}
	else if (gps.date.isUpdated())
	{
		Serial.print(F("DATE       Fix Age="));
		Serial.print(gps.date.age());
		Serial.print(F("ms Raw="));
		Serial.print(gps.date.value());
		Serial.print(F(" Year="));
		Serial.print(gps.date.year());
		Serial.print(F(" Month="));
		Serial.print(gps.date.month());
		Serial.print(F(" Day="));
		Serial.println(gps.date.day());
		display.setCursor(0,20);
		display.printf("%d ms %d/%d/%d",gps.date.age(), gps.date.month(), gps.date.day(), gps.date.year());
	}

	if (gps.location.isUpdated() || gps.date.isUpdated()){display.display();}
}
