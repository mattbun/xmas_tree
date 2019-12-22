extern bool otaInProgress;

void initOTA(bool runInBackground);

void initDisplay();

void initWifi(const char ssid[], const char password[]);

void initSerial();

void displayMessage(String message);

void logAndDisplayMessage(char message[]);
