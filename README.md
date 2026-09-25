# BlueShift™

<p align="center">
  <img src="docs/assets/blueshift-logo.png" alt="BlueShift" width="420">
</p>

<p align="center">
  <a href="https://microgulp.dev/de/ready/">
    <img src="docs/assets/microgulp-ready.png" alt="µGulp Ready" width="110">
  </a>
</p>

**Bringing Classic Bluetooth into the BLE era.**

**BlueShift™** ist der Projekt- und Produktname von Meinolf Amekudzi — eine Bridge von **Classic Bluetooth HID** (BR/EDR) nach **BLE HID**, zunächst vorgesehen für das Board **LILYGO T-Lion**.

Paket-/Repo-Identifier (ASCII): `blueshift` — öffentlicher Wortlaut **BlueShift™** (Wortmarke; nicht in technischen Identifiern). Details: [`docs/licensing/branding-policy.md`](docs/licensing/branding-policy.md).

**Status:** Milestone 3 — maximale Vorab-Implementierung (`0.3.0-dev`). Board-Layer, SSD1306-Backend, OLED-UI, Nav/ADC, NVS, Factory-Reset, Classic/BLE-Spikes, Bridge-Simulation und Host-Tests sind **IMPLEMENTED_UNVERIFIED**. Physische Hardware ist **noch nicht** verfügbar — daher **nichts physisch VERIFIED**.

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
| **DOCUMENTED** | Hersteller-/Schematic-Beleg, bei uns noch nicht gemessen |
| **IMPLEMENTED_UNVERIFIED** | Code vorhanden/kompiliert/getestet, nicht auf unserem T-Lion gelaufen |
| **EXPERIMENTAL** | Vorhanden, Verifikation unvollständig |
| **PLANNED** | Architektur/Ziel, noch nicht gebaut |
| **ASSUMED** | Arbeitsannahme, muss verifiziert werden |
| **INCOMPATIBLE** | Mit belegter Begründung ausgeschlossen |
| **UNKNOWN** | Evidenz unzureichend |

Solange das T-Lion nicht physisch getestet ist, dürfen OLED, Akku, Classic BT, BLE und PSRAM **nicht** als VERIFIED gelten.

---

## Vorgesehene Hardware (T-Lion)

Dossier: [`docs/hardware/t-lion.md`](docs/hardware/t-lion.md) (Schematic `t18_v2.3.pdf` + offizielles LilyGO-Beispiel).

| | | Confidence |
| --- | --- | --- |
| Board | LILYGO T-Lion / T-Controller / T18 | DOCUMENTED |
| MCU | ESP32-WROVER | DOCUMENTED |
| OLED | SSD1306 128×64 I²C `0x3C`, SDA21/SCL22 | DOCUMENTED |
| 5-Wege | GPIO 32/33/34/36/39 | DOCUMENTED |
| Akku-ADC | GPIO35, Teiler ×2 | DOCUMENTED |
| Charger | TP5400 | DOCUMENTED |
| USB-UART | Schematic CP2104 vs. Marketing CH9102 | CONFLICT / UNKNOWN |
| Zellschutz | Nicht als vollständig nachgewiesen | UNKNOWN |

PlatformIO: `skeleton` (esp32dev smoke) + `t-lion-debug` / `t-lion-release` (custom board JSON, **IMPLEMENTED_UNVERIFIED**).

---

## Architektur (IMPLEMENTED_UNVERIFIED)

```
Classic HID Device
        |
ClassicHidHostSpike (stub / mock; Radio später)
        |
   Raw HID Report
        |
 Device Parser / Profile
        |
 Normalized Input
        |
    BridgeCore
        |
 BLE report builder / BleHidPeripheralSpike
        |
   BLE-only Host
```

In `components/` (Host-Tests ohne Radio):

- Board-Facade T-Lion (keine App-GPIO-Hardcodes)
- SSD1306 via ThingPulse (nur `t-lion-*`) + Framebuffer für Host/Skeleton
- OLED-UI-Renderer (Status/Pair/Devices/Profiles/Diagnostics/Settings/About)
- GPIO-Nav + ADC-Battery-Backends (DOCUMENTED Pins)
- NVS `ConfigStore` + Factory-Reset-Orchestrierung
- Classic/BLE-Spikes mit Timeouts; Dual-Radio bewusst **nicht** default-linked
- E2E-Simulation, Stuck-Key-/Disconnect-/Malformed-HID-Tests
- i18x en-US/de-DE für alle aktuellen UI-Strings

Stack-Entscheidung: [`docs/bluetooth/m3-stack-decision.md`](docs/bluetooth/m3-stack-decision.md).

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
platformio.ini   — skeleton / t-lion-debug / t-lion-release / native
src/main.cpp     — Boot-Skeleton + dokumentierte Pin-Logs
include/blueshift/version.h  — 0.2.0-dev
```

Gepinnt: `espressif32 @ 6.9.0`. Bluetooth-Stacks noch **nicht** als `lib_deps`.

```bash
pio run -e skeleton
pio run -e t-lion-debug
pio run -e native          # Host-Unit-Tests
npm run test:infra
```

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
| Repository / µGulp / Docs | Milestone 1+2 Foundation |
| T-Lion Dossier / Schematic | DOCUMENTED |
| PlatformIO `t-lion-*` | IMPLEMENTED_UNVERIFIED |
| Bridge / HID / UI Logic (Host-Tests) | IMPLEMENTED_UNVERIFIED |
| Classic BT / BLE Stack in Firmware | PLANNED (Recherche abgeschlossen) |
| Physische Verifikation | ausstehend |

Version: **0.2.0-dev**. Nächster physischer Schritt: **T-LION HARDWARE BRING-UP**.

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

Die endgültige Projektlizenz von BlueShift wird derzeit vorbereitet.

Das Projekt soll für persönliche Nutzung, Hobby, Bildung, Forschung und
andere nicht-kommerzielle Zwecke frei verfügbar bleiben.

Interesse an Herstellung, Vertrieb, Integration oder Verkauf von BlueShift
oder BlueShift-basierter Hardware? Bitte den Projekt-Autor kontaktieren, um
kommerzielle Lizenzierung oder OEM-Vereinbarungen zu besprechen:

- Repository: https://github.com/mamekudz/BlueShift
- Issues: https://github.com/mamekudz/BlueShift/issues
- Profil: https://github.com/mamekudz

Drittanbieter-Komponenten unterliegen weiterhin ihren eigenen Lizenzen
([`THIRD_PARTY_LICENSES.md`](THIRD_PARTY_LICENSES.md),
[`docs/licensing/license-audit.md`](docs/licensing/license-audit.md)).

Bluepad32 / BTstack / ESP32KeyBridge / EspBle sind **referenziert, nicht
eingebunden**. Eine BlueShift-Lizenz ersetzt nicht die Lizenzen dieser
Projekte (bei BTstack insbesondere die kommerziellen Bedingungen von
BlueKitchen).

Marken-/Logo-Rechte sind von der Softwarelizenz getrennt; siehe
[`docs/licensing/branding-policy.md`](docs/licensing/branding-policy.md) und
[`docs/licensing/license-policy-draft.md`](docs/licensing/license-policy-draft.md)
(**Policy-Entwurf, kein Rechtstext**).

Wortmarke: **BlueShift™** (nicht ®). Grafisches Logo: Entwurf —
Markenbehandlung **UNDECIDED**.

µGulp-Ready-Badge: offizielles Artwork unter `docs/assets/microgulp-ready.png`
([Regeln](https://microgulp.dev/de/ready/)).

BlueShift-Logo-Entwurf: `docs/assets/blueshift-logo.png` (Vollfarbe; OLED-Mono
später separat) — kein automatisches Markenrecht für kommerzielle Produkte.
