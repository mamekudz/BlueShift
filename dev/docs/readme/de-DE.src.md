<!-- note
Pflegequelle der deutschen BlueShift-README. Nur diese Datei und en-US.src.md bearbeiten.
Danach: npx gulp docs  → erzeugt README.de-DE.md und aktualisiert die Baseline de-DE.md.
Der Marker COMPATIBILITY_TABLE (HTML-Kommentar im Dokumentkörper) wird aus
docs/compatibility/devices.json ersetzt.
website-Blöcke können später ergänzt werden; unmarked Text erscheint in der Git-README.
-->

# BlueShift™

[English](README.md) | **Deutsch**

<p align="center">
  <img src="docs/assets/blueshift-logo.png" alt="BlueShift" width="420">
</p>

<p align="center">
  <a href="https://microgulp.dev/de/ready/">
    <img src="docs/assets/microgulp-ready.png" alt="µGulp Ready" width="110">
  </a>
</p>

> **🚧 Work in Progress / In Arbeit**
>
> Dieses Projekt wird aktiv weiterentwickelt.
> Hardware, Firmware, APIs, Dokumentation und Kompatibilität können sich ändern.
> Nichts ist physisch VERIFIED, solange kein T-Lion getestet wurde.

**Bringing Classic Bluetooth into the BLE era.**

**BlueShift™** ist der Projekt- und Produktname von Meinolf Amekudzi — eine Bridge von **Classic Bluetooth HID** (BR/EDR) nach **BLE HID**, zunächst vorgesehen für das Board **LILYGO T-Lion**.

Paket-/Repo-Identifier (ASCII): `blueshift` — öffentlicher Wortlaut **BlueShift™** (Wortmarke; nicht in technischen Identifiern). Details: [`docs/licensing/branding-policy.md`](docs/licensing/branding-policy.md).

**Status:** Milestone 5 — Audio-Erweiterungs-Feasibility (`0.5.0-dev`). HID-V1-Pfad unveraendert. Experimentelle ESP][-Speaker-zu-A2DP-Architektur in Entwicklung (Protokoll + Edge→PCM host-getestet; A2DP/Triple-Role-Spikes). Physisches T-Lion **noch nicht** verfuegbar — **nichts PHYSICALLY VERIFIED**.

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

<!-- COMPATIBILITY_TABLE -->

---

## Entwicklung

### PlatformIO

```text
platformio.ini   — skeleton / t-lion-debug / t-lion-release / native
src/main.cpp     — Boot-Skeleton + dokumentierte Pin-Logs
include/blueshift/version.h  — 0.5.0-dev
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
npx gulp docs          # README.md + README.de-DE.md + Kompatibilitätstabelle
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
npm run test:host
```

---

## µGulp-ready

Dieses Repository nutzt den **µGulp**-Automatisierungsworkflow für:

- Dokumentationsgenerierung (`gulp docs`)
- Lokalisierung der README-Quellen (en-US / de-DE)
- Infrastruktur-Validierung (`npm run test:infra`)
- C/C++-Formatierungshilfen (`gulp format` / `format:check`)
- Git-Checkpoints (`gulp backup:git`)
- NAS-Backup, sofern konfiguriert (`gulp backup` / `backup:nas` / `backup:all`)

µGulp-Ready-Badge: offizielles Artwork unter `docs/assets/microgulp-ready.png` ([Regeln](https://microgulp.dev/de/ready/)).

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

Version: **0.5.0-dev**. Nächster physischer Schritt: **T-LION HARDWARE BRING-UP** (`t-lion-idf-debug`).

---

## Beziehung zu ESP][

ESP][ ist der erste vorgesehene Abnehmer (ESP32-S3 Apple-II-Emulator-Projekt). BlueShift bleibt ein **eigenständiges** Open-Source-Projekt und wird nicht ESP][-spezifisch.

---

## Roadmap (kurz)

1. **Milestone 1** — Repository-Foundation
2. T-Lion Hardware-Bring-up (OLED, Buttons, Akku, Diagnose)
3. Classic HID Input (ein Gerät, ein Profil)
4. BLE HID Output + minimale Bridge
5. Reconnect, Profile, Kompatibilitätsausbau

### Experimentell / Roadmap (Audio)

**Eine experimentelle ESP][→Classic-Bluetooth-Audio-Brücke befindet sich in Entwicklung.**

Kein fertiger „Bluetooth-Audio-Bridge“-Anspruch. Physisches Audio: UNVERIFIED. Siehe [`docs/audio/architecture.md`](docs/audio/architecture.md) und [`docs/protocol/esp2-extension.md`](docs/protocol/esp2-extension.md).
HID-Bridging bleibt die primäre V1-Fähigkeit.

---

## Lizenzierung

Originale BlueShift-Materialien stehen unter der **BlueShift Community License 1.0**
([`LICENSE.md`](LICENSE.md)) — vorläufiger Custom-Lizenztext.

Nicht-kommerzielle persönliche, Hobby-, Bildungs- und Forschungsnutzung ist unter
dieser Lizenz erlaubt, einschließlich nicht-kommerzieller Weitergabe von Quellcode,
Firmware-Binaries und zugehörigen Materialien gemäß `LICENSE.md`. **Commercial Use
erfordert eine separate schriftliche Lizenz** des Autors. Kommerzielle Organisationen
dürfen laut Abschnitt 5 intern evaluieren und Prototypen bauen.

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
[`LICENSE.md`](LICENSE.md) §7.

Wortmarke: **BlueShift™** (nicht ®). Grafisches Logo: Entwurf —
Markenbehandlung **UNDECIDED**.

BlueShift-Logo-Entwurf: `docs/assets/blueshift-logo.png` (Vollfarbe; OLED-Mono
später separat) — kein automatisches Markenrecht für kommerzielle Produkte.
