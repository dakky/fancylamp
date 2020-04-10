#include <Arduino.h>
#include <common.h>
#include <FastLED.h>

// FastLED settings:
#define NUM_STRIPS 3
#define NUM_LEDS_PER_STRIP 14
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS
#define FRAMES_PER_SECOND 1000
#define BRIGHTNESS 75

void simpleColor();
void rainbow();
void confetti();
void sinelon();
void bpm();
void juggle();
void menuLed(bool);