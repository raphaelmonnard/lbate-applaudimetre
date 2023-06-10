#include <SPI.h> // Not actually used but needed to compile
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

#define BTN_1 4
#define BTN_2 5
#define LED 6

int btn = LOW;
int memory_btn = LOW;

bool memory_btn_flag = false;
int analogue_data = 0;

struct DataPacket {
  int channel;
  int value;
};

DataPacket currentPacket;
DataPacket previousPacket;

int channel = 0;                  // Initial channel selection
int rows = sizeof(desc) / sizeof(desc[0]);

char buffer[255];

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

  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
}

void loop()
{
  // Update the current packet
  currentPacket.value = map(analogRead(A1), 0, 1023, 0, 3); // Example analog value
    
  btn = digitalRead(BTN_1);
  if (btn == LOW && !memory_btn_flag) {
    currentPacket.channel = (currentPacket.channel + 1) % rows;
    Serial.print("Selected Channel: ");
    Serial.println(currentPacket.channel);
    strcpy_P(buffer, (char *)pgm_read_word(&(desc[currentPacket.channel])));  // Necessary casts and dereferencing, just copy.
    memory_btn_flag = true;
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(buffer);
  display.display();
  } else if (btn == HIGH) {
    memory_btn_flag = false;
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
}
