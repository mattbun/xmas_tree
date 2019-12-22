extern bool otaInProgress;

void initOTA();

void initDisplay();

void initWifi(const char ssid[], const char password[]);

void initSerial();

void displayMessage(String message);

void logAndDisplayMessage(char message[]);
