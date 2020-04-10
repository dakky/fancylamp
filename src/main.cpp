#include <Arduino.h>
#include <ArduinoOTA.h>
#include <config.h>
#include <common.h>
#include <EasyButton.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_INTERRUPT_RETRY_COUNT 3
#include <FastLED.h>
#include <LedProfiles.h>
#include <functionsOTA.h>
#include <functionsWLAN.h>

// dynamic functioncalls
// SimplePatternList[]: List of used profiles
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {simpleColor, rainbow, confetti, sinelon, bpm, juggle};
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

// TODO: aus eeprom lesen, aktuell startet es immer mit dem ersten profil
// Index number of which pattern is current
int gCurrentPatternNumber = 0;
int gHue = 0; // rotating "base color" used by many of the patterns

// array of LED stripe
CRGB leds[NUM_LEDS];

bool poweredOn = true;
bool menuMode = false;

WiFiClient espClient;
EasyButton button(buttonPin);

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
}

void button_long_pressed_callback()
{
  Serial.println("Button long pressed.");
  if (menuMode)
  {
    // Menucontrolling
    Serial.println("Menumode: saving and leaving menumode");
    menuMode = false;
    menuLed(false);
  }
  else
  {
    Serial.print("Powermode: ");
    // this long press is meant for powering on/off
    poweredOn = !poweredOn;
    if (!poweredOn)
    {
      Serial.println("powering OFF");

      // switch off all leds
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
    }
  }
}

void button_short_pressed_callback()
{
  if (menuMode)
  {
    Serial.println("Menumode: Doing something ...");
  }
  else
  {
    Serial.println("defaultmode: next profile ...");
    nextPattern();
  }
}

void button_menu_sequence_pressed_callback()
{
  if (!menuMode)
  {
    Serial.println("Entering menu ...");
    menuMode = true;
    menuLed(true);
  }
}

void setup()
{
  delay(3000); // 3 second delay for recovery
  Serial.begin(115200);

  // FastLED: stripe definition
  Serial.println("FastLED: Setting up ...");
  FastLED.addLeds<WS2812B, dataPin, GRB>(leds, 0, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  Serial.println("FastLED: Setup done ...");

  // WIFI and OTA
  setup_wifi(ssid, password, hostname);
  setup_ota(hostname);

  // Button handling
  Serial.println("Button: Setting up ...");
  button.begin();
  button.onPressedFor(2000, button_long_pressed_callback);
  button.onPressed(button_short_pressed_callback);
  button.onSequence(3, 2000, button_menu_sequence_pressed_callback);
  Serial.println("Button: Setup done ...");

  Serial.println("All setup steps are done.");
}

void loop()
{
  button.read();
  ArduinoOTA.handle();
  if (poweredOn)
  {
    if (menuMode)
    {
    }
    else
    {
      // Call the current pattern function once, updating the 'leds' array
      gPatterns[gCurrentPatternNumber]();

      FastLED.show();
      // insert a delay to keep the framerate modest
      FastLED.delay(1000 / FRAMES_PER_SECOND);

      EVERY_N_MILLISECONDS(20) { gHue++; } // slowly cycle the "base color" through the rainbow
    }
  }
}
