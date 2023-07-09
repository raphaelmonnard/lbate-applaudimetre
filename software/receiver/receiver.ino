#include <SPI.h> // Not actually used but needed to compile
#include <avr/pgmspace.h>
#include "spectacle.h"
#include <RH_ASK.h>
#include <FastLED.h>

// #include "SSD1306Ascii.h"
// #include "SSD1306AsciiAvrI2c.h"

// // 0X3C+SA0 - 0x3C or 0x3D
// #define I2C_ADDRESS 0x3C

// // Define proper RST_PIN if required.
// #define RST_PIN -1

// SSD1306AsciiAvrI2c oled;

#define NUM_LEDS_CE 33
#define DATA_PIN_CE 7
#define COLOR_CE 0x0EA500
CRGB leds_CE[NUM_LEDS_CE];

#define NUM_LEDS_JB 28
#define DATA_PIN_JB 6
#define COLOR_JB 0x0045E5
CRGB leds_JB[NUM_LEDS_JB];

#define NUM_LEDS_LB 28
#define DATA_PIN_LB 5
#define COLOR_LB 0xD200AB
CRGB leds_LB[NUM_LEDS_LB];

#define NUM_LEDS_SA 34
#define DATA_PIN_SA 4
#define COLOR_SA 0xE5DD00
CRGB leds_SA[NUM_LEDS_SA];

struct DataPacket {
  int channel = 0;
  int ledHeightPotValue = 0;
  int micSensitivity = 0;
};

// Adafruit_SSD1306 display(128, 32, &Wire, -1);
RH_ASK driver;
DataPacket packet;
int memoryFlag = 0;
int memoryMillis = millis();
int brightness = 0;
int channel = 0;
int ledHeightPotValue = 100;
int micSensitivity = 1023;
int micLowerThreshold = 90; 

const int attackTime = 100;     // Attack time in milliseconds
const int decayTime = 1800;      // Decay time in milliseconds
const int sustainLevel = 0;    // Sustain level
const int releaseTime = 2000;    // Release time in milliseconds

unsigned long startTime;         // Time when the note starts
int initialValue;
int envelopeValue = 0;           // Current value of the envelope signal$

int B;

// void updateDisplay(const String& text) {
//   display.clearDisplay();
//   display.setCursor(0, 0);
//   display.println(text);
//   display.display();
// }

void setLEDs(CRGB* leds, int numLeds, int scaledValue, CRGB color) {
  for (int i = 0; i < numLeds; i++) {
    leds[i] = (i < scaledValue) ? color : CRGB::Black;
  }
}

template <uint8_t DATA_PIN, CRGB* leds, int numLeds>
void setupLeds() {
  FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, numLeds);
  pinMode(DATA_PIN, OUTPUT);
}

void randomLEDs(CRGB* leds, int numLeds) {
  if (random(4)==1){
    for (int i = 0; i < numLeds; i++){
      leds[i] = CRGB::Black;
    }
  }
  
  int randomIndex = random(numLeds);
  
  // Generate random RGB values for the color components
  byte randomRed = random(256);
  byte randomGreen = random(256);
  byte randomBlue = random(256);

  // Assign the random color to the selected LED
  leds[randomIndex] = CRGB(randomRed, randomGreen, randomBlue);
}

void upLEDs(CRGB* leds, int numLeds, CRGB color){
    for (int i = 0; i < numLeds; i++){
      leds[i] = color;
      FastLED.show();
      delay(20);
  }
}

void blinkLEDs(){
  for (int i = 0; i<3; i++ ) {
    FastLED.setBrightness(0);
    FastLED.show();
    delay(200);
    FastLED.setBrightness(brightness);
    FastLED.show();
    delay(200);
  }
}

void setup() {
  Serial.begin(9600);
  if (!driver.init())
    Serial.println("init failed");

  // #if RST_PIN >= 0
  //   oled.begin(&Adafruit128x32, I2C_ADDRESS, RST_PIN);
  // #else // RST_PIN >= 0
  //   oled.begin(&Adafruit128x32, I2C_ADDRESS);
  // #endif // RST_PIN >= 0
  //   // Call oled.setI2cClock(frequency) to change from the default frequency.

  // oled.setFont(Adafruit5x7);

  // oled.clear();

  // // first row
  // oled.println("set1X test");

  // if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  //   Serial.println(F("SSD1306 allocation failed"));
  //   for (;;) {}
  // }
  // display.display();
  // delay(500);
  // display.setTextSize(1);
  // display.setTextColor(SSD1306_WHITE);

  setupLeds<DATA_PIN_CE, leds_CE, NUM_LEDS_CE>();
  setupLeds<DATA_PIN_JB, leds_JB, NUM_LEDS_JB>();
  setupLeds<DATA_PIN_LB, leds_LB, NUM_LEDS_LB>();
  setupLeds<DATA_PIN_SA, leds_SA, NUM_LEDS_SA>(); 
}

void loop() {
  // updateDisplay("Spectacle");
  // Serial.println("start loop");

  // Adjust brightness of stripe LED
  if (memoryFlag == 3 || memoryFlag == 6 ) {
    brightness = 0;
  } else {
    brightness = map(analogRead(A1), 0, 1023, 0, 255);
  }
  FastLED.setBrightness(brightness);

  // update brightness for the red LED diodes, 
  // output PIN 3 (required to avoid conflict wiht RadioHead)
  // Analog sensor A2
  if (memoryFlag == 6 || memoryFlag == 7){
    analogWrite(3, 0);
  }
  else{
    analogWrite(3, map(analogRead(A2), 0, 1023, 0, 255));
  }

  if (driver.available()) {
    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t buflen = sizeof(buf);
    if (driver.recv(buf, &buflen) && buflen == sizeof(packet)) {
      memcpy(&packet, buf, sizeof(packet));
      ledHeightPotValue = packet.ledHeightPotValue;
      channel = packet.channel;
      micSensitivity = packet.micSensitivity;
    }
    FastLED.show();
  }

  int flag = pgm_read_word(&(params[channel][6]));
  // auto control LEDs withou ledHeightPotValue
  if (flag == 5){  
    ledHeightPotValue = 100;
  }

  // Test intensité LED  
  if (flag == 1) {
    setLEDs(leds_CE, NUM_LEDS_CE, 100, COLOR_CE);
    setLEDs(leds_JB, NUM_LEDS_JB, 100, COLOR_JB);
    setLEDs(leds_LB, NUM_LEDS_LB, 100, COLOR_LB);
    setLEDs(leds_SA, NUM_LEDS_SA, 100, COLOR_SA);  
  }
  
  // Introduction - random LEDs  
  else if (flag == 2) {
    randomLEDs(leds_CE, NUM_LEDS_CE);
    randomLEDs(leds_JB, NUM_LEDS_JB);
    randomLEDs(leds_LB, NUM_LEDS_LB);
    randomLEDs(leds_SA, NUM_LEDS_SA);
    delay(random(50, 200));
  }

  // Explosion, les lumières de l’applaudimètre s’allument.
  else if (flag == 3) {
    if (memoryFlag != 3) {
      FastLED.clear();
      FastLED.show();
      upLEDs(leds_CE, NUM_LEDS_CE, COLOR_CE);
      upLEDs(leds_JB, NUM_LEDS_JB, COLOR_JB);  
      upLEDs(leds_LB, NUM_LEDS_LB, COLOR_LB);  
      upLEDs(leds_SA, NUM_LEDS_SA, COLOR_SA);
      blinkLEDs();
    }
  }

  // Coup de téléphone - clignoter ou "Et en plus, l’applaudimètre fonctionne... c’est super !"
  else if (flag == 4) {
    setLEDs(leds_CE, NUM_LEDS_CE, 100, COLOR_CE);
    setLEDs(leds_JB, NUM_LEDS_JB, 100, COLOR_JB);
    setLEDs(leds_LB, NUM_LEDS_LB, 100, COLOR_LB);
    setLEDs(leds_SA, NUM_LEDS_SA, 100, COLOR_SA);
    blinkLEDs();
  }

  else if (flag == 6){
    if (memoryFlag != 6) {
      setLEDs(leds_CE, NUM_LEDS_CE, 100, COLOR_CE);
      setLEDs(leds_JB, NUM_LEDS_JB, 100, COLOR_JB);
      setLEDs(leds_LB, NUM_LEDS_LB, 100, COLOR_LB);
      setLEDs(leds_SA, NUM_LEDS_SA, 100, COLOR_SA);  
      blinkLEDs();

      setLEDs(leds_CE, NUM_LEDS_CE, 100, COLOR_CE);
      setLEDs(leds_JB, NUM_LEDS_JB, 100, COLOR_JB);
      setLEDs(leds_LB, NUM_LEDS_LB, 100, COLOR_LB);
      setLEDs(leds_SA, NUM_LEDS_SA, 100, COLOR_SA); 
      
      for (int i=255; i>=0; i=i-2){
        FastLED.setBrightness(i);
        FastLED.show();
        analogWrite(3, i);     
      }

      FastLED.setBrightness(255);

      for (int i=0; i<10; i++){
        randomLEDs(leds_CE, NUM_LEDS_CE);
        randomLEDs(leds_JB, NUM_LEDS_JB);
        randomLEDs(leds_LB, NUM_LEDS_LB);
        randomLEDs(leds_SA, NUM_LEDS_SA);
        FastLED.show();        
        analogWrite(3, random(255));
        delay(random(500));
        analogWrite(3, 0);
        delay(random(500));
      }
    }            
  }

  else{
    int weight_CE = pgm_read_word(&(params[channel][0]));
    int weight_JB = pgm_read_word(&(params[channel][1]));
    int weight_LB = pgm_read_word(&(params[channel][2]));
    int weight_SA = pgm_read_word(&(params[channel][3]));
    int weight = pgm_read_word(&(params[channel][4]));
    int max = pgm_read_word(&(params[channel][5]));

    // compute factor for potentiometer value (0-100)
    int A = weight * ledHeightPotValue / 100; 

    // compute factor for mic (0-100)
    int micValue = analogRead(A0);

    //ensure lower limit: 
    if (micValue < micLowerThreshold) {
      micValue = micLowerThreshold;
    }

    //ensure higher limit: 
    if (micValue > micSensitivity) {
      micValue = micSensitivity;
    }

    int mic = map(micValue, micLowerThreshold, micSensitivity, 0, 100); // scaled value of mic with micSensitivity
    
    if (mic>0) {
      // Start the note and reset the envelope
      startTime = millis();
      // update initial value only if mic signal is higher
      if (mic > initialValue) {
        initialValue = mic;
      }
    }

    // Calculate the time since the note started
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - startTime;

    // Generate the envelope signal
    if (elapsedTime < attackTime) {
      // Attack phase
      envelopeValue = map(elapsedTime, 0, attackTime, initialValue, 100);
    } else if (elapsedTime < (attackTime + decayTime)) {
      // Decay phase
      envelopeValue = map(elapsedTime, attackTime, attackTime + decayTime, 100, sustainLevel);
    } else {
      // Release phase
      unsigned long releaseElapsedTime = elapsedTime - (attackTime + decayTime);
      envelopeValue = map(releaseElapsedTime, 0, releaseTime, sustainLevel, 0);
      initialValue = 0;
    }

    B = ledHeightPotValue / 100.0 * (100 - weight) * initialValue / 100.0 * envelopeValue / 100.0;

    // scale with max value (0-100)
    int C = map(A + B, 0, 100, 0, max);

    // compute the number of LED powered ON for each musician
    int scaledValue_CE = map(C * weight_CE / 100.0, 0, 100, 0, NUM_LEDS_CE);
    int scaledValue_JB = map(C * weight_JB / 100.0, 0, 100, 0, NUM_LEDS_JB);
    int scaledValue_LB = map(C * weight_LB / 100.0, 0, 100, 0, NUM_LEDS_LB);
    int scaledValue_SA = map(C * weight_SA / 100.0, 0, 100, 0, NUM_LEDS_SA);

    // update the LED controlers
    setLEDs(leds_CE, NUM_LEDS_CE, scaledValue_CE, COLOR_CE);
    setLEDs(leds_JB, NUM_LEDS_JB, scaledValue_JB, COLOR_JB);
    setLEDs(leds_LB, NUM_LEDS_LB, scaledValue_LB, COLOR_LB);
    setLEDs(leds_SA, NUM_LEDS_SA, scaledValue_SA, COLOR_SA);

    // Serial.print("\t A: ");
    // Serial.print(A);
    // Serial.print("\t ledHeightPotValue: ");
    // Serial.print(ledHeightPotValue);
    // Serial.print("\t envelopeValue: ");
    // Serial.print(envelopeValue);
    // Serial.print("\t weight: ");
    // Serial.print(weight);  
    // Serial.print("\t mic: ");
    // Serial.print(mic);
    // Serial.print("\t initialValue: ");
    // Serial.print(initialValue);
    // Serial.print("\t B: ");
    // Serial.print(B);
    // Serial.print("\t C: ");
    // Serial.println(C);
    // Serial.print("\t scaledValue_CE: ");
    // Serial.println(scaledValue_CE);
    // Serial.print("\t weight: ");
    // Serial.println(weight);
  }

  Serial.print("channel: ");
  Serial.println(channel);

  memoryFlag = flag;
  // int currentMillis= millis();
  // Serial.println(millis()-memoryMillis);
  // memoryMillis = currentMillis;

}
