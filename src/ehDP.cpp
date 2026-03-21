#include "ehDP.h"

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#else
  #error "ehDP requires an ESP32 or ESP8266 board."
#endif

// Support both ArduinoJson v6 and v7
#include <ArduinoJson.h>
#if ARDUINOJSON_VERSION_MAJOR >= 7
  #define EHDP_JSON_DOC JsonDocument
#else
  #define EHDP_JSON_DOC DynamicJsonDocument(512)
#endif

EhDP::EhDP() : _uiPort(0), _capCount(0) {}

void EhDP::setName(const char* name)            { _name = name; }
void EhDP::setProject(const char* project)      { _project = project; }
void EhDP::setFirmware(const char* firmware)    { _firmware = firmware; }
void EhDP::setVersion(const char* version)      { _version = version; }
void EhDP::setMdns(const char* mdns)            { _mdns = mdns; }
void EhDP::setUIPort(uint16_t port)             { _uiPort = port; }

void EhDP::setMaterialSymbol(uint32_t codepoint) {
  _materialSymbol = String(codepoint);  // Store as decimal string
}

void EhDP::setMaterialSymbol(const char* codepoint) {
  _materialSymbol = codepoint;  // Store as provided (hex string or decimal string)
}

void EhDP::addCapability(const char* cap) {
  if (_capCount < EHDP_MAX_CAPS) {
    _caps[_capCount++] = cap;
  }
}

bool EhDP::begin() {
  return _udp.begin(EHDP_PORT) == 1;
}

void EhDP::loop() {
  int packetSize = _udp.parsePacket();
  if (packetSize <= 0) return;
  char buf[32];
  int len = _udp.read(buf, sizeof(buf) - 1);
  if (len <= 0) return;
  buf[len] = '\0';
  while (len > 0 && (buf[len - 1] == '\r' || buf[len - 1] == '\n' || buf[len - 1] == ' ')) {
    buf[--len] = '\0';
  }
  if (strcmp(buf, EHDP_REQUEST) != 0) return;
  _respond(_udp.remoteIP(), _udp.remotePort());
}

String EhDP::_buildJson() {
  #if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
  #else
    DynamicJsonDocument doc(512);
  #endif
  // Required
  doc["proto"] = EHDP_PROTO;
  doc["ip"]    = WiFi.localIP().toString();
  // Material symbol (optional)
  if (_materialSymbol.length()) {
    // Try to parse as integer if it doesn't start with "0x"
    if (_materialSymbol.startsWith("0x") || _materialSymbol.startsWith("0X")) {
      doc["material_symbol"] = _materialSymbol;  // Send as hex string
    } else {
      // Try to parse as integer
      long val = _materialSymbol.toInt();
      if (val > 0 || _materialSymbol == "0") {
        doc["material_symbol"] = val;  // Send as integer
      } else {
        doc["material_symbol"] = _materialSymbol;  // Send as string if not parseable
      }
    }
  }
  // Name fields (at least one recommended)
  if (_project.length())       doc["project"]          = _project;
  if (_firmware.length())      doc["firmware"]         = _firmware;
  if (_name.length())          doc["name"]             = _name;
  // Extra info
  if (_version.length())       doc["version"]          = _version;
  if (_mdns.length())          doc["mdns"]             = _mdns;
  if (_uiPort > 0)             doc["ui_port"]          = _uiPort;
  if (_capCount > 0) {
    JsonArray caps = doc["capabilities"].to<JsonArray>();
    for (uint8_t i = 0; i < _capCount; i++) {
      caps.add(_caps[i]);
    }
  }
  String out;
  serializeJson(doc, out);
  return out;
}

void EhDP::_respond(IPAddress remoteIP, uint16_t remotePort) {
  String json = _buildJson();
  if (_udp.beginPacket(remoteIP, remotePort)) {
    _udp.write((const uint8_t*)json.c_str(), json.length());
    _udp.endPacket();
  }
}
