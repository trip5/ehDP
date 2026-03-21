# ehDP for Tasmota

**ehDP** responder implementation for Tasmota devices using Berry scripting.

## Features

- ✅ **Zero-config auto-detection**: Automatically pulls device name, IP, firmware version, and mDNS hostname
- ✅ **Persistent configuration**: Settings survive reboots
- ✅ **Tasmota console commands**: Configure material symbol and capabilities via the web UI
- ✅ **Lightweight**: Uses only ~50ms polling cycles, minimal memory footprint

---

## Requirements

- **Tasmota firmware** with Berry scripting support (v12.1.0+)
- Most Tasmota32 builds include Berry by default
- Check with: `Berry` command in console (should not return "Unknown command")

---

## Installation

### Method 1: Web UI Upload (Easiest)

1. Open your Tasmota device's web interface
2. Navigate to **Consoles → Manage File system**
3. Click **Choose File** and upload `ehdp.be`
4. The script auto-starts on next boot, or run `Restart 1`

### Method 2: Console Paste

1. Open **Consoles → Berry Scripting console**
2. Copy and paste the entire contents of `ehdp.be`
3. To make it permanent, save it via file system upload

---

## Configuration

### Set Material Symbol (Optional)

Open the Tasmota console and run:

```
EhDPSymbol 0xe0f0
```

**Common Material symbol codepoints:**
- **Lightbulb**: `0xe0f0` / `57584`
- **Power**: `0xe63c` / `58940`
- **Switch**: `0xe8e0` / `59616`
- **Radio**: `0xe03e` / `57406`
- **TV**: `0xe333` / `58163`
- **Speaker**: `0xe32d` / `58157`
- **Clock**: `0xe192` / `57746`

See [Material Symbols](https://fonts.google.com/icons) for all available symbols.

Set symbols using hex strings like `"0xe3af"` for maximum compatibility.

### Add Capabilities (Optional)

Add extra capability tags:

```
EhDPCapability mqtt
EhDPCapability homeassistant
```

### Check Current Config

```
EhDPSymbol ?
EhDPCapability ?
```

---

## Testing

Once installed and configured, test discovery with the Python scanner:

```bash
python scanner.py
```

Your Tasmota device should appear in the results:

```json
{
  "proto": "ehdp/1.0",
  "ip": "192.168.1.100",
  "material_symbol": "0xe0f0",
  "project": "Tasmota",
  "firmware": "14.0.0",
  "name": "Living Room Light",
  "mdns": "tasmota-living",
  "ui_port": 80
}
```

---

## Troubleshooting

### Script not loading

Check the Berry console for errors:
```
BrList
```

You should see `ehdp.be` in the list.

### Not responding to discovery

1. Check if UDP socket bound successfully:
   - Look for `ehDP: Listening on port 11867` in the console logs
2. Verify WiFi is connected:
   ```
   Status 5
   ```
3. Check if icon is set:
   ```
   EhDPIcon ?
   ```

### Remove the script

To uninstall, delete the file from the filesystem:
```
Consoles → Manage File system → Delete ehdp.be
```

Then restart the device.

---

## Advanced Usage

### Custom Autostart Script

If you already have an `autoexec.be`, you can import ehDP instead:

```berry
# In autoexec.be
load("ehdp.be")
```

### Modify Response Data

Edit the `respond()` function in `ehdp.be` to customize the JSON response. For example, to change the UI port:

```berry
"ui_port": 8080
```

---

## How It Works

1. **Berry Driver**: The script registers as a Tasmota driver with a 50ms polling loop
2. **UDP Listener**: Binds to port `11867` and listens for `EHDP_DISCOVER` packets
3. **Auto-Detection**: Queries Tasmota's internal state for device info
4. **Unicast Reply**: Sends a single JSON packet back to the discovery client

---

## License

Same as parent project: [LGPL-2.1](../LICENSE)

---

## Links

- [ehDP Specification](../SPEC_1.md)
- [Python Scanner](../python/)
- [Arduino Library](../README.md)
- [ESPHome Component](../components/ehdp/)
