# Host / unit test foundation

Future pure-logic tests (no physical hardware required) live here, for example:

- HID normalization
- report mapping
- compatibility JSON parsing
- configuration parsing
- i18x string/lookup helpers
- diagnostic JSON generation

Infrastructure smoke tests for Gulp helpers:

```bash
npm run test:infra
```

Do not invent fake Bluetooth stack tests merely to increase coverage.
Firmware-on-device and physical regression tests arrive in later milestones.
