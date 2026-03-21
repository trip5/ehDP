# ehDP (eh Discovery Protocol) Library

[![License: LGPL-2.1](https://img.shields.io/badge/License-LGPL%202.1-blue.svg)](LICENSE)

## Overview

The ehDP Library is a simple and lightweight discovery protocol designed for local networks. Just send a UDP broadcast and get instant JSON responses from all your ehDP-capable devices.

## Why?

Many ESP-based devices include a Web UI dashboard but without Home Assistant integration, it becomes tedious to type type in it's IP address into a mobile device's browser.

This library, combined with an app like [eh Device Scanner](https://) simplifies this.  All devices listed and accessible with a single touch.

### Features

- Fast: Single UDP broadcast discovers all devices in milliseconds
- Zero-config: No mDNS, no Bonjour, no complex setup
- Minimal memory and code size
- Works on Arduino, ESPHome, Tasmota (with Berry), Python
- Fixed port (11867), Fixed protocol identifier (`ehdp/1.0`)
- Extensible but backwards and forwards-compatible

### PlatformIO / Arduino

Your platformio.ini should contain:
```
lib_deps =
  trip5/ehDP@^1.0.0
```

---

## Usage

To use the ehDP library in your project, include the header file, configure ehDP in your setup, and call the `loop` function in your own `loop`. Here is a basic example:

```cpp
#include <ehDP.h>

EhDP ehdp;

void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  // Required for WebUI - omit if no WebUI is available
  ehdp.setUIPort(80);
  // At least one of project / firmware / name is recommended
  ehdp.setName("My Clock")
  ehdp.setProject("AmazingClock");
  ehdp.setFirmware("clockfirmware.bin");
  // Optional fields
  ehdp.setMaterialSymbol("0xe8b5");  // Schedule (clock face)
  ehdp.setVersion("1.00");
  ehdp.setMdns("myclock");
  ehdp.begin();
}

void loop() {
  ehdp.loop();
}
```
---

## The Process

A scanning app or device sends a broadcast message of `EHDP_DISCOVER` to `255.255.255.255:11867`.
The `ehdp.loop()` listening on UDP Port `11867` for `EHDP_DISCOVER` sends a unicast reply directly back to the sender's port and IP.

### Example Response

```json
{
  "proto": "ehdp/1.0",
  "ip": "192.168.1.101",
  "name": "My Clock",
  "project": "AmazingClock",
  "firmware": "clockfirmware.bin",
  "version": "2026.01.01",
  "mdns": "vfd-clock",
  "ui_port": 80,
  "material_symbol": "0xe8b5"
}
```

---

## Network Topology Notes

### Single Subnet

Works on standard home networks (`192.168.1.0/24`).

### Multiple Subnets

For networks with multiple isolated subnets (e.g., VLANs), you'll need either a Avahi/mDNS Reflector on the router or a UDP Relay/IP Helper configured for port `11867`.

This is a standard Layer 3 limitation shared by all broadcast-based discovery protocols (mDNS, SSDP, etc.).

---

## Other Supported Platforms

### ESPHome Component

Fully automatic metadata extraction—no manual configuration needed (except an icon).

```yaml
external_components:
  - source: github://trip5/ehDP/components

ehdp:
  material_symbol: "0xe03e"  # Radio symbol (optional)
```

| Response JSON | extracted from `esphome:` block key |
| ------------- |------------------------------------ |
| `name`        | `friendly_name:`, fallback to `name:` |
| `project`     | `project: name:` |
| `version`     | `project: version:` |
| `mdns`        | `name:` |

The response field `ui_port` is assumed to be `80` and `firmware` is directly derived from the ESPHome version used in to build the firmware.


```yaml
esphome:
  name: "${name}"
  friendly_name: "${friendly_name}"
  project:
    name: "${project_name}"
    version: "${project_version}"
```

Please see the `yml` file for more notes and on how to over-ride default derived fields.

---

### Tasmota Berry Script

Drop-in Berry script for Tasmota devices with persistent configuration.  Untested.

Upload `ehdp.be` via Tasmota's web UI?

```
EhDPIcon 0xe0f0
```

---

## Tools

### Python Scanner

Zero-dependency discovery tool with JSON and human-readable output.

```bash
python scanner.py          # Standard discovery
python scanner.py -v       # Verbose with protocol status
python scanner.py -j       # JSON output
python scanner.py --all    # Promiscuous mode (show unofficial protocols)
```

---

## Examples

Check the `examples` folder for examples of how to add the ehDP library to your project.

---

## Links

- [Protocol Specification](https://github.com/trip5/ehDP/blob/main/SPEC.md)
- [GitHub Repository](https://github.com/trip5/ehDP)

---

## Update History

| Date       | Version | Release Notes             |
| ---------- | ------- |-------------------------- |
| 2026.03.21 | 1.0.0   | First release of specification, libraries, ESPHome component, Tasmota-Berry script (untested) |

---

## License

LGPL-2.1 — Free to use, modify, and embed in your projects.

See [LICENSE](LICENSE) for details.