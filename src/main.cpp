#include <Arduino.h>
#include <TofbunStandard.h> // Lots of fun stuff in here
#include "secrets.h" // Wifi passwords and such (it should be gitignored)

#include <WiFi.h> // Only used here to display the IP address

void setup() {
  initDisplay();
  initSerial();
  initWifi(WIFI_SSID, WIFI_PASSWORD); // Configured in src/secrets.h
  initOTA();

  displayMessage(WiFi.localIP().toString());

  // TODO
}

void loop() {
  delay(2000);

  if (otaInProgress) {
    return;
  }

  // TODO
}
