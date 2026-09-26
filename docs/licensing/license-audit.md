# BlueShift license audit

**Status:** engineering / legal inventory — **NOT** final license text.  
**Baseline commit audited:** `b9d3506` (`feat: implement pre-hardware BlueShift firmware stack`)  
**Audit date:** 2026-09-25  
**Repository:** https://github.com/mamekudz/BlueShift

This document classifies repository material so a future dual
community/commercial BlueShift license can cover **only** what the author
owns, without pretending to re-license third-party components.

Classification keys:

| Code | Meaning |
| --- | --- |
| **ORIGINAL_BLUESHIFT** | Authored for BlueShift in Milestones 1–3 |
| **THIRD_PARTY_DEPENDENCY** | External package linked or required at build/runtime |
| **ADAPTED_THIRD_PARTY** | Substantial third-party code modified into the tree |
| **GENERATED** | Machine-generated from another source |
| **REFERENCE_ONLY** | Studied / documented; not vendored or linked |
| **LICENSE_UNCLEAR** | Origin or terms not fully established |

---

## Summary verdict (current tree)

| Finding | Result |
| --- | --- |
| Final BlueShift `LICENSE` file | **Not present** (intentional) |
| Vendored `vendor/` / `lib/` trees | **None** |
| Bluepad32 | **REFERENCE_ONLY** — not dependency, not vendored |
| BTstack | **REFERENCE_ONLY** — not dependency, not vendored |
| ESP32KeyBridge / EspBle | **REFERENCE_ONLY** — architecture research; **no code copied** |
| Firmware-linked third-party (T-Lion) | ThingPulse SSD1306 **MIT** via PlatformIO `lib_deps` |
| Build framework (distributed binary) | Arduino-ESP32 / ESP-IDF family under Espressif terms (Apache-2.0 lineage) |
| npm packages | **dev-only** (Gulp / µGulp API) — not in firmware image |

**No `ADAPTED_THIRD_PARTY` code was found in the current tree.**

---

## Inventory table

| Component | Origin class | Current use | License (upstream) | In repo / Distributed? | Commercial implications | Attribution | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `src/`, `include/blueshift/*`, most of `components/**` | ORIGINAL_BLUESHIFT | Firmware + host tests | **BlueShift Community License 1.0** ([`LICENSE.md`](../../LICENSE.md), provisional) | Yes / yes (firmware) | Non-commercial under BCL; commercial needs separate license | Author | Clear |
| `boards/lilygo_t_lion.json` | ORIGINAL_BLUESHIFT | PlatformIO board JSON | *BlueShift TBD* | Yes / build meta | Author | Clear |
| `platformio.ini`, `gulpfile.mjs`, `dev/tools/*` | ORIGINAL_BLUESHIFT | Build / µGulp | *BlueShift TBD* | Yes / tool-only | Author | Clear |
| `docs/**` (except noted assets/evidence) | ORIGINAL_BLUESHIFT | Documentation | *BlueShift TBD* | Yes / docs | Author | Clear |
| `test/**` | ORIGINAL_BLUESHIFT | Host tests | *BlueShift TBD* | Yes / not product | Author | Clear |
| `CLAUDE.md`, `docs/compatibility/devices.json` | ORIGINAL_BLUESHIFT | Project rules / DB | *BlueShift TBD* | Yes | Author | Clear |
| ThingPulse SSD1306 (`SSD1306Wire`) | THIRD_PARTY_DEPENDENCY | OLED backend on `t-lion-*` | **MIT** (`license` in upstream; © 2016 Daniel Eichhorn / Fabrice Weinberg) | Not vendored; PlatformIO fetch @ **4.6.2** / yes in T-Lion firmware | Permissive; keep notice | Required by MIT | Clear |
| PlatformIO platform `espressif32` **6.9.0** | THIRD_PARTY_DEPENDENCY | Build platform | Platform Apache-2.0; packages have own notices | Toolchain / yes in firmware | Must keep Espressif notices in binary distributions | Package notices | Clear enough |
| Arduino-ESP32 framework (via platform) | THIRD_PARTY_DEPENDENCY | Firmware framework | Espressif **Apache-2.0** family (verify package NOTICE at release) | Toolchain / yes | Apache terms apply to framework code | Espressif | Clear enough |
| ESP-IDF components (via Arduino) | THIRD_PARTY_DEPENDENCY | BT/HAL/etc. when enabled | Primarily **Apache-2.0** + 3rd-party NOTICE files | Toolchain / yes when linked | Cannot be re-licensed by BlueShift | Espressif NOTICE | Clear enough |
| Arduino `Wire`, `Preferences` | THIRD_PARTY_DEPENDENCY | I²C / NVS | Part of Arduino-ESP32 | Framework / yes | Same as framework | Espressif | Clear |
| `gulp` ^5 | THIRD_PARTY_DEPENDENCY (dev) | Docs/format/backup tasks | **MIT** (npm) | `node_modules` / **not** firmware | Dev-only | npm | Clear |
| `gulp-mu-gulp-api` ^0.5.0 | THIRD_PARTY_DEPENDENCY (dev) | µGulp task API | **MIT** (npm) | `node_modules` / **not** firmware | Dev-only | µGulp / npm | Clear |
| Node.js built-in `node:test` | THIRD_PARTY_DEPENDENCY (dev) | Tests | Node license | Host only | Dev-only | — | Clear |
| Bluepad32 | REFERENCE_ONLY | Research / profile notes | **Apache-2.0** + **BTstack** dependency notice | Docs only / **no** | If later linked: BTstack commercial rules apply | Apache NOTICE | Clear |
| BTstack (BlueKitchen) | REFERENCE_ONLY | Research (via Bluepad32 path) | Modified BSD-style with **clause 4: non-commercial only**; commercial license from BlueKitchen | **Not** in tree / **no** | **Major blocker** if adopted for commercial BlueShift products without BlueKitchen deal | Required | Clear |
| ESP32KeyBridge | REFERENCE_ONLY | Architecture research | **MIT** (upstream LICENSE, © 2026 TANAKA Masayuki) | Docs only / **no** code copied | MIT allows commercial reuse *if* code later incorporated with notice | MIT notice if copied | Clear |
| EspBle / EspBleClassic | REFERENCE_ONLY | Dual-host candidate | **MIT** (upstream LICENSE) | Docs only / **no** | Same as KeyBridge; bundled NimBLE notices if adopted | MIT + NimBLE | Clear |
| NimBLE-Arduino | REFERENCE_ONLY (optional spike flag off) | Optional BLE path in docs/`platformio.ini` comments | **Apache-2.0** (+ bundled notices) | Not default-linked / **no** | Permissive if enabled later | Apache NOTICE | Clear |
| Button2 (LilyGO example) | REFERENCE_ONLY | Mentioned in hardware docs | Verify before adopt (typically MIT; **not audited as dependency**) | **no** | N/A until adopted | — | LICENSE_UNCLEAR until adopt |
| LilyGO T-Controller repo / schematic | REFERENCE_ONLY (+ evidence copy) | Pin/hardware evidence | Manufacturer materials; **not** BlueShift code | `docs/hardware/evidence/*` / docs only | Do not treat as BlueShift IP; redistribution of PDF may have manufacturer constraints | LilyGO | Evidence |
| USB HID boot layouts / report shapes | ORIGINAL_BLUESHIFT (spec-derived) | `ble_hid_reports`, keyboard parser | Public HID usage conventions; BlueShift-authored bytes | Yes | Spec-derived original implementation | — | Clear |
| SN30 / Nimbus parser stubs | ORIGINAL_BLUESHIFT | Conservative layouts informed by public notes / Bluepad32 *research* | *BlueShift TBD* | Yes | **No Bluepad32 source copied**; re-verify if expanding from upstream parsers | Cite research docs | Clear |
| `docs/assets/blueshift-logo.png` | ORIGINAL_BLUESHIFT (draft logo) | README draft artwork | Word mark is BlueShift™; **figurative mark UNDECIDED** | Yes | Not a commercial brand grant; see branding-policy.md | Author | Clear |
| `components/ui/oled_logo_placeholder.h` | ORIGINAL_BLUESHIFT | Temporary mono slot | *BlueShift TBD* | Yes | Placeholder only | Author | Clear |
| `docs/assets/microgulp-ready.png` | THIRD_PARTY_DEPENDENCY (asset) | Badge | µGulp Ready badge rules ([microgulp.dev/de/ready](https://microgulp.dev/de/ready/)) — free badge use when criteria met; **not** a software license | Yes / docs | Badge rights ≠ firmware license | Link to Ready page | Clear |
| `dev/drafts/*` | ORIGINAL_BLUESHIFT / drafts | Author drafts | *Author* | Yes | Not product | Author | Clear |
| Generated OLED header (`oled_logo_generated.h`) | GENERATED (when created) | Future asset pipeline | Inherits author artwork rights | Not present yet | Same as logo | — | N/A |

---

## Source-origin notes (Milestones 1–3)

### Newly written for BlueShift

Application layer, bridge FSM, bounded queues, navigation debounce, battery curve, UI controller/renderer, i18n tables, NVS config, factory-reset orchestration, Classic/BLE **spike** state machines, host tests, PlatformIO board JSON, µGulp tooling, most documentation.

### Spec / documentation derived (still BlueShift-authored code)

- USB HID boot keyboard / mouse report packing  
- Conservative gamepad report builder  
- T-Lion pin constants from LilyGO schematic + `adc.ino` (**facts**, not LilyGO source code)

### Adapted / copied third-party source

**None found** in the repository at `b9d3506`.

`components/display/ssd1306_display.*` **calls** ThingPulse APIs; it does not vendor ThingPulse sources.

Comments referencing Bluepad32 / community dumps document **research influence** for SN30 field guesses; the parser implementation is BlueShift-written and marked IMPLEMENTED_UNVERIFIED.

### Architectural inspiration (no code copy)

ESP32KeyBridge / EspBle patterns (raw → normalize → output; release-on-disconnect) informed BlueShift interfaces. Evaluation docs state code was **not vendored**. Spot-check of tree confirms no KeyBridge/EspBle source files.

---

## Bluepad32

| Question | Answer |
| --- | --- |
| Evaluated only? | **Yes** |
| `lib_deps` / vendored? | **No** |
| Adapted code in tree? | **No** |
| Linked at build? | **No** |
| Version/commit in BlueShift | **N/A** (not present) |
| Upstream license | **Apache-2.0** for Bluepad32 itself ([LICENSE](https://github.com/ricardoquesada/bluepad32/blob/main/LICENSE)) |
| Critical transitive | **BTstack** (see next section) |

Upstream explicitly warns that Bluepad32 depends on BlueKitchen BTstack, free for open-source projects, commercial for closed-source / commercial products (contact BlueKitchen).

---

## BTstack (BlueKitchen) — commercial implications

| Question | Answer |
| --- | --- |
| In BlueShift today? | **No** (REFERENCE_ONLY / EVALUATED) |
| Required by selected architecture? | **No** — production candidate is ESP-IDF Bluedroid BTDM |
| Upstream LICENSE | https://github.com/bluekitchen/btstack/blob/master/LICENSE |

**Decision:** Do **not** change BlueShift licensing to accommodate BTstack. Prefer Espressif stack.

---

## Selected Bluetooth production candidate (Milestone 4)

| Item | Value |
| --- | --- |
| Architecture | ESP-IDF Bluedroid `ESP_BT_MODE_BTDM` + Classic HID Host + BLE HID Device |
| Adapters | `BluetoothPlatform`, `EspIdfClassicHidHost`, `EspIdfBleHidPeripheral` |
| Commercial-dependency class | **GREEN** |
| Current Arduino pin | Classic HID Host **API missing** in prebuilt SDK — Partial |
| Docs | `docs/licensing/bluetooth-stack-matrix.md`, `docs/bluetooth/esp-idf-dual-mode.md`, `docs/bluetooth/esp-idf-dual-mode-spike.md` |
| Native spike (2026-09-26) | ESP-IDF **5.3.1** dual-mode Classic HID Host + BLE HID Device: **LINK VERIFIED** |
| BTstack | **NOT REQUIRED** for production |

---

## ESP32KeyBridge / EspBle

| Item | Finding |
| --- | --- |
| Code copied / adapted / translated into BlueShift? | **No** |
| Structurally derived? | Ideas only (layering, dual-host research) |
| ESP32KeyBridge license | **MIT** (upstream LICENSE) |
| EspBle license | **MIT** (upstream LICENSE) |
| Attribution if later incorporated | Retain MIT copyright notices; audit NimBLE/Bluedroid notices inside EspBle packages |

---

## OLED / ThingPulse

| Item | Finding |
| --- | --- |
| Package | `thingpulse/ESP8266 and ESP32 OLED driver for SSD1306 displays @ 4.6.2` |
| License | **MIT** |
| Used when | `t-lion-debug` / `t-lion-release` |
| Skeleton | Framebuffer mock — no ThingPulse |
| Wrapper | `components/display/ssd1306_display.*` is ORIGINAL_BLUESHIFT |

---

## Assets / branding

| Asset | Rights note |
| --- | --- |
| Word mark **BlueShift™** | Project/product name used by Meinolf Amekudzi; **™** only (not ®). See [`branding-policy.md`](branding-policy.md) |
| `blueshift-logo.png` | Draft graphical logo — figurative mark treatment **UNDECIDED**; not a registered mark; not a commercial brand grant by repo visibility |
| OLED mono placeholder | Temporary; not a figurative mark claim |
| `microgulp-ready.png` | Official µGulp Ready badge under published Ready-page rules; software license unchanged |
| LilyGO evidence PDF/txt | Manufacturer evidence; not BlueShift branding |

**Trademark:** Word-mark claim uses **™**. Do **not** claim `BlueShift®` or imply registration. Logo trademark treatment remains undecided.

---

## Contribution-model options (analysis)

| Option | Pros | Cons |
| --- | --- | --- |
| **A) CLA** | Clear relicensing / commercial dual-license rights | Friction; legal drafting needed |
| **B) DCO + inbound=outbound** | Lightweight; common in OSS | Harder to dual-license later if inbound equals a permissive OSS grant that conflicts with commercial exclusivity |
| **C) No outside code until license finalized** | Safest for dual/community-commercial intent | Slower community growth |

**Recommendation for current early stage:** **Option C** — accept docs/issue reports freely; **do not merge third-party code PRs** until the final license + contribution terms exist. Optionally prepare a CLA later (Option A) before opening code contributions.

---

## Blockers for the intended community/commercial model

1. **BlueShift Community License 1.0** in [`LICENSE.md`](../../LICENSE.md) (**provisional** until author declares final).  
2. **BTstack** if that stack path is chosen — separate commercial deal required for commercial products. **Not** a production dependency.  
3. **Brand/logo** must be carved out of any software grant (BCL §7; ™ only, never ®; logo **UNDECIDED**).  
4. **Third-party notices** (ThingPulse MIT; Espressif Apache NOTICE; future stacks) must ship with binaries.  
5. **Contributor inbound rights** — risk if merging PRs before dual-license CLA/terms (BCL §6).  
6. **LilyGO evidence materials** — keep as documentation evidence; do not present as BlueShift-owned hardware IP.

Paths that currently look **compatible** with a commercial BlueShift layer (subject to final counsel review):

- Author-owned firmware (`components/`, `src/`, …)  
- ThingPulse MIT OLED  
- Espressif Apache-2.0 framework components  
- Future EspBle/KeyBridge MIT code **if** incorporated with notices (still verify all transitive notices)

---

## Related documents

- [`license-policy-draft.md`](license-policy-draft.md) — plain-language policy draft (**not** legal text)  
- [`../../THIRD_PARTY_LICENSES.md`](../../THIRD_PARTY_LICENSES.md) — incorporated vs evaluated  
- [`../bluetooth/m3-stack-decision.md`](../bluetooth/m3-stack-decision.md) — stack choice vs license risk  
