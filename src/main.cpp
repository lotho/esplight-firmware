
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include "FastLED.h"
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "config.h"

WiFiServer TelnetServer(8266);

#define DATA_PIN 7

// Define the array of leds
CRGB leds[NUM_LEDS];

int var = 0;

int ledPin = 2; // GPIO2

CRGB endclr;
CRGB midclr;

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[200];
int value = 0;

void cycle() {
  fill_gradient_RGB(leds, 0, endclr, NUM_LEDS/2, midclr);
  fill_gradient_RGB(leds, NUM_LEDS/2+1, midclr, NUM_LEDS, endclr);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String json;
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("payload: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    json.concat((char)payload[i]);
  }
  Serial.println();

  StaticJsonBuffer<500> jsonBuffer;

  // char json[] =
  // "{\"brightness\":20,\"color_temp\":155,\"color\":{\"r\":140,\"g\":100,\"b\":220,\"x\":0.127,\"y\":0.123}
  //,"effect":"colorloop","state":"ON","transition":2,"white_value":150}";

  JsonObject& root = jsonBuffer.parseObject(json);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  byte brightness = root["brightness"];

  byte color = root["color"];
  byte red = root["color"]["r"];
  byte green = root["color"]["g"];
  byte blue = root["color"]["b"];

  Serial.println(red);

  String effect = root["effect"];
  String state = root["state"];

  if (state == "OFF") {
    fill_solid( leds, NUM_LEDS, CRGB(0, 0, 0));
    FastLED.setBrightness(0);
    FastLED.show();
  }
  else if (brightness != NULL) {
    FastLED.setBrightness(brightness);
    FastLED.show();
  }
  else if (state == "ON" && color != NULL) {
    fill_solid( leds, NUM_LEDS, CRGB(red, green, blue));
    // FastLED.setBrightness(brightness);
    FastLED.show();
  }
  else {
    fill_solid( leds, NUM_LEDS, CRGB(200, 30, 20));
    FastLED.setBrightness(127);
    FastLED.show();
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      Serial.println(mqtt_server);
      client.subscribe(mqtt_topic);
      Serial.println ("subscribed");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  //To make Arduino software autodetect OTA device
  TelnetServer.begin();

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  FastLED.clear();

  Serial.begin(115200);
  delay(10);

  leds[40] = CRGB::Green;
  FastLED.show();

  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("OTA End");
    Serial.println("Rebooting...");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  // Connect to WiFi network
  Serial.println();

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Print the IP address
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  ArduinoOTA.handle();

  delay(1);
}
