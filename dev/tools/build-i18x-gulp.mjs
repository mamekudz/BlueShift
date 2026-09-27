/**
 * Build i18x/gulp dictionaries for BlueShift µGulp task metadata.
 * Source language is English (keys). de-DE provides German UI strings.
 *
 * Run: node dev/tools/build-i18x-gulp.mjs
 */
import { mkdirSync, writeFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const root = join(dirname(fileURLToPath(import.meta.url)), "../..");
const outDir = join(root, "i18x", "gulp");
mkdirSync(outDir, { recursive: true });

/** @type {Record<string, string>} */
const de = {
  // Groups
  'Firmware<context="µGroup"/>': "Firmware",
  'Tools<context="µGroup"/>': "Werkzeuge",
  'Docs<context="µGroup"/>': "Docs",
  'Release<context="µGroup"/>': "Release",
  'Git<context="µGroup"/>': "Git",
  'Backup<context="µGroup"/>': "Backup",

  // Firmware
  'Build Firmware V<version/><context="µDisplayName"/>': "Firmware bauen V<version/>",
  'Rebuild Firmware V<version/><context="µDisplayName"/>': "Firmware neu bauen V<version/>",
  'Build & Upload V<version/><context="µDisplayName"/>': "Bauen & hochladen V<version/>",
  'Upload Firmware V<version/><context="µDisplayName"/>': "Firmware hochladen V<version/>",
  'Memory Size V<version/><context="µDisplayName"/>': "Speicherbelegung V<version/>",
  'Clean Build V<version/><context="µDisplayName"/>': "Build bereinigen V<version/>",
  'Show Firmware Env V<version/><context="µDisplayName"/>': "Firmware-Umgebung anzeigen V<version/>",
  'Compiles BlueShift with PlatformIO. Default env t-lion-idf-debug (BLUESHIFT_PIO_ENV).<context="µDescription"/>':
    "Kompiliert BlueShift mit PlatformIO. Standard-Umgebung t-lion-idf-debug (BLUESHIFT_PIO_ENV).",
  'Rebuilds and flashes BlueShift firmware in one step.<context="µDescription"/>':
    "Baut die BlueShift-Firmware neu und flasht sie in einem Schritt.",
  'Flashes the last build. Asks for the COM port.<context="µDescription"/>':
    "Flasht den letzten Build. Fragt nach dem COM-Port.",
  'Shows RAM / Flash usage of the last firmware build.<context="µDescription"/>':
    "Zeigt RAM-/Flash-Nutzung des letzten Firmware-Builds.",
  'Removes .pio/build artefacts for the active PlatformIO env.<context="µDescription"/>':
    "Entfernt .pio/build-Artefakte der aktiven PlatformIO-Umgebung.",
  'clean → build for the active PlatformIO env.<context="µDescription"/>':
    "clean → build für die aktive PlatformIO-Umgebung.",
  'Prints the active PlatformIO environment and known envs.<context="µDescription"/>':
    "Zeigt die aktive PlatformIO-Umgebung und bekannte Envs.",
  'Set BLUESHIFT_PORT=COMx to skip the port dialog.<context="µTooltip"/>':
    "BLUESHIFT_PORT=COMx setzen, um den Port-Dialog zu überspringen.",

  // Tools
  'List Devices V<version/><context="µDisplayName"/>': "Geräte auflisten V<version/>",
  'Serial Monitor V<version/><context="µDisplayName"/>': "Seriellmonitor V<version/>",
  'Lists connected serial ports.<context="µDescription"/>': "Listet angeschlossene Seriellports.",
  'Opens the PlatformIO serial monitor at 115200 baud.<context="µDescription"/>':
    "Öffnet den PlatformIO-Seriellmonitor mit 115200 Baud.",

  // Docs
  'Help V<version/><context="µDisplayName"/>': "Hilfe V<version/>",
  'Compose README V<version/><context="µDisplayName"/>': "README erzeugen V<version/>",
  'Format C/C++ V<version/><context="µDisplayName"/>': "C/C++ formatieren V<version/>",
  'Check C/C++ format V<version/><context="µDisplayName"/>': "C/C++-Format prüfen V<version/>",
  'Project checks V<version/><context="µDisplayName"/>': "Projektprüfungen V<version/>",
  'Lists BlueShift Gulp tasks. Safe default — no build, commit, or NAS.<context="µDescription"/>':
    "Listet BlueShift-Gulp-Tasks. Sichere Standardaufgabe — kein Build, Commit oder NAS.",
  'Generates README.md and README.de-DE.md from en-US/de-DE sources and injects the compatibility table from devices.json.<context="µDescription"/>':
    "Erzeugt README.md und README.de-DE.md aus en-US/de-DE-Quellen und fügt die Kompatibilitätstabelle aus devices.json ein.",
  'Runs clang-format -i on src/, include/, components/, test/. Requires clang-format on PATH or CLANG_FORMAT.<context="µDescription"/>':
    "Führt clang-format -i auf src/, include/, components/, test/ aus. Benötigt clang-format im PATH oder CLANG_FORMAT.",
  'clang-format --dry-run --Werror on project sources. Soft-skips if clang-format is missing.<context="µDescription"/>':
    "clang-format --dry-run --Werror auf Projektquellen. Soft-Skip, wenn clang-format fehlt.",
  'format:check plus optional PlatformIO cppcheck when available. Does not flash hardware.<context="µDescription"/>':
    "format:check plus optional PlatformIO-cppcheck, falls verfügbar. Flasht keine Hardware.",

  // Release
  'Release history up to date V<version/><context="µDisplayName"/>':
    "Release-Historie aktuell V<version/>",
  'Update release history — pending V<version/><context="µDisplayName"/>':
    "Release-Historie aktualisieren — ausstehend V<version/>",
  'Release history V<version/><context="µDisplayName"/>': "Release-Historie V<version/>",
  'Merges fresh contributor notes from dev/releases/*.json into RELEASES.json (30-day window, fingerprint duplicates, release-info context tags).<context="µDescription"/>':
    "Mergt frische Contributor-Notizen aus dev/releases/*.json nach RELEASES.json (30-Tage-Fenster, Fingerprint-Duplikate, release-info-Kontext-Tags).",
  'Shows localized BlueShift release history from RELEASES.json (date, version, info lines). Does not dump raw JSON.<context="µDescription"/>':
    "Zeigt lokalisierte BlueShift-Release-Historie aus RELEASES.json (Datum, Version, Infozeilen). Kein Roh-JSON-Dump.",
  'Unmerged contributor release notes available.<context="µAttentionTooltip"/>':
    "Nicht gemergte Contributor-Release-Notes verfügbar.",
  'ACTION_AVAILABLE when unmerged contributor notes exist — not a build failure. Safe to re-run (idempotent).<context="µTooltip"/>':
    "ACTION_AVAILABLE bei ungemergten Contributor-Notizen — kein Build-Fehler. Sicheres erneutes Ausführen (idempotent).",
  'Read-only history view. Translations: i18x/gulp/releases/{en-US,de-DE}.json.<context="µTooltip"/>':
    "Nur-Lesen-Historie. Übersetzungen: i18x/gulp/releases/{en-US,de-DE}.json.",

  // Git (publication)
  'Publish Git checkpoint V<version/><context="µDisplayName"/>':
    "Git-Checkpoint veröffentlichen V<version/>",
  'Publishes a Git checkpoint commit (CLAUDE.md included). Shows staged files first. Pushes when a remote exists. Never force-pushes. This is publication, not backup — use NAS for private backup.<context="µDescription"/>':
    "Veröffentlicht einen Git-Checkpoint-Commit (inkl. CLAUDE.md). Zeigt zuerst gestagte Dateien. Pusht bei vorhandenem Remote. Nie Force-Push. Das ist Veröffentlichung, kein Backup — für privates Backup NAS nutzen.",

  // NAS Backup
  'Backup to NAS V<version/><context="µDisplayName"/>': "Backup auf NAS V<version/>",
  'Docs + Git publish + NAS V<version/><context="µDisplayName"/>':
    "Docs + Git-Veröffentlichung + NAS V<version/>",
  'Copies BlueShift sources and valuable local/reference assets (incl. gitignored 3dprint/vendor CAD when present) to up to three NAS folders. Zero targets is valid. Skips node_modules, .pio, disposable caches, .env.<context="µDescription"/>':
    "Kopiert BlueShift-Quellen und wertvolle Lokal-/Referenz-Assets (inkl. gitignorierter 3dprint/Vendor-CAD) auf bis zu drei NAS-Ordner. Null Ziele sind gültig. Lässt node_modules, .pio, disposable Caches, .env weg.",
  'Runs docs, Git publication checkpoint (backup:git), then NAS backup.<context="µDescription"/>':
    "Führt docs, Git-Veröffentlichungs-Checkpoint (backup:git), dann NAS-Backup aus.",
  'NAS_TARGET_1..3 / config/nas.targets.local / µGulp form. BLUESHIFT_NAS_DRY_RUN=1 for preview. Gitignore ≠ NAS policy — see docs/backup/nas-policy.md.<context="µTooltip"/>':
    "NAS_TARGET_1..3 / config/nas.targets.local / µGulp-Formular. BLUESHIFT_NAS_DRY_RUN=1 für Vorschau. Gitignore ≠ NAS-Policy — siehe docs/backup/nas-policy.md.",
  'NAS backup destinations<context="task parameter"/>': "NAS-Backup-Ziele",
  'Start backup<context="button text"/>': "Backup starten",
  'Destination 1 (NAS_TARGET_1)<context="task parameter"/>': "Ziel 1 (NAS_TARGET_1)",
  'Destination 2 (NAS_TARGET_2)<context="task parameter"/>': "Ziel 2 (NAS_TARGET_2)",
  'Destination 3 (NAS_TARGET_3)<context="task parameter"/>': "Ziel 3 (NAS_TARGET_3)",
  'Primary backup folder. Leave empty to skip. Up to three destinations. Optional defaults from config/nas.targets.local.<context="task parameter"/>':
    "Primärer Backup-Ordner. Leer lassen zum Überspringen. Bis zu drei Ziele. Optionale Defaults aus config/nas.targets.local.",
  'Second folder when set. Leave empty to skip.<context="task parameter"/>':
    "Zweiter Ordner, falls gesetzt. Leer lassen zum Überspringen.",
  'Third folder when set. Leave empty to skip.<context="task parameter"/>':
    "Dritter Ordner, falls gesetzt. Leer lassen zum Überspringen.",
  'Preview only (robocopy /L)<context="task parameter"/>': "Nur Vorschau (robocopy /L)",
  'List differences without copying or deleting.<context="task parameter"/>':
    "Unterschiede auflisten, ohne zu kopieren oder zu löschen.",
};

/** en-US identity map (keys = values) for scanner / fallback. */
const en = Object.fromEntries(Object.keys(de).map((k) => [k, k]));

writeFileSync(join(outDir, "de-DE.json"), JSON.stringify(de, null, "\t") + "\n", "utf8");
writeFileSync(join(outDir, "en-US.json"), JSON.stringify(en, null, "\t") + "\n", "utf8");
console.log(`Wrote ${Object.keys(de).length} phrases → ${outDir}`);
