#include <SPI.h> // Not actualy used but needed to compile
#include <avr/pgmspace.h>
#include "spectacle.h"

// Init RadioHead for transmission
#include <RH_ASK.h>
RH_ASK driver;

// Init FastLED
#include<FastLED.h>

#define NUM_LEDS_CE 3
#define DATA_PIN_CE 4
CRGB leds_CE[NUM_LEDS_CE];

#define NUM_LEDS_JB 3
#define DATA_PIN_JB 7
CRGB leds_JB[NUM_LEDS_JB];

#define NUM_LEDS_LB 3
#define DATA_PIN_LB 8
CRGB leds_LB[NUM_LEDS_LB];

#define NUM_LEDS_SA 3
#define DATA_PIN_SA 12
CRGB leds_SA[NUM_LEDS_SA];


struct DataPacket {
  int channel;
  int value;
};

void setLEDs(CRGB* leds, int numLeds, int scaledValue, CRGB color) {
  for (int i = 0; i < numLeds; i++) {
    if (i < scaledValue) {
      leds[i] = color;
    } else {
      leds[i] = CRGB::Black;
    }
  }
  int brightness = map(analogRead(A1), 0, 1023, 0, 255);
  FastLED.setBrightness(brightness);
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

  setupLeds<DATA_PIN_CE>(leds_CE, NUM_LEDS_CE);
  setupLeds<DATA_PIN_JB>(leds_JB, NUM_LEDS_JB);
  setupLeds<DATA_PIN_LB>(leds_LB, NUM_LEDS_LB);
  setupLeds<DATA_PIN_SA>(leds_SA, NUM_LEDS_SA);
}

void loop()
{

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
      analogWrite(DATA_PIN_CE, scaledValue);
      setLEDs(leds_CE, NUM_LEDS_CE, scaledValue, CRGB::Green);
      setLEDs(leds_JB, NUM_LEDS_JB, scaledValue, CRGB::Blue);
      setLEDs(leds_LB, NUM_LEDS_LB, scaledValue, CRGB::Red);
      setLEDs(leds_SA, NUM_LEDS_SA, scaledValue, CRGB::Yellow);
    }
  }
}

void displayPacket(DataPacket packet) {
  Serial.print("Received Packet - channel: ");
  Serial.print(packet.channel);
  Serial.print(", Value: ");
  Serial.println(packet.value);
}
