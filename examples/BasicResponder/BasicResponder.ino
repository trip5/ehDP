/**
 * ehDP BasicResponder example
 *
 * Minimal ehDP responder — advertises this device on the local network.
 * Any ehDP client (Python scanner, Android app, etc.) sending EHDP_DISCOVER
 * to the subnet broadcast address on port 11867 will receive a JSON response.
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

    // Configure ehDP identity — at least one of setProject/setFirmware/setName recommended.
    // Do NOT call setMdns() if mDNS is disabled on this device — omit it instead.
    // Clients always fall back to 'ip' if the mdns field is absent.
    ehdp.setProject("MyProject");   // software/project name
    ehdp.setName("My ESP Device"); // user-given device label
    ehdp.setUIPort(0);              // 0 = no web UI
    ehdp.setVersion("1.0");

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
