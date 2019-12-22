#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "SSD1306Wire.h"
#include "OLEDDisplay.h"

#include "TofbunStandard.h"

// Try to put ota on a different core
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

const int SERIAL_BAUD_RATE = 115200;
SSD1306Wire display(0x3c, 5, 4);

// Use this to stop doing things when an OTA is in progress
bool otaInProgress = false;

void displayMessage(String message) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.drawString(display.getWidth()/2, display.getHeight()/2 - 10, message);
  display.display();
}

void logAndDisplayMessage(char message[]) {
  Serial.println(message);
  displayMessage(message);
}

void initSerial() {
  Serial.begin(SERIAL_BAUD_RATE);
}

void initDisplay() {
  display.init();
  display.flipScreenVertically();
  display.setContrast(255);
  displayMessage("Connecting to wifi...");
}

void initWifi(const char ssid[], const char password[]) {
  logAndDisplayMessage("Connecting to WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    logAndDisplayMessage("Connection failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void otaLoop(void *pvParameters) {
  while (1) {
    ArduinoOTA.handle();
  }
}

void initOTA(bool runInBackground) {

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.begin();
  ArduinoOTA.onStart([=]() {
    otaInProgress = true;
    Serial.println("OTA update!");

    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display.drawString(display.getWidth()/2, display.getHeight()/2 - 10, "OTA Update");
    display.display();
  });

  ArduinoOTA.onProgress([=](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));

    display.drawProgressBar(4, 32, 120, 8, progress / (total / 100) );
    display.display();
  });

  ArduinoOTA.onEnd([=]() {
    otaInProgress = false;
    Serial.println("\nDone!");

    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display.drawString(display.getWidth()/2, display.getHeight()/2, "Restart");
    display.display();
  });

  ArduinoOTA.onError([=](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");

    otaInProgress = false;
  });

  if (runInBackground) {
    // Running on core 0 makes it reboot over and over
    xTaskCreatePinnedToCore(otaLoop, "otaLoop", 4096, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
    Serial.println("Watching for OTA updates on core " + ARDUINO_RUNNING_CORE);
  }
}
