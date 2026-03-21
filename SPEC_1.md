# ehDP — eh Discovery Protocol Specification

> Version: 1.0  
> Status: Production Candidate  
> Last Updated: 2026-03-20

---

## Overview

**ehDP** is a lightweight, UDP-based discovery protocol designed for ESP8266/ESP32 and similar IoT hardware. It enables fast, zero-configuration discovery of devices on a local network — no REST API, no web server, no prior knowledge of device IPs required.

### Design Philosophy

- **Simple over comprehensive** — easy enough for hobbyists, implementable in a few lines
- **Practical over standardized** — works in the real world, not just on paper
- **Hackable over formal** — extensible without breaking compatibility

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

1. **Scanner → Broadcast**: Send UDP packet containing `EHDP_DISCOVER` to port **11867** on broadcast addresses (255.255.255.255 and interface-specific broadcasts)
2. **Device → Response**: Devices reply with JSON payload to the sender's address
3. **Scanner**: Parse JSON responses and display discovered devices

---

## Request Packet

- **Transport:** UDP broadcast
- **Port:** `11867`
- **Payload (ASCII, exact):**

```
EHDP_DISCOVER
```

## Request Packet

- **Transport:** UDP broadcast
- **Port:** `11867`
- **Payload (ASCII, exact):**

```
EHDP_DISCOVER
```

No headers, no framing, no newline required (but tolerated).

---

## Response Packet

- **Transport:** UDP unicast (reply to sender IP)
- **Port:** Sender's port
- **Payload:** UTF-8 encoded JSON, single packet

---

## JSON Response Format

### Required Fields

| Field   | Type   | Description                                     | Example |
|---------|--------|-------------------------------------------------|---------|
| `proto` | string | Protocol identifier and version (must start with `ehdp/`) | `"ehdp/1.0"` |
| `ip`    | string | Device's current IPv4 address                   | `"192.168.1.100"` |

### Optional Fields

| Field | Type | Description | Example |
|-------|------|-------------|---------|
| `material_symbol` | integer or string | [Material symbol](https://fonts.google.com/icons) codepoint - accepts decimal integer, hex string, or decimal string | `58287`, `"0xe3af"`, `"58287"` |
| `project` | string | Project or software name | `"ehRadio"`, `"WLED"` |
| `firmware` | string | Firmware variant or build name | `"ESP32S3OLED"` |
| `name` | string | User-assigned device label | `"Kitchen Radio"` |
| `version` | string or number | Firmware/software version - accepts string or numeric format | `"2.1.0"`, `2.1`, `"1.0"` |
| `mdns` | string | mDNS hostname (without .local suffix) | `"mydevice"` |
| `ui_port` | integer or string | HTTP port for web UI - accepts integer or numeric string (0 or omit if no web UI) | `80`, `"80"`, `8080`, `"8080"` |
| `capabilities` | array[string] | Additional capability tags | `["mqtt", "homeassistant"]` |

---

## Display Priority

### Device Icon
1. If `material_symbol` is specified → Use the Material symbol at that codepoint
2. Else → Use generic device icon (codepoint `0xe6de` / `59102`)

### Device Name Display
Display uses the first available field:
1. `name` (user label) - **Bold**
2. `project` (software name) - Bold if name not present
3. `firmware` (build variant) - Bold if name and project not present
4. Show all three fields if available, with only the top priority one bolded

---

## Flexible Field Types

The scanner accepts multiple data types for numeric fields to accommodate different device implementations and JSON libraries:

**Fields Accepting Multiple Types:**
- **`material_symbol`**: Integer, hex string (`"0xe3af"`), or decimal string (`"58287"`)
- **`ui_port`**: Integer (`80`) or numeric string (`"80"`)
- **`version`**: String (`"1.0.0"`) or number (`1.0`)

This flexibility allows devices to send fields in the most convenient format for their programming environment without requiring type conversion.

---

## Material Symbol Codepoints

The `material_symbol` field accepts the Unicode codepoint of any [Material Symbol](https://fonts.google.com/icons) in multiple formats:

**Supported Formats:**
1. **Decimal integer**: `58287`
2. **Hex integer** (in JSON): `0xe3af` (Note: Not all JSON parsers support this)
3. **Hex string**: `"0xe3af"` or `"0xE3AF"` (Recommended for compatibility)
4. **Decimal string**: `"58287"`

**Finding Symbol Codepoints:**
1. Visit [Material Symbols](https://fonts.google.com/icons)
2. Search for your desired symbol
3. Click on the symbol to see details
4. The codepoint is shown (e.g., `e3af` for camera)
5. Send as decimal `58287`, hex string `"0xe3af"`, or hex integer `0xe3af`

**Recommendation:** Use hex strings (`"0xe3af"`) for maximum compatibility across all JSON parsers and programming languages.

**Common Icon Codepoints:**
- **Camera**: `0xe3af` / `58287`
- **Lightbulb**: `0xe0f0` / `57584`
- **Speaker**: `0xe32d` / `58157`
- **Access Time (Clock)**: `0xe192` / `57746`
- **Thermostat**: `0xf076` / `61558`
- **Sensors**: `0xe51e` / `58654`
- **Radio**: `0xe03e` / `57406`
- **Power**: `0xe63c` / `58940`
- **TV**: `0xe333` / `58163`

The protocol supports **all** Material Symbols without any limitations.

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
  "material_symbol": "0xe03e",
  "project": "ehRadio",
  "firmware": "ESP32S3OLED",
  "name": "Kitchen Radio",
  "version": "2.1.0",
  "mdns": "kitchen-radio",
  "ui_port": 80,
  "capabilities": ["mqtt", "homeassistant"]
}
```

### Clock with Custom Icon (Decimal Format)
```json
{
  "proto": "ehdp/1.0",
  "ip": "192.168.1.50",
  "material_symbol": 57746,
  "project": "VFD16-Clock-HA",
  "name": "Desk Clock",
  "version": "1.0.0",
  "ui_port": 80
}
```

### Device with Flexible Types (Strings as Numbers)
```json
{
  "proto": "ehdp/1.0",
  "ip": "192.168.1.75",
  "material_symbol": "0xe0f0",
  "name": "Smart Bulb",
  "version": 2.5,
  "ui_port": "8080"
}
```

---

## Network Requirements

- UDP port **11867** must be open for discovery broadcasts
- Devices should respond within **3 seconds** of receiving `EHDP_DISCOVER`
- If a web UI is provided, HTTP server must be accessible on the specified `ui_port`

---

## Protocol Version

Current version: **ehdp/1.0**

The `proto` field must start with `ehdp/` followed by the version number. Future versions may extend fields while maintaining backward compatibility.

---

## Protocol Governance

The `ehdp/X` version identifier is **reserved** for protocol versions ratified by this project (https://github.com/trip5/ehDP).

- New official versions require approval by the project maintainer.
- Forks or extensions that are not ratified **must not** use the `ehdp/X` identifier.
- Unofficial variants should use a distinct version string, e.g. `myfork/1` or `com.mycompany.ehdp/1`.
- This ensures that a device advertising `proto: "ehdp/1"` is unambiguously conformant with this specification.

This governance applies to the **protocol identifier only**. The library source code is licensed under the GNU Lesser General Public License v2.1 (LGPL-2.1) — see `LICENSE`.

---

## Implementation Notes

### For ESP Devices (Arduino/ESP-IDF)

- Listen on UDP port `7373` in a loop or task
- On receipt of a packet matching `EHDP_DISCOVER`:
  - Build a JSON string (ArduinoJson recommended)
  - Send it as a UDP unicast reply to the source IP/port
- Keep JSON under 512 bytes to stay within a single UDP packet
- Do not respond to malformed or unknown payloads
- Optionally compare against a payload prefix for forward compatibility

### For Clients (Android, Desktop, etc.)

- Determine the subnet broadcast address (e.g., `192.168.1.255`)
- Send `EHDP_DISCOVER` as a UDP broadcast on port `7373`
- Listen for UDP replies for 1–3 seconds
- Parse each reply as JSON
- Validate `proto` field starts with `ehdp/`
- Display or act on discovered devices

---

## Security Considerations

- ehDP is designed for **trusted local networks only**
- No authentication is defined in v1 (by design — simplicity)
- Do **not** expose port `7373` on the internet or via NAT
- Optional future versions may add challenge/response pairing
- Clients should validate JSON before using any field values

---

## Comparison to Existing Protocols

| Protocol        | Transport     | Payload | Complexity | ESP friendly |
|-----------------|---------------|---------|------------|--------------|
| **ehDP**        | UDP broadcast | JSON    | Very low   | Yes          |
| mDNS / DNS-SD   | UDP multicast | DNS RR  | Medium     | Possible     |
| SSDP (UPnP)     | UDP multicast | XML     | High       | Difficult    |
| Tasmota autodis | REST/HTTP     | JSON    | Medium     | Limited      |
| Subnet scan     | TCP/ICMP      | varies  | High       | N/A          |

---

## Reference Implementations

- [x] **ESP32 / ESP8266 (Arduino library)** — `include/` + `src/` — C++ library with `begin()` / `handle()` API
  - Example: `examples/BasicResponder/` — minimal generic device
  - Example: `examples/RadioResponder/` — full-featured audio radio device
- [x] **ESPHome external component** — `components/ehdp/` — drop-in YAML component, no ArduinoJson required
  - Example: `examples/EhDPComponent/ehdp_example.yaml`
  - Load via `external_components` from this repo; works with Arduino framework on ESP32 and ESP8266
- [ ] **Android scanner** — Kotlin UDP broadcast client with device list UI
- [x] **Python scanner** — `python/scanner.py` — CLI discovery tool with `--json` output
- [ ] **Node.js scanner** — for desktop/server tooling

---

## Changelog

| Version | Date       | Notes                        |
|---------|------------|------------------------------|
| 1.0     | 2026-03-19 | Initial draft specification  |
