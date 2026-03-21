# ehDP — eh Discovery Protocol

> **Lightweight UDP-based device discovery for ESP32/ESP8266 and IoT hardware**

[![License: LGPL-2.1](https://img.shields.io/badge/License-LGPL%202.1-blue.svg)](LICENSE)

**ehDP** is a simple, zero-configuration discovery protocol designed for local networks. No REST API, no web server, no prior knowledge of device IPs required—just send a UDP broadcast and get instant JSON responses from all your devices.

---

## Features

- 🚀 **Fast**: Single UDP broadcast discovers all devices in milliseconds
- 🔌 **Zero-config**: No mDNS, no Bonjour, no complex setup
- 📦 **Tiny footprint**: Minimal memory and code size
- 🌐 **Cross-platform**: Works on Arduino, ESPHome, Tasmota, Python
- 📡 **Standards-ready**: Fixed port (11867), Fixed protocol identifier (`ehdp/1.0`)
- 🧩 **Extensible**: Material symbols, capabilities, and metadata support

---

## Quick Start

### Discover Devices (Python)

```bash
cd python
python scanner.py
```

```json
[
  {
    "proto": "ehdp/1.0",
    "ip": "192.168.1.50",
    "material_symbol": "0xe03e",
    "project": "ehRadio",
    "firmware": "ESP32S3",
    "name": "Kitchen Radio",
    "ui_port": 80
  }
]
```

---

## Supported Platforms

### 🔷 ESPHome Component

Fully automatic metadata extraction—no manual configuration needed.

```yaml
external_components:
  - source: github://trip5/ehDP/components

ehdp:
  material_symbol: "0xe03e"  # Radio symbol (optional)
```

**Auto-detects**: `friendly_name`, `project.version`, ESPHome version, mDNS hostname, `web_server` port

📖 [ESPHome Documentation](components/ehdp/)

---

### 🔶 Arduino Library

Compatible with ESP32 and ESP8266 via PlatformIO or Arduino IDE.

```cpp
#include <ehDP.h>

EhDP ehdp;

void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  ehdp.setProject("MyProject");
  ehdp.setMaterialSymbol("0xe03e");  // Optional: Radio symbol
  ehdp.setUIPort(80);
  ehdp.begin();
}

void loop() {
  ehdp.loop();
}
```

📖 [Arduino Examples](examples/)

---

### 🟧 Tasmota Berry Script

Drop-in Berry script for Tasmota devices with persistent configuration.

**Installation**: Upload `ehdp.be` via Tasmota's web UI.

```
EhDPIcon 0xe0f0
```

📖 [Tasmota Documentation](Tasmota-Berry/)

---

## Protocol Overview

| Property    | Value                              |
|-------------|------------------------------------|
| Protocol    | UDP                                |
| Port        | `11867` (Fixed)                    |
| Request     | `EHDP_DISCOVER` (ASCII)            |
| Response    | JSON (UTF-8, <512 bytes)           |

### Minimal Response

```json
{
  "proto": "ehdp/1.0",
  "ip": "192.168.1.100"
}
```

### Full Response

```json
{
  "proto": "ehdp/1.0",
  "ip": "192.168.1.95",
  "material_symbol": "0xe192",
  "project": "VFDClock",
  "firmware": "ESP32-C3",
  "name": "Bedroom Clock",
  "version": "2026.03.18",
  "mdns": "vfdclock",
  "ui_port": 80,
  "capabilities": ["mqtt", "homeassistant"]
}
```

📖 [Full Protocol Specification](SPEC_1.md)

---

## Tools

### 🐍 Python Scanner

Zero-dependency discovery tool with JSON and human-readable output.

```bash
python scanner.py          # Standard discovery
python scanner.py -v       # Verbose with protocol status
python scanner.py -j       # JSON output
python scanner.py --all    # Promiscuous mode (show unofficial protocols)
```

📖 [Python Tool Documentation](python/)

---

## Use Cases

- **Home automation dashboards**: Auto-discover devices without manual IP entry
- **Mobile apps**: Find devices on the local network instantly
- **Diagnostic tools**: Quickly enumerate all IoT devices on a subnet
- **Provisioning systems**: Verify device presence during setup

---

## Network Topology Notes

### Single Subnet
Works perfectly on standard home networks (`192.168.1.0/24`).

### Multiple Subnets
For networks with multiple isolated subnets (e.g., VLANs), you'll need:
- **Avahi/mDNS Reflector** on the router, or
- **UDP Relay/IP Helper** configured for port `11867`

This is a standard Layer 3 limitation shared by all broadcast-based discovery protocols (mDNS, SSDP, etc.).

---

## Contributing

Contributions welcome! This project follows a "Simple over comprehensive" philosophy—PRs should maintain the lightweight, hackable nature of the protocol.

### Project Structure

```
ehDP/
├── components/ehdp/      # ESPHome component
├── examples/             # Arduino examples
├── include/              # Arduino library headers
├── src/                  # Arduino library implementation
├── python/               # Discovery scanner tool
├── tasmota/              # Tasmota Berry script
├── SPEC_1.md             # Protocol specification
└── LICENSE               # LGPL-2.1
```

---

## Roadmap

- ✅ Arduino library (ESP32/ESP8266)
- ✅ ESPHome component with auto-detection
- ✅ Python scanner with promiscuous mode
- ✅ Tasmota Berry script
- 🔲 Submit IANA port registration for `11867`
- 🔲 Home Assistant integration
- 🔲 Android client app
- 🔲 Web-based discovery dashboard

---

## License

**LGPL-2.1** — Free to use, modify, and embed in your projects.

See [LICENSE](LICENSE) for details.

---

## Links

- [Protocol Specification](SPEC_1.md)
- [GitHub Repository](https://github.com/trip5/ehDP)
- [Issue Tracker](https://github.com/trip5/ehDP/issues)

---

**Made with ❤️ for the ESP community**
