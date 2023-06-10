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

#define NUM_LEDS_JB 3
#define DATA_PIN_JB 7

#define NUM_LEDS_LB 3
#define DATA_PIN_LB 8

#define NUM_LEDS_SA 3
#define DATA_PIN_SA 12

CRGB leds_CE[NUM_LEDS_CE];
CRGB leds_JB[NUM_LEDS_JB];
CRGB leds_LB[NUM_LEDS_LB];
CRGB leds_SA[NUM_LEDS_SA];

struct DataPacket {
  int channel;
  int value;
};

void setup()
{
    Serial.begin(9600);	// Debugging only
    if (!driver.init())
         Serial.println("init failed");
         
    FastLED.addLeds<WS2812, DATA_PIN_CE, RGB>(leds_CE, NUM_LEDS_CE);
    pinMode(DATA_PIN_CE, OUTPUT);

    FastLED.addLeds<WS2812, DATA_PIN_JB, RGB>(leds_JB, NUM_LEDS_JB);
    pinMode(DATA_PIN_JB, OUTPUT);

    FastLED.addLeds<WS2812, DATA_PIN_LB, RGB>(leds_LB, NUM_LEDS_LB);
    pinMode(DATA_PIN_LB, OUTPUT);

    FastLED.addLeds<WS2812, DATA_PIN_SA, RGB>(leds_SA, NUM_LEDS_SA);
    pinMode(DATA_PIN_SA, OUTPUT);
}

void loop()
{
  int brightness = map(analogRead(A1), 0, 1023, 0, 255);
  FastLED.setBrightness(brightness);

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
       
      // Turn on first n LEDs
      for (int i = 0; i < NUM_LEDS_CE; i++) {
        if (i < scaledValue) {
          leds_CE[i] = CRGB::Green;
        } else {
          leds_CE[i] = CRGB::Black;
        }
        FastLED.show();
      }

      // Turn on first n LEDs
      for (int i = 0; i < NUM_LEDS_JB; i++) {
        if (i < scaledValue) {
          leds_JB[i] = CRGB::Blue;
        } else {
          leds_JB[i] = CRGB::Black;
        }
        FastLED.show();
      }   

      // Turn on first n LEDs
      for (int i = 0; i < NUM_LEDS_LB; i++) {
        if (i < scaledValue) {
          leds_LB[i] = CRGB::Red;
        } else {
          leds_LB[i] = CRGB::Black;
        }
      FastLED.show();
      }

      // Turn on first n LEDs
      for (int i = 0; i < NUM_LEDS_SA; i++) {
        if (i < scaledValue) {
          leds_SA[i] = CRGB::Yellow;
        } else {
          leds_SA[i] = CRGB::Black;
        }
      FastLED.show();
      }   
    }
  }
  delay(10); // Delay between brightness changes (adjust as needed)
}

void displayPacket(DataPacket packet) {
  Serial.print("Received Packet - channel: ");
  Serial.print(packet.channel);
  Serial.print(", Value: ");
  Serial.println(packet.value);
}
