# Firmware i18n / i18x foundation

BlueShift user-visible strings (OLED, pairing hints, errors, battery messages)
must go through this layer — never hard-coded English-only UI text in bridge
or display code.

## Project-family i18x

The broader project family already has a full **i18x** toolchain (`i18xe` /
µGulp i18x profiles). BlueShift must stay conceptually compatible with that
approach and must **not** invent a competing localization architecture.

This folder is a **flash/RAM-conscious embedded placeholder**:

- Locales planned: `en-US` (technical fallback), `de-DE`
- Beyond translation: numbers, percentages, durations, units, date/time later
- Prefer compact immutable resources in flash — do not load every language into RAM

## Current milestone

Only key stubs exist (`i18n.h`, `locale_en_us.h`, `locale_de_de.h`).
A full catalog and formatters arrive with OLED bring-up.

See also: `docs/i18x/README.md`.
