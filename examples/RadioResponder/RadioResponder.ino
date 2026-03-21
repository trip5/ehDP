/**
 * ehDP RadioResponder example
 *
 * Full-featured ehDP responder for an internet radio device.
 * Demonstrates all optional fields: version, mDNS, web UI port,
 * and capabilities.
 *
 * Requirements:
 *   - Board: ESP32 or ESP8266
 *   - Library: ehDP (this library)
 *   - Library: ArduinoJson >= 6.x  (https://arduinojson.org)
 */

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <ehDP.h>

// --- WiFi credentials ---
const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASS = "your-password";

// --- Device identity ---
// project = software/project name, firmware = build variant, name = user-given label
const char*    DEVICE_PROJECT    = "ehRadio";
const char*    DEVICE_FIRMWARE   = "ESP32S3OLED";
const char*    DEVICE_NAME       = "Kitchen Radio";   // user-given
const char*    DEVICE_SYMBOL     = "0xe03e";          // Material symbol (Radio: 0xe03e / 57406)
const char*    DEVICE_VERSION    = "2026.03.18";
const char*    DEVICE_MDNS       = "kitchenradio";   // ONLY set if mDNS is enabled on this device
                                                         // Leave as "" or omit setMdns() if mDNS is disabled.
                                                         // Clients always fall back to 'ip' if mdns is absent.
const uint16_t DEVICE_UI_PORT    = 80;                   // 80 standard http port / 0 no Web UI

EhDP ehdp;

void setup() {
    Serial.begin(115200);
    delay(200);

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

    // Required for WebUI
    ehdp.setUIPort(DEVICE_UI_PORT);

    // At least one of project / firmware / name is recommended
    ehdp.setProject(DEVICE_PROJECT);
    ehdp.setFirmware(DEVICE_FIRMWARE);
    ehdp.setName(DEVICE_NAME);

    // Optional fields
    ehdp.setMaterialSymbol(DEVICE_SYMBOL);  // Material symbol (hex string)
    ehdp.setVersion(DEVICE_VERSION);
    ehdp.setMdns(DEVICE_MDNS);

    // Start listening
    if (ehdp.begin()) {
        Serial.println("ehDP listening on port " + String(EHDP_PORT));
    } else {
        Serial.println("ehDP failed to start.");
    }
}

void loop() {
    ehdp.loop();
}
