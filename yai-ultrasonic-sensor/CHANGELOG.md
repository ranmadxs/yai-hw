# Changelog

All notable changes to yai-ultrasonic-sensor will be documented in this file.

## [1.0.6-YUS-COSTA] - 2025-03-11

### Changed

- **Header HTTP:** Reemplazo de `aia_origin` por `X-Aia-Origin` en todos los requests a tomi-metric-collector (forzar-guardado y batch de lecturas).
- Soporte ESP8266 (NodeMCU v2): YaiHttpClient con includes condicionales, BearSSL para HTTPS, envío síncrono.
- JSON de lecturas incluye `channelId`.
- Offset del sensor configurable (`SENSOR_HEIGHT_OFFSET_CM`).
- Versión definida en `platformio.ini` como única fuente de verdad.

### Added

- Biblioteca `YaiHttpClient` para envío batch HTTP y forzar-guardado.
- Especificación API forzar-guardado en `specs/API-FORZAR-GUARDADO-SPEC.md`.
