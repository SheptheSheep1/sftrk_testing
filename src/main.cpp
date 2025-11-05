#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>
#include <RadioLib.h>
#include <SPI.h>

// Definitions
#define LED PIN_015
#define OLED_RESET -1
#define gpsSerial Serial1
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

// Declarations Functions
void setFlag(void);
void setup();
void loop();
String bytesToAscii(const uint8_t *, size_t);

// Declarations Objects/prims
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);
uint8_t buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];
TinyGPSPlus gps;
SX1262 radio = new Module(SX126X_CS, SX126X_DIO1, SX126X_RESET, SX126X_BUSY);
char latBuffer[30]; // Buffer for latitude display
char lngBuffer[30]; // Buffer for longitude display
char dateBuffer[30]; // Buffer for date display
char recvBuffer[64]; // Buffer for LoRa recv display

double lastLat = 0;
double lastLng = 0;
TinyGPSDate lastDate;

// Modem Parameters (Meshtastic LongFast)
const float FREQ_MHZ = 906.875;
const uint8_t SF = 11;
const unsigned long BW = 250000;
const uint8_t CR = 5;
const uint16_t PREAMBLE = 16;
const uint8_t SYNCWORD = 0x2B;
volatile bool receivedFlag = false; // flag to indicate that a packet was received (IRQ)

void setFlag(void) {
  // we got a packet, set the flag
  receivedFlag = true;
}

void setup(){
	pinMode(LED, OUTPUT); //set output mode
	digitalWrite(LED, LOW);
	
	gpsSerial.begin(9600);
	//SPI.begin(); // uses variant default SPI pins

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

	// modem init
	// initialize SX1262 with default settings
	Serial.print(F("[SX1262] Initializing ... "));
	int state = radio.begin();
	if (state == RADIOLIB_ERR_NONE) {
	  Serial.println(F("success!"));
	} else {
	  Serial.print(F("failed, code "));
	  Serial.println(state);
	  while (true) { delay(10); }
	}

	int st = radio.begin();
	Serial.print("radio.begin() -> ");
	Serial.println(st);
	if (st != RADIOLIB_ERR_NONE) {
	  Serial.println("radio.begin() failed. Check wiring, power, constructor ordering.");
	  while (true) { delay(1000); }
	}
	
	// set modem parameters
	radio.setFrequency(FREQ_MHZ);
	radio.setSpreadingFactor(SF);
	radio.setBandwidth(BW);
	radio.setCodingRate(CR);
	radio.setPreambleLength(PREAMBLE);
	radio.setSyncWord(SYNCWORD);
	//radio.setCRC(2);
	radio.setPacketReceivedAction(setFlag); // IRQ func
	
	// start listening for LoRa packets
	Serial.print(F("[SX1262] Starting to listen ... "));
	state = radio.startReceive();
	if (state == RADIOLIB_ERR_NONE) {
	  Serial.println(F("success!"));
	} else {
	  Serial.print(F("failed, code "));
	  Serial.println(state);
	  while (true) { delay(10); }
	}
	Serial.println("boot successful...");
	
	delay(1000);
	display.clearDisplay();
	display.display();
}

void loop() {
	const size_t BUF_SZ = 256;
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

	if(receivedFlag) {
	  // reset flag
	  receivedFlag = false;

	  // you can read received data as an Arduino String
	  String str;
	  int state = radio.readData(str);

	  // you can also read received data as byte array
	  //byte byteArr[8];
	  //int numBytes = radio.getPacketLength();
	  //int state = radio.readData(byteArr, numBytes);

	  if (state == RADIOLIB_ERR_NONE) {
	    // packet was successfully received
	    Serial.println(F("[SX1262] Received packet!"));

	    // print data of the packet
	    Serial.print(F("[SX1262] Data:\t\t"));
	    Serial.println(str);
		char charStr[16];
		str.toCharArray(charStr, sizeof(charStr));
		snprintf(recvBuffer, sizeof(recvBuffer), "%s", &charStr);
        display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // Set color for date (also for overwr)
        display.setCursor(0, 30); // curosr pos for lora stuff
        display.print(recvBuffer);


	    // print RSSI (Received Signal Strength Indicator)
	    Serial.print(F("[SX1262] RSSI:\t\t"));
	    Serial.print(radio.getRSSI());
	    Serial.println(F(" dBm"));

	    // print SNR (Signal-to-Noise Ratio)
	    Serial.print(F("[SX1262] SNR:\t\t"));
	    Serial.print(radio.getSNR());
	    Serial.println(F(" dB"));

	    // print frequency error
	    Serial.print(F("[SX1262] Frequency error:\t"));
	    Serial.print(radio.getFrequencyError());
	    Serial.println(F(" Hz"));

	  } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
	    // packet was received, but is malformed
	    Serial.println(F("CRC error!"));

	  } else {
	    // some other error occurred
	    Serial.print(F("failed, code "));
	    Serial.println(state);

	  }
	}
}

// Convert bytes to ASCII-safe String
String bytesToAscii(const uint8_t *buf, size_t len) {
  String s;
  s.reserve(len);
  for (size_t i = 0; i < len; i++) {
    char c = (char)buf[i];
    if (c >= 32 && c <= 126) {
      s += c;            // printable ASCII
    } else {
      s += '.';          // non-printable → dot
    }
  }
  return s;
}
