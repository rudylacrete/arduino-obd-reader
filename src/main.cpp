#include <Arduino.h>

#include <ObdReader.h>
#include "FastLED.h"

#define MAX_RPM 7000

#define GREEN CRGB::Green
#define RED CRGB::Red
#define BLUE CRGB::Blue
#define ORANGE CRGB(255, 100, 0)

#define STATE_LED 6
#define BUTTON 8
#define LONG_PRESS_DELAY 3000

void drawRpm(int);
void drawInitProgress(uint8_t);
void onBluetoothProgressCallback(uint8_t progress) {
  drawInitProgress(progress);
}

bool bluetoothInitOk = false;

uint8_t ledBrightness = 255;

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time
long btnHighTime = 0;
bool buttonState = false;
bool longPressTriggered = false;
long lastRpmTime = 0;
long rpmDelay = 100; // read one each 100ms

int rpmErrorCount = 0;

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

void toggleLedBrightness() {
  ledBrightness = ledBrightness == 255 ? 10 : 255;
  FastLED.setBrightness(ledBrightness);
  FastLED.show();
}

void stop() {
  FastLED.showColor(BLUE);
  delay(300);FastLED.clear(true);delay(200);
  FastLED.showColor(BLUE);
  delay(300);FastLED.clear(true);delay(200);
  FastLED.showColor(BLUE);
  delay(300);
  FastLED.clear(true);
  digitalWrite(STATE_LED, LOW);
  reader.stop();
  bluetoothInitOk = false;
  rpmErrorCount = 0;
}

void start() {
  FastLED.showColor(GREEN);
  delay(300);FastLED.clear(true);delay(200);
  FastLED.showColor(GREEN);
  delay(300);FastLED.clear(true);
  bluetoothInitOk = reader.setup();
  if(!bluetoothInitOk) {
    FastLED.showColor(RED);
    delay(4000);
    FastLED.clear(true);
    stop();
  }
  else
    digitalWrite(STATE_LED, HIGH);
}


void setup()
{
  pinMode(STATE_LED, OUTPUT);
  digitalWrite(STATE_LED, LOW);
  Serial.begin(9600);
  FastLED.addLeds<WS2801, RBG>(leds, NUM_LEDS);
  FastLED.setBrightness(ledBrightness);
  start();
}



void handleBtnState() {
  // read button state
  uint8_t reading = digitalRead(BUTTON);
  if (reading != buttonState && !lastDebounceTime) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if (lastDebounceTime && (millis() - lastDebounceTime) > debounceDelay) {
    bool previousState = buttonState;
    buttonState = reading;
    // count how long the button is pressed
    if(!previousState && buttonState) btnHighTime = millis();
    if(buttonState && btnHighTime) {
      long pressDelay = millis() - btnHighTime;
      // long press
      if(pressDelay > LONG_PRESS_DELAY) {
        btnHighTime = 0;
        longPressTriggered = true;
        if(bluetoothInitOk) stop();
        else start();
      }
    }
    // short press
    else if(!longPressTriggered && previousState && !buttonState){
      if(bluetoothInitOk) toggleLedBrightness();
      else start();
    }
    if(!buttonState) {
      lastDebounceTime = 0;
      longPressTriggered = false;
    }
  }
}

void loop()
{
  handleBtnState();
  if(!bluetoothInitOk) return;

  // update RPM
  if(millis() - lastRpmTime >= rpmDelay) {
    lastRpmTime = millis();
    int rpm = reader.getRpm();
    if(rpm) drawRpm(rpm);
    else {
      if(++rpmErrorCount > 4) stop();
    }
  }
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
