#include <SPI.h> // Not actualy used but needed to compile
#include <avr/pgmspace.h>
#include "spectacle.h"

// Init RadioHead for transmission
#include <RH_ASK.h>
RH_ASK driver;

// Init FastLED
#include<FastLED.h>
#define BRIGHTNESS 16 // Adjust this value to change the brightness (0-255)
#define NUM_LEDS_CE 3
#define DATA_PIN_CE 12
CRGB leds[NUM_LEDS_CE];

struct DataPacket {
  int channel;
  int value;
};

void setup()
{
    Serial.begin(9600);	// Debugging only
    if (!driver.init())
         Serial.println("init failed");
         
    FastLED.addLeds<WS2812, DATA_PIN_CE, RGB>(leds, NUM_LEDS_CE);
    FastLED.setBrightness(BRIGHTNESS);
    pinMode(DATA_PIN_CE, OUTPUT); // set pin mode to output
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
    }
  }

    //Serial.print("Mic Value:");
    // Serial.println(analogRead(A0));

    // Get potentiometer value
    int scaledValue = round(analogRead(A1)/1024.0*3.0);
    // Serial.println(scaledValue);
    analogWrite(DATA_PIN_CE, scaledValue);    
    // Turn on first n LEDs
    for (int i = 0; i < NUM_LEDS_CE; i++) {
      if (i < scaledValue) {
        leds[i] = CRGB::Green;
      } else {
        leds[i] = CRGB::Black;
      }
    }
    FastLED.show(); // update LEDs
}

void displayPacket(DataPacket packet) {
  Serial.print("Received Packet - channel: ");
  Serial.print(packet.channel);
  Serial.print(", Value: ");
  Serial.println(packet.value);
}
