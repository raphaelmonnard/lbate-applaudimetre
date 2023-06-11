#include <SPI.h> // Not actualy used but needed to compile
#include <avr/pgmspace.h>
#include "spectacle.h"

// Init RadioHead for transmission
#include <RH_ASK.h>
RH_ASK driver;

// Init display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Init FastLED
#include<FastLED.h>

#define NUM_LEDS_CE 3
#define DATA_PIN_CE 2
CRGB leds_CE[NUM_LEDS_CE];

#define NUM_LEDS_JB 3
#define DATA_PIN_JB 3
CRGB leds_JB[NUM_LEDS_JB];

#define NUM_LEDS_LB 3
#define DATA_PIN_LB 4
CRGB leds_LB[NUM_LEDS_LB];

#define NUM_LEDS_SA 3
#define DATA_PIN_SA 5
CRGB leds_SA[NUM_LEDS_SA];


struct DataPacket {
  int channel;
  int value;
};

int mic;
int micSensitivity = 512;

int brightness = 32;

const int BTN_1 = 12;  // Pin connected to the reference button
const int BTN_2 = 13;  // Pin connected to the button
int buttonState = HIGH;    // Current state of the button
int lastButtonState = HIGH;  // Previous state of the button
int mode = 0;  // Current mode (0: brightness, 1: mic sensitivity, 2: spectacle)

void setLEDs(CRGB* leds, int numLeds, int scaledValue, CRGB color) {
  for (int i = 0; i < numLeds; i++) {
    if (i < scaledValue) {
      leds[i] = color;
    } else {
      leds[i] = CRGB::Black;
    }
  }
  FastLED.show();
}

template <uint8_t DATA_PIN>
void setupLeds(CRGB* leds, int numLeds) {
  FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, numLeds);
  pinMode(DATA_PIN, OUTPUT);
}

void setup()
{
  Serial.begin(9600);	// Debugging only
  if (!driver.init())
        Serial.println("init failed");

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(500);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);    

  setupLeds<DATA_PIN_CE>(leds_CE, NUM_LEDS_CE);
  setupLeds<DATA_PIN_JB>(leds_JB, NUM_LEDS_JB);
  setupLeds<DATA_PIN_LB>(leds_LB, NUM_LEDS_LB);
  setupLeds<DATA_PIN_SA>(leds_SA, NUM_LEDS_SA);

  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
}

void loop()
{
 // Read the button state
  buttonState = digitalRead(BTN_2);
  // Check if the button state has changed
  if (buttonState != lastButtonState) {
    // Check if the button is pressed (LOW state)
    if (buttonState == LOW) {
      // Increment the mode and wrap around if necessary
      mode = (mode + 1) % 3;
    }
  }
  
  // Perform mode-specific actions based on the current mode
  switch (mode) {
    case 0:
      // Brightness mode
      updateDisplay("Brightness");
      brightness = map(analogRead(A1), 0, 1023, 0, 255);
      FastLED.setBrightness(brightness);
      FastLED.show();
      break;
    case 1:
      // Mic sensitivity mode
      updateDisplay("Mic Sensitivity");
      break;
    case 2:
      // Spectacle mode
      updateDisplay("Spectacle");
      mic = map(analogRead(A0), 0, micSensitivity, 0, 100);
      uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
      uint8_t buflen = sizeof(buf);
      if (driver.recv(buf, &buflen)) {
        Serial.println("Message received !");
        // Message received
        DataPacket packet;
        if (buflen == sizeof(packet)) {
          memcpy(&packet, buf, sizeof(packet));
          // Process the received packet
          displayPacket(packet);

          // Get potentiometer value
          int scaledValue = packet.value;
          setLEDs(leds_CE, NUM_LEDS_CE, scaledValue, CRGB::Green);
          setLEDs(leds_JB, NUM_LEDS_JB, scaledValue, CRGB::Blue);
          setLEDs(leds_LB, NUM_LEDS_LB, scaledValue, CRGB::Red);
          setLEDs(leds_SA, NUM_LEDS_SA, scaledValue, CRGB::Yellow);
        }
      }
      break;
  }
  // Update the last button state
  lastButtonState = buttonState;
}

void updateDisplay(String text) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(text);
  display.display();  
}

void displayPacket(DataPacket packet) {
  Serial.print("Received Packet - channel: ");
  Serial.print(packet.channel);
  Serial.print(", Value: ");
  Serial.println(packet.value);
}
