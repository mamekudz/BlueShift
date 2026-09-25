# BlueShift i18x notes

Firmware UI localization is prepared from milestone 1.

| Locale | Role |
| --- | --- |
| `en-US` | Technical fallback |
| `de-DE` | Primary human-facing locale (planned OLED) |

Reuse the project-family **i18x** concepts (`i18xe` / µGulp i18x) where practical.
Do not build a second incompatible localization system.

Embedded constraints:

- Prefer immutable flash string tables
- Do not load every translation into RAM
- OLED layouts must tolerate shorter/longer translations (clip / scroll / abbreviate)

Implementation stub: `components/i18n/`.
OLED strings in later milestones **must** call through that layer.
