
#include "WInterrupts.h"
#include "variant.h"
#include "wiring_constants.h"
#define LED PIN_015
// TX sketch — RadioLib SX1262, Meshtastic LongFast params
// Flash this to board A

// #define RADIOLIB_DEBUG   // uncomment for verbose RadioLib debug
#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET -1
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
// Variant pin mapping (from your variant)
#define SX126X_CS    (32 + 13) // P1.13
#define SX126X_DIO1  (0 + 10)  // P0.10
#define SX126X_BUSY  (0 + 29)  // P0.29
#define SX126X_RESET (0 + 9)   // P0.09
#define SX126X_RXEN  (0 + 17)  // P0.17
#define SX126X_TXEN  RADIOLIB_NC

// Use the 4-arg Module constructor: CS, IRQ(DIO1), RESET, GPIO(BUSY)
SX1262 radio = new Module(SX126X_CS, SX126X_DIO1, SX126X_RESET, SX126X_BUSY);
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);
uint8_t buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];

char recvBuffer[64]; // Buffer for LoRa recv display
					 //
					 // === Modem params (Meshtastic LongFast) ===
					 // IMPORTANT: set FREQ_MHZ to the exact frequency from meshtastic --info
const float FREQ_MHZ = 906.875;   // <-- REPLACE with exact value from your node
const uint8_t SF = 11;
const unsigned long BW = 250000;
const uint8_t CR = 5;            // RadioLib uses 5 for CR 4/5 in many APIs
const uint16_t PREAMBLE = 16;
const uint8_t SYNCWORD = 0x2B;

volatile bool buttonEvent = false; // Flag to indicate a button event
bool lastButtonState = HIGH; // Last stable button state
const unsigned long debounceDelay = 50; // Debounce time in milliseconds
unsigned long lastDebounceTime = 0; // Timestamp of the last button state change
bool buttonPressed = false; // Current button state


void buttonInterrupt(void){
	buttonEvent = true;
}

void setup() {
	pinMode(LED, OUTPUT);
	Serial.begin(115200);
	while (!Serial && millis() < 2000);

	Serial.println();
	Serial.println("TX: RadioLib SX1262 test (Meshtastic LongFast)");
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { //if error then builtin led light up
													  //digitalWrite(LED, HIGH); //Set the LED to high
		while (true);
	}
	digitalWrite(LED, HIGH);
	delay(1000);
	digitalWrite(LED, LOW);
	// button interrupt
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	//attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonInterrupt, CHANGE);

	// display boot
	memset(buffer, 0, sizeof(buffer)); // set loop display buffer with zeros
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(0, 0);
	display.println("Booting or smth...");
	display.display();
	// Use variant default SPI pins
	SPI.begin(); // do not pass pins on this core

	int st = radio.begin();
	Serial.print("radio.begin() -> ");
	Serial.println(st);
	if (st != RADIOLIB_ERR_NONE) {
		Serial.println("radio.begin() failed. Check wiring, power, constructor ordering.");
		while (true) { delay(1000); }
	}

	// Apply modem settings
	radio.setFrequency(FREQ_MHZ);
	radio.setSpreadingFactor(SF);
	radio.setBandwidth(BW);
	radio.setCodingRate(CR);
	radio.setPreambleLength(PREAMBLE);
	radio.setSyncWord(SYNCWORD);
	//radio.setCRC(2);  // explicit setCRC(2)
	delay(50);

	Serial.println("Configured. Starting periodic TX (1s).");
	Serial.println(BUTTON_PIN);
	delay(1000);
	display.clearDisplay();
	display.display();
}

void loop() {
	static uint32_t seq = 0;
	String payload = String("meshtest-") + seq++;
	int st = radio.transmit(payload);
	if (st == RADIOLIB_ERR_NONE) {
	//if (0) {
		Serial.print("TX ok: ");
		Serial.println(payload);
		char charStr[16];
		payload.toCharArray(charStr, sizeof(charStr));
		snprintf(recvBuffer, sizeof(recvBuffer), "%s", &charStr);
		display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // Set color for date (also for overwr)
		display.setCursor(0, 30); // curosr pos for lora stuff
		display.print(recvBuffer);
		display.display();
	//} else if(0) {
	} else {
		Serial.print("TX err: ");
		Serial.println(st);
	}

	//if (buttonEvent) {
	//	// Disable interrupts to avoid re-entrant issues
	//	detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));

	//	// Debounce logic
	//	unsigned long currentTime = millis(); // Get the current time

	//	if (currentTime - lastDebounceTime > debounceDelay) {
	//		bool currentButtonState = digitalRead(BUTTON_PIN); // Read the actual button state

	//		if (currentButtonState != lastButtonState) { // Only if the state has changed
	//			lastDebounceTime = currentTime; // Update the time of the state change

	//			if (currentButtonState == LOW) {
	//				// Button pressed
	//				buttonPressed = true;
	//				Serial.println("Button Pressed!");
	//				digitalWrite(LED, HIGH);
	//			} else {
	//				// Button released
	//				Serial.println("Button Released!");
	//				digitalWrite(LED, LOW);
	//			}
	//		}

	//		lastButtonState = currentButtonState; // Update last button state
	//	}

	//	buttonEvent = false; // Reset the button event flag

	//	// Re-enable interrupts
	//	attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonInterrupt, CHANGE);
	//}
	//digitalWrite(PIN_LED1, LOW);
	//delay(1000);
}
