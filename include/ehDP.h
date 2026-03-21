#pragma once

#include <Arduino.h>
#include <WiFiUdp.h>

#define EHDP_PORT       11867
#define EHDP_REQUEST    "EHDP_DISCOVER"
#define EHDP_PROTO      "ehdp/1.0"
#define EHDP_MAX_CAPS   8

// ArduinoJson >= 6.x required
// ArduinoJson v7 uses JsonDocument directly
// ArduinoJson v6 uses DynamicJsonDocument

class EhDP {
public:
  EhDP();
  // Required
  void setUIPort(uint16_t port);          // 80 standard http port / 0 no Web UI
  // At least one of setName / setProject / setFirmware is recommended.
  void setName(const char* name);         // User-given device name, e.g. "My Radio"
  void setProject(const char* project);   // Project/software name, e.g. "ehRadio"
  void setFirmware(const char* firmware); // Firmware variant, e.g. "ESP32S3OLED"
  // Optional
  void setMaterialSymbol(uint32_t codepoint);          // Material symbol codepoint (decimal)
  void setMaterialSymbol(const char* codepoint);       // Material symbol codepoint (hex string like "0xe3af" or decimal string)
  void setVersion(const char* version);
  void setMdns(const char* mdns);
  void addCapability(const char* cap);
  // Call once after WiFi is connected
  bool begin();
  // Call every loop() iteration
  void loop();

private:
  WiFiUDP  _udp;
  String   _name;
  String   _project;
  String   _firmware;
  String   _version;
  String   _mdns;
  String   _materialSymbol;  // Stores symbol as string (hex or decimal)
  uint16_t _uiPort;
  String   _caps[EHDP_MAX_CAPS];
  uint8_t  _capCount;
  void   _respond(IPAddress remoteIP, uint16_t remotePort);
  String _buildJson();
};
