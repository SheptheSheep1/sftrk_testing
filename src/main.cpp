#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>
#include <RadioLib.h>

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
char latBuffer[30]; // Buffer for latitude display
char lngBuffer[30]; // Buffer for longitude display
char dateBuffer[30]; // Buffer for date display

double lastLat = 0;
double lastLng = 0;
TinyGPSDate lastDate;

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

void loop() {
    // Read and decode GPS data
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }

    bool locationUpdated = gps.location.isUpdated();
    bool dateUpdated = gps.date.isUpdated();
 
    // Update latitude and longitude if changed
    if (locationUpdated) {
        double lat = gps.location.lat();
        double lng = gps.location.lng();
        bool redraw = false;

        // Only update display if lat or lng has changed
        if (lat != lastLat) {
            lastLat = lat;
            snprintf(latBuffer, sizeof(latBuffer), "Lat: %3.6f", lat);
            redraw = true;
        }

        if (lng != lastLng) {
            lastLng = lng;
            snprintf(lngBuffer, sizeof(lngBuffer), "Lng: %3.6f", lng);
            redraw = true;
        }

        // if loc data changed, redraw
        if (redraw) {
            display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // White text on black background (for overwr)
            display.setCursor(0, 0); // cursor pos for lat
            display.print(latBuffer); // Print new latitude
            
            display.setCursor(0, 10); // cursor pos for lng
            display.print(lngBuffer); // Print new longitude

            display.display();
        }
    }

    if (dateUpdated) {
        TinyGPSDate date = gps.date;

        if (date.isValid() && date.value() != lastDate.value()) {
            lastDate = date;
            snprintf(dateBuffer, sizeof(dateBuffer), "Date: %02d-%02d-%02d", date.year(), date.month(), date.day());

            display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // Set color for date (also for overwr)
            display.setCursor(0, 20); // curosr pos for date
            display.print(dateBuffer);

            display.display();
        }
    }
}
