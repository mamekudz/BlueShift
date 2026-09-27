# BlueShift™ — RELEASES.json workflow

**License:** this document does not change BlueShift Community License 1.0.  
**Scope:** project history / µGulp release tasks only.

---

## Purpose

Root `RELEASES.json` is the **canonical consolidated** BlueShift project history.

Contributor files under `dev/releases/*.json` (e.g. `MAM.json`) avoid merge conflicts during team development.

| File | Role |
| --- | --- |
| `RELEASES.json` | Canonical history (newest release first) |
| `dev/releases/<INITIALS>.json` | Authoring source (en-US) |
| `i18x/gulp/releases/*.json` | Optional de-DE (and en-US) translations of info bodies |
| `i18x/gulp/*.json` | µGulp task display names / descriptions |

---

## Authoring (contributors)

1. Write notes in **en-US** only.
2. Do **not** add `<context="release info"/>` — the update tool owns that.
3. Use `{ main, minor, revision, date, beta, info[] }` contributions.
4. Run `gulp releases:update` / `npm run releases:update`.

---

## 30-day merge

At update time, contributor entries are considered only if:

- `date` is within the last **30 days**, and
- the info line is **not already** present in `RELEASES.json`

Older notes remain in contributor files for authorship history but are not re-merged.

---

## Duplicate identity

Duplicates are detected by:

    version string + fingerprint(stripped info text)

Not by array position. Re-running update is **idempotent**.

---

## Context tags

Canonical lines in `RELEASES.json` end with:

    <context="release info"/>

Before applying, tooling strips equivalent tags so tags never accumulate.

---

## µGulp tasks

| Technical ID | Role |
| --- | --- |
| `releases:update` | Merge contributor notes → `RELEASES.json` |
| `releases:history` | Localized readable history (accordion / CLI) |

Technical IDs stay stable. Visible names are dynamic:

- Update: `Release history up to date` or `Update release history — N pending`
- History: `Release history — <version>` (entry count in tooltip/description as needed)

When pending merges exist, the update task may show **µAttention** (`ACTION_AVAILABLE`, not a build failure). After sync, attention clears via `NotifyTasksChanged`.

Group: **Release** (alongside Firmware / Tools / Docs / Git / Backup — ESP][-aligned).

---

## Verification language

Release notes must preserve BlueShift evidence labels where relevant:

`IMPLEMENTED` · `HOST_VERIFIED` · `LINK_VERIFIED` · `PHYSICALLY_VERIFIED` · `IMPLEMENTED_UNVERIFIED` · `DOCUMENTED`

Do not claim physical success for implemented-but-unverified work.

---

## Backup

`RELEASES.json` and `dev/releases/*.json` are normal tracked source (Git + NAS include lists).  
They do not change the independent Git / NAS / redistribution policy for local vendor assets.
