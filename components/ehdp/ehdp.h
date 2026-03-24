#pragma once

#include "esphome/core/component.h"

#include <string>
#include <vector>

namespace esphome {
namespace ehdp {

class EhDPComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }
  void set_name(const char *val)            { name_ = val; }
  void set_project(const char *val)         { project_ = val; }
  void set_firmware(const char *val)        { firmware_ = val; }
  void set_material_symbol(const char *val)   { material_symbol_ = val; }
  void set_version(const char *val)         { version_ = val; }
  void set_mdns(const char *val)            { mdns_ = val; }
  void set_ui_port(uint16_t port)           { ui_port_ = port; }
  void add_capability(const char *cap)      { capabilities_.push_back(cap); }
  void disable();
  void enable();
  bool is_enabled() const                   { return sock_ >= 0; }

 protected:
  std::string name_;
  std::string project_;
  std::string firmware_;
  std::string material_symbol_;
  std::string version_;
  std::string mdns_;
  uint16_t    ui_port_{0};
  std::vector<std::string> capabilities_;
  int sock_{-1};
  std::string build_json_();
  std::string get_ip_str_();
};

}  // namespace ehdp
}  // namespace esphome
