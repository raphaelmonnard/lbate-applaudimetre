#include <SPI.h> // Not actually used but needed to compile
#include <avr/pgmspace.h>
#include "spectacle.h"

// Init RadioHead for transmission
#include <RH_ASK.h>
RH_ASK driver(2000, 11, 14, 9);

// Init display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long activationTime = 0;
const unsigned long activationDuration = 3000;

#define BTN_next 8
#define BTN_previous 7
#define BTN_disp 9

int btn_next = LOW;
int btn_previous = LOW;
int memory_next_btn = LOW;
int memory_previous_btn = LOW;
bool memory_btn_next_flag = false;
bool memory_btn_previous_flag = false;

struct DataPacket {
  int channel = 0;
  int ledHeightPotValue = 0;
  int micSensitivity = 0;
  bool display = false;
};

DataPacket currentPacket;
DataPacket previousPacket;

int channel = 0;                  // Initial channel selection
int rows = sizeof(desc) / sizeof(desc[0]);

char buffer[255];

void updateDisplay(const char* text) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(text);
  display.display();
}

void setup()
{
  Serial.begin(9600);	  // Debugging only
  if (!driver.init()) {
    Serial.println("RF module initialization failed!");
  }

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

  pinMode(BTN_next, INPUT_PULLUP);
  pinMode(BTN_previous, INPUT_PULLUP);
}

void loop()
{
  // Update the current packet
  currentPacket.ledHeightPotValue = map(analogRead(A1), 0, 1023, 0, 100);
  currentPacket.micSensitivity = analogRead(A0);
  currentPacket.display = digitalRead(BTN_disp);
  Serial.print("Selected disp: ");
  Serial.println(currentPacket.display);

  if (currentPacket.display==1) {
    activationTime = millis();
    updateDisplay(buffer);
  }
    
  btn_next = digitalRead(BTN_next);
  if (btn_next == LOW && !memory_btn_next_flag) {
    currentPacket.channel = (currentPacket.channel + 1) % rows;
    Serial.print("Selected Channel: ");
    Serial.println(currentPacket.channel);
    strcpy_P(buffer, (char *)pgm_read_word(&(desc[currentPacket.channel])));  // Necessary casts and dereferencing, just copy.
    memory_btn_next_flag = true;
    
    activationTime = millis();  // Record the time when the button is pressed
    updateDisplay(buffer);
  } else if (btn_next == HIGH) {
    memory_btn_next_flag = false;
  }

  btn_previous = digitalRead(BTN_previous);
  if (btn_previous == LOW && !memory_btn_previous_flag) {
    currentPacket.channel = (currentPacket.channel + rows - 1) % rows;
    Serial.print("Selected Channel: ");
    Serial.println(currentPacket.channel);
    strcpy_P(buffer, (char *)pgm_read_word(&(desc[currentPacket.channel])));  // Necessary casts and dereferencing, just copy.
    memory_btn_previous_flag = true;
    
    activationTime = millis();  // Record the time when the button is pressed
    updateDisplay(buffer);
  } else if (btn_previous == HIGH) {
    memory_btn_previous_flag = false;
  }
  
  // Compare with the previous packet
  if (memcmp(&currentPacket, &previousPacket, sizeof(DataPacket)) != 0) {
    // If the packets are different, send the current packet
    driver.send((uint8_t*)&currentPacket, sizeof(DataPacket));
    driver.waitPacketSent();
    Serial.println("Package sent !");
    
    // Update the previous packet
    memcpy(&previousPacket, &currentPacket, sizeof(DataPacket));
  }

  // Check if activation duration has passed
  if (millis() - activationTime >= activationDuration) {
    display.clearDisplay();  // Deactivate the screen
    display.display();
  }
}
