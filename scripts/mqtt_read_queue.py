#!/usr/bin/env python3

import argparse
import shutil
import subprocess
import sys


DEFAULT_HOST = "broker.mqttdashboard.com"
DEFAULT_PORT = 1883
DEFAULT_USERNAME = "test"
DEFAULT_PASSWORD = "test"
DEFAULT_TOPIC = "yai-mqtt/YUS-0.2.8-COSTA/out"


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        description=(
            "Escucha un topic MQTT usando mosquitto_sub. "
            "Por defecto usa los valores del skill mqtt-read-queue-skill.md."
        )
    )
    p.add_argument("--host", default=DEFAULT_HOST, help=f"Broker host (default: {DEFAULT_HOST})")
    p.add_argument("--port", type=int, default=DEFAULT_PORT, help=f"Broker port (default: {DEFAULT_PORT})")
    p.add_argument("--username", "-u", default=DEFAULT_USERNAME, help=f"Username (default: {DEFAULT_USERNAME})")
    p.add_argument("--password", "-P", default=DEFAULT_PASSWORD, help=f"Password (default: {DEFAULT_PASSWORD})")
    p.add_argument("--topic", "-t", default=DEFAULT_TOPIC, help=f"Topic (default: {DEFAULT_TOPIC})")
    p.add_argument(
        "--once",
        action="store_true",
        help="Salir tras recibir 1 mensaje (usa mosquitto_sub -C 1).",
    )
    p.add_argument(
        "--check",
        action="store_true",
        help="Solo verifica que mosquitto_sub está disponible (equivalente a 'mosquitto_sub --help').",
    )
    return p


def ensure_mosquitto_sub() -> str:
    path = shutil.which("mosquitto_sub")
    if path:
        return path
    print("Error: no encuentro 'mosquitto_sub' en tu PATH.", file=sys.stderr)
    print("En macOS con Homebrew suele instalarse con: brew install mosquitto", file=sys.stderr)
    return ""


def main() -> int:
    args = build_parser().parse_args()

    mosquitto_sub = ensure_mosquitto_sub()
    if not mosquitto_sub:
        return 127

    if args.check:
        # mosquitto_sub suele devolver exit code 1 con --help, pero eso no es un fallo real.
        res = subprocess.run([mosquitto_sub, "--help"])
        return 0 if res.returncode in (0, 1) else res.returncode

    cmd = [
        mosquitto_sub,
        "-h",
        args.host,
        "-p",
        str(args.port),
        "-u",
        args.username,
        "-P",
        args.password,
        "-t",
        args.topic,
    ]
    if args.once:
        cmd.extend(["-C", "1"])

    try:
        # Dejamos que mosquitto_sub maneje stdout/stderr directamente (streaming).
        return subprocess.call(cmd)
    except KeyboardInterrupt:
        return 130


if __name__ == "__main__":
    raise SystemExit(main())

