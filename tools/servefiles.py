#!/usr/bin/env python3
"""Send CIA/3DSX URLs to FBI (Remote install -> Receive URLs over the network)."""

import os
import socket
import struct
import sys
import threading
import time
from http.server import SimpleHTTPRequestHandler
from socketserver import TCPServer
from urllib.parse import quote

ACCEPTED = (".cia", ".tik", ".cetk", ".3dsx")


def detect_host_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 53))
        return s.getsockname()[0]
    finally:
        s.close()


def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <3ds-ip> <file-or-dir> [host-ip] [host-port]")
        sys.exit(1)

    target_ip = sys.argv[1]
    target_path = sys.argv[2].strip()
    host_ip = sys.argv[3] if len(sys.argv) >= 4 else detect_host_ip()
    host_port = int(sys.argv[4]) if len(sys.argv) >= 5 else 8080

    if not os.path.exists(target_path):
        print(f"{target_path}: not found")
        sys.exit(1)

    base_url = f"{host_ip}:{host_port}/"
    if os.path.isfile(target_path):
        if not target_path.endswith(ACCEPTED):
            print(f"Unsupported extension. Use one of: {ACCEPTED}")
            sys.exit(1)
        directory = os.path.dirname(target_path) or "."
        payload = base_url + quote(os.path.basename(target_path))
    else:
        directory = target_path
        files = [f for f in os.listdir(target_path) if f.endswith(ACCEPTED)]
        if not files:
            print("No supported files in directory")
            sys.exit(1)
        payload = "".join(base_url + quote(f) + "\n" for f in files)

    payload_bytes = payload.encode("ascii")
    os.chdir(directory)

    print("URLs:")
    print(payload)
    print()

    class ReuseServer(TCPServer):
        allow_reuse_address = True

    print(f"HTTP server on 0.0.0.0:{host_port}")
    server = ReuseServer(("", host_port), SimpleHTTPRequestHandler)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()

    try:
        print(f"Sending URL(s) to {target_ip}:5000 ...")
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(30)
        sock.connect((target_ip, 5000))
        sock.sendall(struct.pack("!L", len(payload_bytes)) + payload_bytes)
        while len(sock.recv(1)) < 1:
            time.sleep(0.05)
        sock.close()
        print("Done. FBI should prompt to install.")
    except Exception as exc:
        print(f"Failed: {exc}")
        print("Make sure FBI is open: Remote install -> Receive URLs over the network")
        sys.exit(1)
    finally:
        server.shutdown()


if __name__ == "__main__":
    main()
