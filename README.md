# Getting Started

1. Install [PlatformIO](https://platformio.org/)

2. Add a `secrets.h`:

    ```c
    #define WIFI_SSID "wifi-h8"
    #define WIFI_PASSWORD "beepbeepboop"
    #define MQTT_HOST "8.6.7.5"
    ```

3. Build it!

    ```shell
    pio run
    ```

4. The OTA process will open a server at port 8267. Make sure your computer doesn't block it. In ChromeOS, you'll need to open a port forward to Linux!

5. Install it via OTA

    ```shell
    pio run -t upload --upload-port 5.3.0.9
    ```

