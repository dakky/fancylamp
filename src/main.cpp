#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <EasyButton.h>
#include <secret.h>

const char *ssid = "firebird";

const char *hostname = "fancylamp";
const char *mqtt_server = "autohome.intern.dakkar.eu";
const int   mqtt_server_port = 1883;

const char *subTopicLight = "home/GF/WZ/lamp1/light/command";
const char *pubTopicLight = "home/GF/WZ/lamp1/light/state";

const int led1 = 5;      // D0
const int buttonPin = 4; // D2

WiFiClient espClient;
PubSubClient mqtt_client(espClient);
EasyButton button(buttonPin);

/* 
 * ###########################
 *  functions
 * ###########################
 */
void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  MDNS.begin(hostname);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_ota()
{

  ArduinoOTA.setHostname(hostname);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
    {
      Serial.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      Serial.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      Serial.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      Serial.println("Receive Failed");
    }
    else if (error == OTA_END_ERROR)
    {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void mqtt_recv_callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1')
  {
    digitalWrite(led1, HIGH);
  }
  else
  {
    digitalWrite(led1, LOW);
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!mqtt_client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt_client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // ... and resubscribe
      mqtt_client.subscribe(subTopicLight);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void button_long_pressed_callback()
{
  Serial.print("Button long pressed.");
  delay(2000);
}

void button_short_pressed_callback()
{
  Serial.print("Button short pressed.");
  delay(2000);
}

void setup()
{
  // Pin config
  pinMode(led1, OUTPUT);
  Serial.begin(115200);

  // WIFI und OTA
  setup_wifi();
  setup_ota();

  // mqtt
  Serial.println("MQTT: Setting up ...");
  mqtt_client.setServer(mqtt_server, mqtt_server_port);
  mqtt_client.setCallback(mqtt_recv_callback);
  Serial.println("MQTT: Setup done ...");

  // Button handling
  Serial.println("Button: Setting up ...");
  button.begin();
  button.onPressedFor(2000, button_long_pressed_callback);
  button.onPressed(button_short_pressed_callback);
  Serial.println("Button: Setup done ...");

  Serial.println("All setup steps are done.");
}

void loop()
{
  if (!mqtt_client.connected())
  {
    reconnect();
  }
  mqtt_client.loop();
  button.read();
  ArduinoOTA.handle(); 
}
