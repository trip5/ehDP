# ehDP — eh Discovery Protocol Specification

> Version: 1.0  

---

## Overview

The ehDP Library is a simple and lightweight discovery protocol designed for local networks. Just send a UDP broadcast and get instant JSON responses from all your ehDP-capable devices.
Using [ehDP Scanner](https:// play store link), a user can use a single app to easily access all devices with WebUIs on their local network.

---

## Transport

| Property    | Value                              |
|-------------|------------------------------------|
| Protocol    | UDP                                |
| Port        | `11867` (Fixed by convention)      |
| Direction   | Client → Broadcast, Device → Unicast reply |
| Packet size | Request: <32 bytes, Response: <512 bytes recommended |

---

## Discovery Process

1. Scanner → Broadcast**: Send UDP packet containing `EHDP_DISCOVER` to port 11867 on broadcast addresses (255.255.255.255 and interface-specific broadcasts)
2. Device → Response**: Devices reply with JSON payload to the sender's address
3. Scanner: Parse JSON responses and display discovered devices

---

## Request Packet

- Transport: UDP broadcast
- Port: `11867`
- Payload (ASCII, exact):

```
EHDP_DISCOVER
```

---

## Response Packet

- Transport: UDP unicast (reply to sender IP)
- Port: Sender's port
- Payload: UTF-8 encoded JSON, single packet

---

## JSON Response Format

No headers, no framing, no newline required (but tolerated).

### Required Fields

| Field   | Type   | Description                                               | Version | Example |
|---------|--------|-----------------------------------------------------------|---------|---------|
| `proto` | string | Protocol identifier and version (must start with `ehdp/`) | 1.0     | `"ehdp/1.0"` |
| `ip`    | string | Device's current IPv4 address                             | 1.0     | `"192.168.1.100"` |

### Optional Fields

| Field             | Type              | Description                                                                                        | ehDP Version | Example |
|-------------------|-------------------|-----------------------------------------------------------------------------------------------------------|-------|---------|
| `name`            | string            | User-assigned device label                                                                                | 1.0   | `"Kitchen Radio"` |
| `project`         | string            | Project or software name                                                                                  | 1.0   | `"ehRadio"`, `"WLED"` |
| `firmware`        | string            | Firmware variant or build name                                                                            | 1.0   | `"ESP32S3OLED"` |
| `version`         | string or integer | Firmware/software version                                                                                 | 1.0   | `"2.1.0"`, `2.1`, `"1.0"` |
| `mdns`            | string            | mDNS hostname (without .local suffix)                                                                     | 1.0   | `"mydevice"` |
| `ui_port`         | integer or string | HTTP port for web UI - accepts integer or numeric string (0 or omit if no web UI)                         | 1.0   | `80`, `"80"`, `8080`, `"8080"` |
| `material_symbol` | string or integer | [Material symbol](https://fonts.google.com/icons) codepoint - decimal as string or integer, hex as string | 1.0   | `58287`, `"0xe3af"`, `"58287"` |
| `capabilities`    | array[string]     | Additional capability tags - not usefully implemented                                                     | 1.0   | `["mqtt", "homeassistant"]` |

---

## Display Priority

### Device Icon

1. If `material_symbol` is specified → Use the Material symbol at that codepoint
2. Else → Use generic device icon (codepoint `0xe6de` / `59102`)

### Device Name Display

Display uses the first available field to show prominently (but should display all fields anyway):
1. `name` (user label)
2. `project` (software name)
3. `firmware` (build variant)

---

## Material Symbol Codepoints

The `material_symbol` field accepts the Unicode codepoint of any [Material Symbol](https://fonts.google.com/icons).
It may be an integer (decimal) or string (decimal or hex).  A hex string is the recommended format for compatibility.

Finding Symbol Codepoints:
1. Visit [Material Symbols](https://fonts.google.com/icons)
2. Search for your desired symbol
3. Click on the symbol to see details
4. The codepoint is shown (e.g., `e3af` for camera)
5. Send as hex string `"0xe3af"`

Common Icon Codepoints:

| Name       | Codepoint | Name             | Codepoint | Name                 | Codepoint |
|-------------|----------|-------------------|----------|-----------------------|----------|
| Home        | `0xe88a` | Settings          | `0xe8b8` | Favorite              | `0xe87d` |
| Camera      | `0xe3af` | Camera Indoor     | `0xefe9` | Camera Outdoor        | `0xefea` |
| Lightbulb   | `0xe0f0` | Light             | `0xf02a` | Floor Lamap           | `0xe21e` |
| Speaker     | `0xe32d` | Speaker Group     | `0xe32e` | Assistant Device      | `0xe987` |
| Punch Clock | `0xeaa8` | Alarm (clock)     | `0xe855` | Schedule (clock face) | `0xe8b5` |
| Thermostat  | `0xf076` | Device Thermostat | `0xe1ff` | Thermometer           | `0xe846` |
| Detector    | `0xe282` | Humidity Indoor   | `0xf558` | Sensor Door           | `0xf1b5` |
| Radio       | `0xe03e` | TV                | `0xe333` | Cast                  | `0xe307` |
| Pinboard    | `0xf3ab` | Ad                | `0xe65a` | Preview               | `0xf1c5` |
| Power       | `0xe63c` | Bolt              | `0xea0b` | Solar Power           | `0xec0f` |

The protocol supports all Material Symbols without any limitations (although apps may have issues displaying them).

---

## Example Responses

### Minimal Response
```json
{
  "proto": "ehdp/1.0",
  "ip": "192.168.1.100"
}
```

### Full-Featured Device
```json
{
  "proto": "ehdp/1.0",
  "ip": "192.168.1.100",
  "name": "Kitchen Radio",
  "project": "ehRadio",
  "firmware": "ESP32S3OLED",
  "version": "2.1.0",
  "mdns": "kitchen-radio",
  "ui_port": 80,
  "material_symbol": "0xe03e",
  "capabilities": ["mqtt", "homeassistant"]
}
```

---

## Network Requirements

- UDP port 11867 must be open for discovery broadcasts
- Devices should respond within 3 seconds of receiving `EHDP_DISCOVER`
- If a web UI is provided, HTTP server must be accessible on the specified `ui_port`

---

## Protocol Version

Current version: `ehdp/1.0`

The `proto` field must start with `ehdp/` followed by the version number. Future versions may extend fields while maintaining backward compatibility.

---

## Protocol Governance

The `ehdp/X` version identifier is reserved for protocol versions ratified by this project (https://github.com/trip5/ehDP).

- New official versions require approval by the project maintainer.
- Forks or extensions that are not ratified must not use the `ehdp/X` identifier.
- Unofficial variants should use a distinct version string, e.g. `myfork/1` or `com.mycompany.ehdp/1`.
- This ensures that a device advertising `proto: "ehdp/1"` is unambiguously conformant with this specification.

This governance applies to the protocol identifier only. The library source code is licensed under the GNU Lesser General Public License v2.1 (LGPL-2.1) — see `LICENSE`.

---

## Security Considerations

- ehDP is designed for trusted local networks only
- No authentication is defined (by design for simplicity)
- Do not expose port `11867` on the internet or via NAT
- If included, it is recommended to include an option to disable it
- Clients should validate JSON before using any field values

---

## Reference Implementations

- ESP32 / ESP8266 (Arduino library) — `include/` + `src/` — C++ library with `begin()` / `loop()` API
- ESPHome external component — `components/ehdp/` — drop-in YAML component, no ArduinoJson required
- Android scanner — Kotlin UDP broadcast client with device list UI
- Python scanner — `python/scanner.py` — CLI discovery tool with `--json` output

---

## Changelog

| Version | Date       | Notes                        |
|---------|------------|------------------------------|
| 1.0     | 2026-03-21 | First specification          |
