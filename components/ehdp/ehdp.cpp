// ehDP ESPHome component
// https://github.com/trip5/ehDP

#include "ehdp.h"
#include "esphome/core/log.h"

#include <cstring>
#include <cstdio>

#ifdef ESP32
  #include <lwip/sockets.h>
  #include <lwip/netdb.h>
  #include <arpa/inet.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <WiFiUdp.h>
#endif

namespace esphome {
namespace ehdp {

static const char *TAG        = "ehdp";
static const uint16_t PORT    = 11867;
static const char *REQUEST    = "EHDP_DISCOVER";
static const char *PROTO      = "ehdp/1.0";

// ---------------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------------

void EhDPComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "ehDP Discovery Protocol");
  if (sock_ < 0) {
    ESP_LOGCONFIG(TAG, "  Status: DISABLED");
  } else {
    ESP_LOGCONFIG(TAG, "  Port: %u", PORT);
    std::string ip = get_ip_str_();
    ESP_LOGCONFIG(TAG, "  IP: %s", ip.c_str());
  }
}

void EhDPComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ehDP...");
  
#ifdef EHDP_ENABLE_DISABLE
  // Initialize preference object for enable/disable feature
  pref_ = global_preferences->make_preference<bool>(fnv1_hash("ehdp_enabled"));
  
  // Restore user preference
  bool restored_disabled = false;
  if (pref_.load(&restored_disabled)) {
    user_disabled_ = restored_disabled;
    ESP_LOGCONFIG(TAG, "Restored state: %s", user_disabled_ ? "disabled" : "enabled");
  }
  
  // If user previously disabled it, don't start the socket
  if (user_disabled_) {
    ESP_LOGCONFIG(TAG, "ehDP disabled by user preference");
    sock_ = -1;
    return;
  }
#endif
  
  #ifdef ESP32
    ESP_LOGD(TAG, "Creating UDP socket (ESP32)");
    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ < 0) {
      ESP_LOGE(TAG, "Failed to create UDP socket");
      this->mark_failed();
      return;
    }
    ESP_LOGD(TAG, "Socket created, setting options");
    int opt = 1;
    setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    ESP_LOGD(TAG, "Binding to port %u", PORT);
    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      ESP_LOGE(TAG, "Failed to bind UDP socket on port %u", PORT);
      close(sock_);
      sock_ = -1;
      this->mark_failed();
      return;
    }
    ESP_LOGD(TAG, "Socket bound successfully");
  #else // ESP8266 fallback to WiFiUDP
    ESP_LOGD(TAG, "Creating UDP (ESP8266)");
    WiFiUDP *udp = new WiFiUDP();
    if (!udp->begin(PORT)) {
      ESP_LOGE(TAG, "Failed to bind UDP port %u", PORT);
      this->mark_failed();
      return;
    }
    sock_ = (int)udp; // Store pointer as int for simplicity in this component
  #endif
  ESP_LOGD(TAG, "ehDP setup complete, sock_ = %d", sock_);
}

void EhDPComponent::loop() {
  if (sock_ < 0) return;
  #ifdef ESP32
    char buf[32];
    struct sockaddr_in sender{};
    socklen_t sender_len = sizeof(sender);
    int len = recvfrom(sock_, buf, sizeof(buf) - 1, MSG_DONTWAIT, (struct sockaddr *)&sender, &sender_len);
    if (len <= 0) return;
    buf[len] = '\0';
    while (len > 0 && (buf[len - 1] == '\r' || buf[len - 1] == '\n' || buf[len - 1] == ' '))
        buf[--len] = '\0';
    if (strcmp(buf, REQUEST) != 0) return;
    ESP_LOGD(TAG, "Received EHDP_DISCOVER from %s", inet_ntoa(sender.sin_addr));
    std::string json = build_json_();
    sendto(sock_, json.c_str(), json.length(), 0,
        (struct sockaddr *)&sender, sender_len);
  #else
    WiFiUDP *udp = (WiFiUDP *)sock_;
    int packetSize = udp->parsePacket();
    if (packetSize <= 0) return;
    char buf[32];
    int len = udp->read(buf, sizeof(buf) - 1);
    if (len <= 0) return;
    buf[len] = '\0';
    while (len > 0 && (buf[len - 1] == '\r' || buf[len - 1] == '\n' || buf[len - 1] == ' '))
        buf[--len] = '\0';
    if (strcmp(buf, REQUEST) != 0) return;
    ESP_LOGD(TAG, "Received EHDP_DISCOVER from %s", udp->remoteIP().toString().c_str());
    std::string json = build_json_();
    udp->beginPacket(udp->remoteIP(), udp->remotePort());
    udp->write(json.c_str());
    udp->endPacket();
  #endif
}

std::string EhDPComponent::get_ip_str_() {
  #ifdef ESP32
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) return "0.0.0.0";
    struct sockaddr_in dummy{};
    dummy.sin_family = AF_INET;
    dummy.sin_port   = htons(80);
    inet_aton("8.8.8.8", &dummy.sin_addr);
    connect(s, (struct sockaddr *)&dummy, sizeof(dummy));
    struct sockaddr_in local{};
    socklen_t len = sizeof(local);
    getsockname(s, (struct sockaddr *)&local, &len);
    close(s);
    return std::string(inet_ntoa(local.sin_addr));
  #else
    return WiFi.localIP().toString().c_str();
  #endif
}

std::string EhDPComponent::build_json_() {
  std::string ip  = get_ip_str_();
  std::string out = "{";
  out += "\"proto\":\"" + std::string(PROTO) + "\"";
  out += ",\"ip\":\""    + ip    + "\"";
  if (!name_.empty())     out += ",\"name\":\""     + name_     + "\"";
  if (!project_.empty())  out += ",\"project\":\""  + project_  + "\"";
  if (!firmware_.empty()) out += ",\"firmware\":\"" + firmware_ + "\"";
  if (!version_.empty())  out += ",\"version\":\""  + version_  + "\"";
  if (!mdns_.empty())     out += ",\"mdns\":\""     + mdns_     + "\"";
  if (ui_port_ > 0) {
    char port_buf[8];
    snprintf(port_buf, sizeof(port_buf), "%u", ui_port_);
    out += ",\"ui_port\":";
    out += port_buf;
  }
  if (!material_symbol_.empty()) {
    // Check if it's a hex string or should be sent as integer
    if (material_symbol_[0] == '0' && (material_symbol_[1] == 'x' || material_symbol_[1] == 'X')) {
      out += ",\"material_symbol\":\"" + material_symbol_ + "\"";
    } else {
      // Try to send as integer (no quotes)
      out += ",\"material_symbol\":" + material_symbol_;
    }
  }
  if (!capabilities_.empty()) {
    out += ",\"capabilities\":[";
    for (size_t i = 0; i < capabilities_.size(); i++) {
      if (i > 0) out += ",";
      out += "\"" + capabilities_[i] + "\"";
    }
    out += "]";
  }
  out += "}";
  return out;
}

#ifdef EHDP_ENABLE_DISABLE

void EhDPComponent::disable() {
  if (sock_ < 0 && user_disabled_) {
    ESP_LOGD(TAG, "ehDP already disabled");
    return;
  }
  ESP_LOGD(TAG, "Disabling ehDP discovery");
  user_disabled_ = true;
  pref_.save(&user_disabled_);
  if (sock_ >= 0) {
    #ifdef ESP32
      close(sock_);
    #else
      WiFiUDP *udp = (WiFiUDP *)sock_;
      udp->stop();
      delete udp;
    #endif
    sock_ = -1;
  }
}

void EhDPComponent::enable() {
  ESP_LOGD(TAG, "enable() called, current sock_ = %d", sock_);
  if (sock_ >= 0) {
    ESP_LOGD(TAG, "ehDP already enabled, nothing to do");
    user_disabled_ = false;
    pref_.save(&user_disabled_);
    return; // Already enabled
  }
  ESP_LOGD(TAG, "Enabling ehDP discovery");
  user_disabled_ = false;
  pref_.save(&user_disabled_);
  #ifdef ESP32
    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ < 0) {
      ESP_LOGE(TAG, "Failed to create UDP socket");
      return;
    }
    int opt = 1;
    setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      ESP_LOGE(TAG, "Failed to bind UDP socket on port %u", PORT);
      close(sock_);
      sock_ = -1;
      return;
    }
  #else
    WiFiUDP *udp = new WiFiUDP();
    if (!udp->begin(PORT)) {
      ESP_LOGE(TAG, "Failed to bind UDP port %u", PORT);
      delete udp;
      sock_ = -1;
      return;
    }
    sock_ = (int)udp;
  #endif
  ESP_LOGD(TAG, "ehDP discovery enabled, sock_ = %d", sock_);
}

#endif  // EHDP_ENABLE_DISABLE

}  // namespace ehdp
}  // namespace esphome
