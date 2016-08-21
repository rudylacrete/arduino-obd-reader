#include "ObdReader.h"
#include "FastLED.h"

#define MAX_RPM 7000

#define GREEN CRGB::Green
#define RED CRGB::Red
#define ORANGE CRGB(255, 100, 0)

#define STATE_LED 6

ObdReader reader(2, 3, 4, 5);

// Define the array of leds
#define NUM_LEDS 10
CRGB leds[NUM_LEDS];

void setup()
{
  pinMode(STATE_LED, OUTPUT);
  digitalWrite(STATE_LED, LOW);
  Serial.begin(9600);
  FastLED.addLeds<WS2801, RBG>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  reader.setup();
  Serial.println("setup done ....");
  digitalWrite(STATE_LED, HIGH);
} 

 
void loop()
{
  drawRpm(reader.getRpm());
  delay(100);
}

void drawRpm(int rpm) {
  uint8_t NUMBER_GREEN = 4, NUMBER_ORANGE = 3, NUMBER_RED = 3;
  uint8_t oThreshold = NUMBER_GREEN, rThreshold = NUMBER_GREEN + NUMBER_ORANGE;
  uint8_t ledToDisplay = map(rpm, 0, MAX_RPM, 0, NUM_LEDS);
  FastLED.clear(true);
  for(int i = 0; i < ledToDisplay; i++) {
    if(i >= rThreshold) leds[i] = RED;
    else if(i >= oThreshold) leds[i] = ORANGE;
    else leds[i] = GREEN;
  }
  FastLED.show();
}

