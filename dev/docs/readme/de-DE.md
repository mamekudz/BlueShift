# BlueShift

<p align="center">
  <img src="docs/assets/blueshift-logo.png" alt="BlueShift" width="420">
</p>

<p align="center">
  <a href="https://microgulp.dev/de/ready/">
    <img src="docs/assets/microgulp-ready.png" alt="µGulp Ready" width="110">
  </a>
</p>

**Bringing Classic Bluetooth into the BLE era.**

**BlueShift** ist ein unabhängiges Open-Source-Projekt: eine Bridge von **Classic Bluetooth HID** (BR/EDR) nach **BLE HID**, zunächst vorgesehen für das Board **LILYGO T-Lion**.

Paket-/Repo-Identifier (ASCII): `blueshift` — sichtbarer Projektname bleibt **BlueShift**.

**Status:** Milestone 1 — Repository-Foundation. Firmware-Funktionen (Classic BT, BLE HID, Bridge, OLED, Akku) sind **noch nicht** implementiert. Physische Hardware ist **noch nicht** verfügbar.

Dieses Repository ist **µGulp-ready** (Gulp-Tasks für Dokumentation, Formatierung und Backup; siehe [Entwicklung](#entwicklung)).

---

## Zweck

Viele HID-Geräte (Gamepads, Tastaturen) sprechen nur **Bluetooth Classic**. Moderne Hosts und ESP32-S3-Projekte wie **ESP][** arbeiten oft nur zuverlässig mit **BLE**. BlueShift soll Classic-Eingaben annehmen, normalisieren und als BLE-HID an einen Host weitergeben.

---

## Warum Classic → BLE?

| Problem | Folge |
| --- | --- |
| Gerät nur Classic BR/EDR | Erscheint nicht in ESP32-S3-BLE-Scans |
| Host nur BLE-HID | Classic-Controller nicht nutzbar |
| Hersteller-Namen unzuverlässig | Profile brauchen Descriptor/VID/PID-Logik |

BlueShift adressiert genau diese Lücke — unabhängig davon, ob der erste Abnehmer ESP][ ist.

---

## Status-Stufen (Dokumentation)

| Stufe | Bedeutung |
| --- | --- |
| **VERIFIED** | Am physischen BlueShift-Gerät nachvollzogen |
| **EXPERIMENTAL** | Vorhanden, Verifikation unvollständig |
| **PLANNED** | Architektur/Ziel, noch nicht gebaut |
| **INCOMPATIBLE** | Mit belegter Begründung ausgeschlossen |
| **UNVERIFIED** | Hersteller-/Rechercheangabe, Board noch nicht da |

Solange das T-Lion nicht physisch getestet ist, dürfen OLED, Akku, Classic BT, BLE und PSRAM **nicht** als VERIFIED gelten.

---

## Vorgesehene Hardware (T-Lion)

Zielplattform (alles **UNVERIFIED** bis Bring-up):

| | |
| --- | --- |
| Board | LILYGO T-Lion (T-Controller) |
| MCU | classic ESP32 / ESP32-WROVER |
| Display | OLED (Controller/Pins unbestätigt) |
| Strom | 18650-Halter |
| Build | **PlatformIO** |

Siehe `docs/hardware/t-lion.md`. Es gibt **keine** offizielle PlatformIO-Board-ID `lilygo-t-lion`. Milestone 1 nutzt `esp32dev` nur als **provisorischen Compile-Smoke**-Target (`skeleton` / `debug` / `release`) — **kein** Pinout-Claim.

---

## Architektur (PLANNED)

```
Classic HID Report
      |
  Device Parser / Profile
      |
 Normalized HID State
      |
  Output Mapper
      |
  BLE HID Report
```

Schichten bleiben getrennt (Raw → Normalized → Mapped), damit Kompatibilitätsfixes die Transportschicht nicht vermischen.

### Classic HID Input — PLANNED

Klassische BR/EDR-HID-Geräte als Eingang. Noch nicht implementiert. Keine Bluepad32/BTstack-Abhängigkeit in diesem Milestone.

### Normalized HID Layer — PLANNED

Geräteprofile (z. B. SN30 Pro, Nimbus) liefern normalisierten Zustand. Keine `if (name == …)`-Verzweigungen in der Bridge-Kernlogik.

### BLE HID Output — PLANNED

BlueShift erscheint dem Host als BLE-HID-Gerät. Nicht gestartet.

### OLED — PLANNED / UNVERIFIED

Status, Pairing-Hinweise, Diagnose. Texte über **i18x** (`components/i18n/`). Das aktuelle Vollfarben-Logo ist **nicht** das spätere monochrome OLED-Logo.

### Battery — PLANNED / UNVERIFIED

18650-Überwachung nach physischer Pin-/ADC-Verifikation.

### Geplante Geräteklassen — PLANNED

Gamepads zuerst; später Tastaturen und weitere HID-Klassen, sobald die Bridge stabil ist.

### Diagnostics — PLANNED

Strukturierte Kategorien (`[BOOT]`, `[BT-CLASSIC]`, `[BLE]`, `[BRIDGE]`, …). Keine Secrets in Logs.

---

## Kompatibilitätsdatenbank

Eine strukturierte Quelle: [`docs/compatibility/devices.json`](docs/compatibility/devices.json).

Generated from [`docs/compatibility/devices.json`](docs/compatibility/devices.json). Do not edit this table by hand.

**BlueShift** = Classic HID input → BLE HID output bridge candidacy. **ESP][** = direct wireless use on the ESP32-S3 ESP][ project. These are different questions.

| Device | Transport | BlueShift | ESP][ direct | Physical test | Notes |
| --- | --- | --- | --- | --- | --- |
| 8BitDo SN30 Pro | classic-br-edr | CANDIDATE | INCOMPATIBLE | NONE | Do not claim BlueShift VERIFIED until Classic HID path is physically tested on T-Lion. |
| SteelSeries Nimbus 69070 | classic-br-edr | CANDIDATE | INCOMPATIBLE | PARTIAL | Classic-path feasibility for BlueShift is PLANNED / CANDIDATE — not yet verified on T-Lion. |
| Microsoft Xbox Controller 1697 | proprietary-wireless, usb | USB_ONLY | INCOMPATIBLE | NONE | USB-oriented; out of scope for Classic→BLE bridge without contrary evidence. |
| Terios / ShanWan T3 BM-769 | ble | NOT_APPLICABLE | CANDIDATE | PENDING | Ordered / physical verification pending for ESP][. Not a primary BlueShift Classic-input target. |
| VR PARK Controller | ble | NOT_APPLICABLE | CANDIDATE | PARTIAL | ESP][ path: BLE discovered. BlueShift Classic bridge is not the primary interest for this device. |

### Evidence / references

- bluepad32SupportedGamepads: https://bluepad32.readthedocs.io/en/latest/supported_gamepads/
- bluepad32Issue154: https://github.com/ricardoquesada/bluepad32/issues/154
- esp32KeyBridge: https://github.com/tanakamasayuki/ESP32KeyBridge
- **8BitDo SN30 Pro**: Classic Bluetooth / BR/EDR gamepad; candidate future BlueShift Classic HID input. Not suitable for direct ESP32-S3 BLE wireless use on ESP][. ([link](https://bluepad32.readthedocs.io/en/latest/supported_gamepads/))
- **SteelSeries Nimbus**: Physically tested against ESP][ BLE scanner: visible to Windows Bluetooth, NOT visible in ESP32-S3 BLE scan. Therefore direct ESP][ BLE wireless: INCOMPATIBLE. Candidate BlueShift Classic input.
- **Microsoft Xbox Controller**: No normal Bluetooth Classic/BLE HID for wireless use. Do not classify as BlueShift Classic HID candidate without new evidence.
- **Terios / ShanWan T3**: Inexpensive BLE controller discussion relevant to direct ESP][ use (not primary BlueShift Classic-input target). ([link](https://github.com/ricardoquesada/bluepad32/issues/154))
- **VR PARK Controller**: Physically visible in ESP32-S3 BLE scan on ESP][. Deeper HID/GATT analysis pending. Direct ESP][ candidate.


---

## Entwicklung

### PlatformIO

```text
platformio.ini   — skeleton / debug / release (provisional esp32dev)
src/main.cpp     — Boot-Skeleton
include/blueshift/version.h  — 0.1.0-dev
```

Gepinnt: `espressif32 @ 6.9.0`. Keine Bluetooth-Libraries in Milestone 1.

Wenn PlatformIO installiert ist:

```bash
pio run -e skeleton
```

Ein erfolgreicher Skeleton-Build beweist nur die Toolchain — **nicht** T-Lion-Hardware.

### µGulp / Gulp

```bash
npm install
npx gulp help          # Default — sicher
npx gulp docs          # README aus de-DE.src.md + Kompatibilitätstabelle
npx gulp format        # clang-format (falls installiert)
npx gulp format:check
npx gulp check
npx gulp backup:git    # expliziter Checkpoint (Dry-Run: BLUESHIFT_BACKUP_GIT_DRY_RUN=1)
npx gulp backup        # NAS 0–3 Ziele
npx gulp backup:nas    # Alias
npx gulp backup:all
```

Default/`help` baut **keine** Firmware, commitet nichts und greift nicht auf NAS zu.

NAS-Ziele: `NAS_TARGET_1` … `NAS_TARGET_3` oder `config/nas.targets.example` → `nas.targets.local` (gitignored). Null Ziele sind gültig.

### Formatierung

`.clang-format` folgt den Projektregeln (4 Spaces, Brace Attach). Tasks `format` / `format:check` brauchen `clang-format` auf dem PATH oder `CLANG_FORMAT`. Fehlt das Tool, melden die Tasks das klar — das Projekt hängt nicht an einem undokumentierten Global-Install.

### i18x

Stub unter `components/i18n/` und Notizen in `docs/i18x/`. Locales: `en-US` (Fallback), `de-DE`. Kompatibel zur Projektfamilien-i18x-Idee; keine zweite Engine.

### Tests

```bash
npm run test:infra
```

Host-Logik-Tests folgen später unter `test/` (HID-Normalisierung, Config, i18x, …) — ohne Fake-Bluetooth-Coverage.

---

## Projektstatus

| Bereich | Stufe |
| --- | --- |
| Repository / µGulp / Docs | Milestone 1 (diese Foundation) |
| PlatformIO Skeleton | EXPERIMENTAL (provisional board) |
| T-Lion OLED / Pins / Akku | UNVERIFIED |
| Classic BT Host | PLANNED |
| BLE HID Device | PLANNED |
| Bridge | PLANNED |
| Physische Verifikation | ausstehend |

Nächster Milestone (separater Prompt): **PHYSICAL T-LION HARDWARE BRING-UP**.

---

## Beziehung zu ESP][

ESP][ ist der erste vorgesehene Abnehmer (ESP32-S3 Apple-II-Emulator-Projekt). BlueShift bleibt ein **eigenständiges** Open-Source-Projekt und wird nicht ESP][-spezifisch.

---

## Roadmap (kurz)

1. **Milestone 1** — Repository-Foundation ← aktuell
2. T-Lion Hardware-Bring-up (OLED, Buttons, Akku, Diagnose)
3. Classic HID Input (ein Gerät, ein Profil)
4. BLE HID Output + minimale Bridge
5. Reconnect, Profile, Kompatibilitätsausbau

---

## Lizenzierung

BlueShift: Open Source (Lizenzdatei folgt mit dem ersten öffentlichen Release-Schnitt; bis dahin Entwicklungscopyright beim Autor).

Drittanbieter und nur recherchierte Stacks: [`THIRD_PARTY_LICENSES.md`](THIRD_PARTY_LICENSES.md). Bluepad32 / BTstack / ESP32KeyBridge sind **referenziert, nicht eingebunden**.

µGulp-Ready-Badge: offizielles Artwork unter `docs/assets/microgulp-ready.png` ([Regeln](https://microgulp.dev/de/ready/)).

BlueShift-Logo: `docs/assets/blueshift-logo.png` (Vollfarbe; OLED-Mono später separat).
