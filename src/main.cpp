#include <Arduino.h>

#include <ObdReader.h>
#include "FastLED.h"

#define MAX_RPM 7000

#define GREEN CRGB::Green
#define RED CRGB::Red
#define ORANGE CRGB(255, 100, 0)

#define STATE_LED 6

void drawRpm(int);
void drawInitProgress(uint8_t);
void onBluetoothProgressCallback(uint8_t progress) {
  drawInitProgress(progress);
}

bool bluetoothInitOk = false;

ObdReader reader((obd_reader_conf_t){
  .rxPin = 2,
  .txPin = 3,
  .powerPin = 4,
  .atPin = 5,
  .slave_mac_addr = "FF:FF:FF:FF:FF:FF",
  .password = "1234",
  .progressCallback = onBluetoothProgressCallback
});

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
  bluetoothInitOk = reader.setup();
  if(bluetoothInitOk)
    Serial.println("setup done ....");
  else {
    Serial.println("Bluetooth was not initialized correctly");
    FastLED.showColor(RED);
    delay(4000);
    FastLED.clear(true);
  }
  digitalWrite(STATE_LED, bluetoothInitOk ? HIGH : LOW);
}


void loop()
{
  if(!bluetoothInitOk) return;
  drawRpm(reader.getRpm());
  delay(100);
}

void drawRpm(int rpm) {
  uint8_t NUMBER_GREEN = 4, NUMBER_ORANGE = 3;
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

void drawInitProgress(uint8_t percent) {
  uint8_t ledToDisplay = map(percent, 0, 100, 0, NUM_LEDS);
  Serial.println(percent);
  FastLED.clear(true);
  for(int i = 0; i < ledToDisplay; i++) {
    leds[i] = GREEN;
  }
  FastLED.show();
}
