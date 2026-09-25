<!-- note
Pflegequelle der BlueShift-README. Nur diese Datei bearbeiten.
Danach: npx gulp docs  → erzeugt README.md und de-DE.md
Der Marker COMPATIBILITY_TABLE (HTML-Kommentar im Dokumentkörper) wird aus
docs/compatibility/devices.json ersetzt.
website-Blöcke können später ergänzt werden; unmarked Text erscheint in der Git-README.
-->

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

**Status:** Milestone 2 — Research + Pre-Implementation. Architektur, Host-Tests und dokumentierte T-Lion-Pinannahmen sind vorbereitet. Physische Hardware ist **noch nicht** verfügbar — daher **nichts physisch VERIFIED**.

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

## Architektur (IMPLEMENTED_UNVERIFIED / PLANNED)

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

Vorbereitet in `components/` (Host-Tests ohne Hardware):

- Normalized HID + BLE Report-Builder
- `BridgeCore` FSM + bounded queues + Disconnect-Policy
- Navigation (5-Wege Debounce/Long-Press), UI-Screens, Battery-Modell, Power-Policy
- i18x en-US/de-DE für OLED-Strings
- BT-Interfaces (`ClassicHidHost` / `BleHidPeripheral`) — noch **ohne** Stack-`lib_deps`

Bluetooth-Recherche: [`docs/bluetooth/stack-architecture.md`](docs/bluetooth/stack-architecture.md) — Research-Lead **EspBle Dual-Host / ESP32KeyBridge-Muster**; Bluepad32 als Alternative.

---

## Kompatibilitätsdatenbank

Eine strukturierte Quelle: [`docs/compatibility/devices.json`](docs/compatibility/devices.json).

<!-- COMPATIBILITY_TABLE -->

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

BlueShift: Open Source (Lizenzdatei folgt mit dem ersten öffentlichen Release-Schnitt; bis dahin Entwicklungscopyright beim Autor).

Drittanbieter und nur recherchierte Stacks: [`THIRD_PARTY_LICENSES.md`](THIRD_PARTY_LICENSES.md). Bluepad32 / BTstack / ESP32KeyBridge sind **referenziert, nicht eingebunden**.

µGulp-Ready-Badge: offizielles Artwork unter `docs/assets/microgulp-ready.png` ([Regeln](https://microgulp.dev/de/ready/)).

BlueShift-Logo: `docs/assets/blueshift-logo.png` (Vollfarbe; OLED-Mono später separat).
