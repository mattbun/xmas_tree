#include <Arduino.h>
#include <ArduinoOTA.h>
#include <TofbunStandard.h> // Lots of fun stuff in here
#include "secrets.h" // Wifi passwords and such (it should be gitignored)
#include <WiFi.h>

#include <Adafruit_NeoPixel.h>
#include <MQTT.h>

#define LED_PIN 14
#define LED_COUNT 8
#define DEFAULT_BRIGHTNESS 50
#define DEFAULT_MESSAGE "Merry Christmas!"

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
MQTTClient mqttClient;
WiFiClient wifiClient;

int currentBrightness = DEFAULT_BRIGHTNESS;
String mode = "rainbow";
int delayTime = 10;
bool allowOTA = true;
bool active = true;
uint32_t solidColor = 0xFF0000;

void messageReceived(String &topic, String &payload) {
  if (payload == 0) {
    return;
  }

  if (topic.equals("/xmas_tree/brightness")) {
    int newBrightness = payload.toInt();

    if (newBrightness < 0 || newBrightness > 255) {
      return;
    }

    currentBrightness = newBrightness;
    strip.setBrightness(newBrightness);
  } else if (topic.equals("/xmas_tree")) {
    if (payload.equals("off") || payload.equals("0")) {
      active = false;

      for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
        strip.setPixelColor(i, strip.Color(0,0,0));
      }

      strip.show();
    } else if (payload.equals("on") || payload.equals("1")) {
      active = true;
    }
  } else if (topic.equals("/xmas_tree/mode")) {
    mode = payload;
  } else if (topic.equals("/xmas_tree/delay")) {
    int newDelayTime = payload.toInt();

    if (newDelayTime < 0) {
      return;
    }
    delayTime = newDelayTime;
  } else if (topic.equals("/xmas_tree/allow_ota")) {
    if (payload.equals("off") || payload.equals("0")) {
      allowOTA = false;
    } else if (payload.equals("on") || payload.equals("1")) {
      allowOTA = true;
    }
  } else if (topic.equals("/xmas_tree/color")) {
    sscanf(payload.c_str(), "#%6x", &solidColor);
  } else if (topic.equals("/xmas_tree/message")) {
    displayMessage(payload);
  }
}

void setup() {
  initDisplay();
  initSerial();
  initWifi(WIFI_SSID, WIFI_PASSWORD); // Configured in src/secrets.h
  initOTA(false);

  displayMessage(WiFi.localIP().toString());

  mqttClient.begin(MQTT_HOST, wifiClient);
  mqttClient.onMessage(messageReceived);

  displayMessage("Starting mqtt...");
  while (!mqttClient.connect("xmas-tree")) {
    delay(1000);
  }

  mqttClient.subscribe("/xmas_tree");
  mqttClient.subscribe("/xmas_tree/brightness");
  mqttClient.subscribe("/xmas_tree/mode");
  mqttClient.subscribe("/xmas_tree/delay");
  mqttClient.subscribe("/xmas_tree/allow_ota");
  mqttClient.subscribe("/xmas_tree/color");
  mqttClient.subscribe("/xmas_tree/message");

  mqttClient.publish("/xmas_tree", "on");
  mqttClient.publish("/xmas_tree/brightness", String(currentBrightness));
  mqttClient.publish("/xmas_tree/mode", mode);
  mqttClient.publish("/xmas_tree/delay", String(delayTime));
  mqttClient.publish("/xmas_tree/allow_ota", allowOTA ? "1" : "0");
  mqttClient.publish("/xmas_tree/color", "#" + String(solidColor, HEX));

  strip.begin();
  strip.show();
  strip.setBrightness(currentBrightness);
}

int iterator = 0;
int increment = 256;
int firstPixelHue = 0;

void loop() {
  if (allowOTA) {
    ArduinoOTA.handle();
  }

  if (otaInProgress) {
    return;
  }

  mqttClient.loop();

  if (!active) {
    return;
  }

  if (firstPixelHue >= 5*65536) {
    firstPixelHue = 0;
  }

  // Now actually change the lights
  if (mode.equals("classic")) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      int pixelHue = firstPixelHue + (65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
  } else if (mode.equals("solid")) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      strip.setPixelColor(i, solidColor);
    }
  } else if (mode.equals("spring")) {
    iterator += increment;

    if (iterator <= 0 || iterator >= 5*65536) {
      increment = increment * -1;
    }
 
    int pixelHue = (65536L / 3) - ((iterator / 5) / 6);
    strip.fill(strip.gamma32(strip.ColorHSV(pixelHue)));
  } else {  // Rainbow
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
  }

  strip.show(); // Update strip with new contents
  firstPixelHue += 256;
  delay(delayTime);  // Pause for a moment
}
