#include "Module.h"
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
	//lora
#define NSS 10
#define DIO1 2
#define NRST 3
#define	BUSY 9

// Declarations Functions
void setup();
void loop();

// Declarations Objects
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);
uint8_t buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];
TinyGPSPlus gps;
SX1262 radio = new Module(NSS, DIO1, NRST, BUSY);

// save transmission states between loops
int transmissionState = RADIOLIB_ERR_NONE;

// flag to indicate transmission or reception state
bool transmitFlag = false;

// flag to indicate that a packet was sent or received
volatile bool operationDone = false;

// this function is called when a complete packet
// is transmitted or received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void setFlag(void) {
  // we sent or received a packet, set the flag
  operationDone = true;
}

void setup(){
	Serial.print(F("[SX1262] Initializing ... "));
	int state = radio.begin();
	if (state == RADIOLIB_ERR_NONE) {
		Serial.println(F("success!"));
	} else {
		Serial.print(F("failed, code "));
		Serial.println(state);
		while (true) { delay(10); }
	}
	// set the function that will be called
	// when new packet is received
	radio.setDio1Action(setFlag);

#if defined(INITIATING_NODE)
	// send the first packet on this node
	Serial.print(F("[SX1262] Sending first packet ... "));
	transmissionState = radio.startTransmit("Hello World!");
	transmitFlag = true;
#else
	// start listening for LoRa packets on this node
	Serial.print(F("[SX1262] Starting to listen ... "));
	state = radio.startReceive();
	if (state == RADIOLIB_ERR_NONE) {
		Serial.println(F("success!"));
	} else {
		Serial.print(F("failed, code "));
		Serial.println(state);
		while (true) { delay(10); }
	}
#endif

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

	if(operationDone) {
		// reset flag
		operationDone = false;

		if(transmitFlag) {
			// the previous operation was transmission, listen for response
			// print the result
			if (transmissionState == RADIOLIB_ERR_NONE) {
				// packet was successfully sent
				Serial.println(F("transmission finished!"));

			} else {
				Serial.print(F("failed, code "));
				Serial.println(transmissionState);

			}

			// listen for response
			radio.startReceive();
			transmitFlag = false;

		} else {
			// the previous operation was reception
			// print data and send another packet
			String str;
			int state = radio.readData(str);

			if (state == RADIOLIB_ERR_NONE) {
				// packet was successfully received
				Serial.println(F("[SX1262] Received packet!"));

				// print data of the packet
				Serial.print(F("[SX1262] Data:\t\t"));
				Serial.println(str);
				display.setCursor(0, 100);
				display.printf("Received: %s", str);

				// print RSSI (Received Signal Strength Indicator)
				Serial.print(F("[SX1262] RSSI:\t\t"));
				Serial.print(radio.getRSSI());
				Serial.println(F(" dBm"));

				// print SNR (Signal-to-Noise Ratio)
				Serial.print(F("[SX1262] SNR:\t\t"));
				Serial.print(radio.getSNR());
				Serial.println(F(" dB"));

			}

			// wait a second before transmitting again
			delay(1000);

			// send another one
			Serial.print(F("[SX1262] Sending another packet ... "));
			transmissionState = radio.startTransmit("Hello World!");
			transmitFlag = true;
			display.display();
		}

	}

	if (gps.location.isUpdated() || gps.date.isUpdated()){display.display();}
}
