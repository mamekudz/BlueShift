# Protocol V1 golden vectors

**Status:** HOST-TESTABLE interoperability contract  
**Physical:** UNVERIFIED  

Neutral fixtures for BlueShift **and** ESP][. Do **not** require BlueShift C++ to interpret these files.

## Format

Each `golden_*.json` may contain:

| Field | Meaning |
| --- | --- |
| `name` | stable id |
| `protocolVersion` | must be `1` |
| `description` | human note |
| `input` | edges / control / sync intent |
| `wireHex` | **exact** little-endian wire bytes (space-separated) |
| `expect` | decode outcomes / timeline / PCM properties |
| `malformed` | if true, decode must fail |

`wireHex` is authoritative for serialization tests.  
Do not only round-trip encode→decode against the same buggy encoder.

## UUID freeze

Canonical definitions: `components/protocol/extension_protocol.h`  
Documented: `docs/protocol/esp2-extension.md`  
**Status: STABLE** for Protocol V1 — do not change casually.
