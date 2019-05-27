#include <Arduino.h>
#include <ArduinoOTA.h>
#include <EasyButton.h>
#define FASTLED_INTERRUPT_RETRY_COUNT 1
#include <FastLED.h>
#include <LedProfiles.h>
#include <functionsOTA.h>
#include <functionsWLAN.h>
#include <config.h>

// dynamic functioncalls
// LedProfiles[]: List of used profiles
// ledProfilesCount: c-style: arrays start at 0 => count -1
typedef void (*profile)();
profile LedProfiles[] = {simpleColor, testProfile2, testProfile3};
int ledProfilesCount = 2;
int colorProfile = 0;

// FastLED settings:
#define NUM_STRIPS 3
#define NUM_LEDS_PER_STRIP 60
#define NUM_LEDS NUM_LEDS_PER_STRIP *NUM_STRIPS
CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP];

WiFiClient espClient;
EasyButton button(buttonPin);

int nextColorProfile(int colorProfile)
{

  if (colorProfile < ledProfilesCount)
  {
    return colorProfile + 1;
  }
  else
  {
    return 0;
  }
}

void button_long_pressed_callback()
{
  Serial.println("Button long pressed.");
}

void button_short_pressed_callback()
{
  Serial.println("Button short pressed.");
  colorProfile = nextColorProfile(colorProfile);
}

void setup()
{
  Serial.begin(115200);

  // WIFI and OTA
  setup_wifi(ssid, password, hostname);
  setup_ota(hostname);

  // Button handling
  Serial.println("Button: Setting up ...");
  button.begin();
  button.onPressedFor(2000, button_long_pressed_callback);
  button.onPressed(button_short_pressed_callback);
  Serial.println("Button: Setup done ...");

  // FastLED: stripe definition
  Serial.println("FastLED: Setting up ...");
  FastLED.addLeds<WS2812B, 6, GRB>(leds, 0, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, 7, GRB>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, 8, GRB>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  Serial.println("ButFastLEDton: Setup done ...");

  Serial.println("All setup steps are done.");
}

void loop()
{
  button.read();
  ArduinoOTA.handle();
  if (colorProfile != -1)
  {
    LedProfiles[colorProfile]();
  }
}
