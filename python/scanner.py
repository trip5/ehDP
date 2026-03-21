#!/usr/bin/env python3
"""
ehDP Scanner
Discovers ehDP-compatible devices on the local network.

Usage:
    python scanner.py
    python scanner.py --all
    python scanner.py --broadcast 192.168.1.255
    python scanner.py --timeout 3 --port 11867 --json
"""

import argparse
import json
import socket
import time

EHDP_PORT    = 11867
EHDP_REQUEST = b"EHDP_DISCOVER"
DEFAULT_TIMEOUT = 2.0  # seconds


def get_broadcast_address() -> str:
    """Auto-detect the local subnet broadcast address (assumes /24)."""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        local_ip = s.getsockname()[0]
        s.close()
        prefix = local_ip.rsplit(".", 1)[0]
        return prefix + ".255"
    except OSError:
        return "255.255.255.255"


def scan(broadcast_addr: str = None, timeout: float = DEFAULT_TIMEOUT, port: int = EHDP_PORT, show_all: bool = False) -> list:
    """
    Send EHDP_DISCOVER and collect all JSON responses within `timeout` seconds.
    Returns a list of parsed device dicts.
    """
    if broadcast_addr is None:
        broadcast_addr = get_broadcast_address()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.settimeout(0.1)
    sock.bind(("", 0))

    sock.sendto(EHDP_REQUEST, (broadcast_addr, port))

    devices = []
    seen_ips = set()
    deadline = time.monotonic() + timeout

    while time.monotonic() < deadline:
        try:
            data, addr = sock.recvfrom(1024)
        except socket.timeout:
            continue

        try:
            device = json.loads(data.decode("utf-8"))
        except (json.JSONDecodeError, UnicodeDecodeError):
            continue

        proto = device.get("proto", "")
        # Filter: Must start with ehdp UNLESS show_all is True
        if not show_all:
            if not isinstance(proto, str) or not proto.startswith("ehdp"):
                continue

        # Deduplicate by source IP in case of duplicate replies
        source_ip = addr[0]
        if source_ip in seen_ips:
            continue
        seen_ips.add(source_ip)

        device["_source_ip"] = source_ip
        devices.append(device)

    sock.close()
    return devices


def print_devices(devices: list) -> None:
    if not devices:
        print("No ehDP devices found.")
        return

    print(f"Found {len(devices)} device(s):\n")
    for d in devices:
        proto = d.get("proto", "unknown")
        if str(proto).startswith("ehdp"):
            print(f"  Protocol: {proto}")
        else:
            print(f"  Unofficial Protocol: {proto}")
        if d.get("name"):
            print(f"  Name    : {d['name']}")
        if d.get("project"):
            print(f"  Project : {d['project']}")
        if d.get("firmware"):
            print(f"  Firmware: {d['firmware']}")
        if d.get("material_symbol"):
            print(f"  Symbol  : {d['material_symbol']}")
        print(f"  IP      : {d.get('ip', d.get('_source_ip', '?'))}")
        if d.get("version"):
            print(f"  Version : {d['version']}")
        if d.get("mdns"):
            print(f"  mDNS    : {d['mdns']}")
        if d.get("ui_port"):
            host = d.get("mdns") or d.get("ip", d.get("_source_ip", "?"))
            port = d["ui_port"]
            url = f"http://{host}/" if port == 80 else f"http://{host}:{port}/"
            print(f"  UI      : {url}")
        if d.get("capabilities"):
            print(f"  Caps    : {', '.join(d['capabilities'])}")
        print()


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Scan for ehDP devices on the local network."
    )
    parser.add_argument(
        "--broadcast", "-b",
        default=None,
        help="Broadcast address to use (default: auto-detect)",
    )
    parser.add_argument(
        "--timeout", "-t",
        type=float,
        default=DEFAULT_TIMEOUT,
        help=f"Discovery window in seconds (default: {DEFAULT_TIMEOUT})",
    )
    parser.add_argument(
        "--port", "-p",
        type=int,
        default=EHDP_PORT,
        help=f"UDP port (default: {EHDP_PORT})",
    )
    parser.add_argument(
        "--json", "-j",
        action="store_true",
        help="Output raw JSON array instead of human-readable text",
    )
    parser.add_argument(
        "--all", "-a",
        action="store_true",
        help="Promiscuous mode: Show all JSON responses regardless of 'proto' field",
    )
    args = parser.parse_args()

    broadcast = args.broadcast or get_broadcast_address()
    print(f"Scanning on {broadcast}:{args.port} (timeout: {args.timeout}s)...")

    devices = scan(broadcast_addr=broadcast, timeout=args.timeout, port=args.port, show_all=args.all)

    if args.json:
        # Strip internal _source_ip before output
        clean = [{k: v for k, v in d.items() if not k.startswith("_")} for d in devices]
        print(json.dumps(clean, indent=2))
    else:
        print()
        print_devices(devices)


if __name__ == "__main__":
    main()
