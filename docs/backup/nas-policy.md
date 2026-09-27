# BlueShift™ — private NAS backup policy

**License:** this document does not change BlueShift Community License 1.0.  
**Scope:** backup behavior only.

---

## Three independent properties

| Property | Meaning |
| --- | --- |
| **GIT_TRACKED** | May the file enter the public/private Git history? |
| **NAS_BACKED_UP** | Should private NAS backup preserve it? |
| **REDISTRIBUTABLE** | May it be published / shared beyond the author’s machines? |

These must **not** be conflated.

`.gitignore` protects the repository.  
NAS backup protects the working environment and valuable local assets.

---

## Policy summary

| Asset class | Git | NAS | Redistrib |
| --- | --- | --- | --- |
| BlueShift source / docs | YES (per project) | YES | per BCL 1.0 |
| Vendor STEP / unclear CAD | NO | YES | NO / UNKNOWN |
| Plasticity masters | usually NO (binary) | YES | project decision |
| Hardware evidence in Git | YES when permitted | YES | per upstream |
| Local evidence drops (`evidence/local/`, `_refs/`) | NO | YES | usually NO |
| Physical test captures (`_physical/`) | NO | YES if hard to recreate | usually NO |
| Verified release artifacts (`releases/`) | per release policy | YES | per release policy |
| `node_modules/`, `.pio/`, `.cache/`, `tmp/` | NO | **NO** | n/a |
| `.env` secrets | NO | **NO** | NO |

Private NAS backup does **not** grant redistribution rights.

---

## Implementation

Module: `dev/tools/nas-backup.mjs`

- Explicit **include** dirs/files (not `git ls-files`, not “everything not ignored”)
- Explicit **exclude** for disposable caches
- `_refs/`, `_physical/`, `3dprint/`, `docs/`, `releases/` are NAS-included when present
- `BACKUP_MANIFEST.json` on the destination is **private** (category counts; do not publish)

µGulp tasks: `backup` / `backup:nas` / `backup:all` (unchanged names).

---

## Restore

`RestoreNasTrees(backupRoot, projectRoot)` restores selected trees (default: `3dprint`, `_refs`, `_physical`, `docs/hardware/evidence`, `releases`).

After restore:

- intentionally gitignored vendor/CAD/Plasticity files remain **untracked**
- `git status` should stay clean regarding those ignores
- do not `git add -f` vendor CAD without a redistribution decision

---

## 3dprint / Plasticity

Gitignored (typical):

- `3dprint/**/vendor/`, `3dprint/**/local/`
- STEP/Parasolid binaries under `cad/step`, `cad/parasolid` (README kept)
- `3dprint/plasticity/**` (README kept)
- `*.plasticity`

NAS still mirrors the whole `3dprint/` tree, including those binaries when present.
