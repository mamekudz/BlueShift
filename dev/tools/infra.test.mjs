/**
 * Infrastructure checks for BlueShift µGulp helpers (no real NAS / no Git commit).
 */
import test from "node:test";
import assert from "node:assert/strict";
import { mkdtempSync, rmSync, existsSync, readFileSync } from "node:fs";
import { join } from "node:path";
import { tmpdir } from "node:os";
import {
  NAS_BACKUP_INCLUDE_FILES,
  NAS_BACKUP_EXCLUDE_DIRS,
  NAS_BACKUP_MAX_DESTINATIONS,
  UniqueNasTargets,
  ResolveNasTargets,
  MirrorNonReproducible,
  VerifyBackupContents,
} from "./nas-backup.mjs";
import {
  BackupCommitMessage,
  GIT_BACKUP_ENSURE_PATHS,
  GIT_BACKUP_NEVER_STAGE,
  RunGitBackup,
} from "./git-backup.mjs";
import { ComposeReadme, FilterChannels, README_SOURCE_RELATIVE } from "./readme-compose.mjs";
import {
  LoadCompatibilityDb,
  RenderCompatibilityTables,
  COMPAT_TABLE_MARKER,
} from "./compatibility-table.mjs";

const ROOT = join(import.meta.dirname, "..", "..");

test("NAS max destinations is three", () => {
  assert.equal(NAS_BACKUP_MAX_DESTINATIONS, 3);
  const { destinations, rejectedExtra } = UniqueNasTargets(["a", "b", "c", "d"]);
  assert.deepEqual(destinations, ["a", "b", "c"]);
  assert.equal(rejectedExtra, 1);
});

test("0 NAS targets from empty config", () => {
  const prev1 = process.env.NAS_TARGET_1;
  const prev2 = process.env.NAS_TARGET_2;
  const prev3 = process.env.NAS_TARGET_3;
  delete process.env.NAS_TARGET_1;
  delete process.env.NAS_TARGET_2;
  delete process.env.NAS_TARGET_3;
  delete process.env.BLUESHIFT_NAS_BACKUP_PATHS;
  try {
    const resolved = ResolveNasTargets(ROOT);
    assert.equal(resolved.destinations.length, 0);
    assert.ok(resolved.source === "none" || resolved.source.includes("nas.targets"));
  } finally {
    if (prev1 !== undefined) process.env.NAS_TARGET_1 = prev1;
    else delete process.env.NAS_TARGET_1;
    if (prev2 !== undefined) process.env.NAS_TARGET_2 = prev2;
    else delete process.env.NAS_TARGET_2;
    if (prev3 !== undefined) process.env.NAS_TARGET_3 = prev3;
    else delete process.env.NAS_TARGET_3;
  }
});

test("CLAUDE.md is in Git ensure + NAS include lists", () => {
  assert.ok(GIT_BACKUP_ENSURE_PATHS.includes("CLAUDE.md"));
  assert.ok(NAS_BACKUP_INCLUDE_FILES.includes("CLAUDE.md"));
  assert.ok(existsSync(join(ROOT, "CLAUDE.md")));
  assert.ok(GIT_BACKUP_NEVER_STAGE.includes("config/nas.targets.local"));
});

test("NAS excludes regenerable trees", () => {
  assert.ok(NAS_BACKUP_EXCLUDE_DIRS.includes("node_modules"));
  assert.ok(NAS_BACKUP_EXCLUDE_DIRS.includes(".pio"));
});

test("checkpoint message format", () => {
  assert.equal(
    BackupCommitMessage(new Date(2026, 8, 25, 19, 7)),
    "backup: BlueShift 2026-09-25 19:07"
  );
});

test("readme filter strips note/website and is deterministic", () => {
  const src =
    "<!-- note\nsecret\n-->\n# Title\n\nvisible\n\n<!-- website\ndrop\n-->\n";
  const a = FilterChannels(src, "git");
  const b = FilterChannels(src, "git");
  assert.equal(a, b);
  assert.match(a, /# Title/);
  assert.match(a, /visible/);
  assert.doesNotMatch(a, /secret/);
  assert.doesNotMatch(a, /drop/);
});

test("ComposeReadme injects compatibility table and is idempotent", () => {
  assert.equal(README_SOURCE_RELATIVE, "dev/docs/readme/de-DE.src.md");
  const first = ComposeReadme({ root: ROOT });
  const second = ComposeReadme({ root: ROOT });
  assert.equal(second.changed, false);
  assert.equal(first.bytes, second.bytes);
  const readme = readFileSync(join(ROOT, "README.md"), "utf8");
  assert.match(readme, /BlueShift/);
  assert.match(readme, /microgulp-ready\.png/);
  assert.match(readme, /blueshift-logo\.png/);
  assert.doesNotMatch(readme, new RegExp(COMPAT_TABLE_MARKER));
  assert.match(readme, /SN30 Pro/);
  assert.match(readme, /Nimbus/);
});

test("compatibility db loads expected initial devices", () => {
  const db = LoadCompatibilityDb(ROOT);
  assert.equal(db.schemaVersion, 1);
  const ids = db.devices.map((d) => d.id);
  assert.ok(ids.includes("8bitdo-sn30-pro"));
  assert.ok(ids.includes("steelseries-nimbus-69070"));
  assert.ok(ids.includes("xbox-controller-1697"));
  assert.ok(ids.includes("terios-t3-shanwan-bm769"));
  assert.ok(ids.includes("vr-park-controller"));
  const table = RenderCompatibilityTables(db);
  assert.match(table, /BlueShift/);
  assert.match(table, /ESP\]\[/);
});

test("NAS mirror includes CLAUDE.md and skips .pio/node_modules", async () => {
  const dest = mkdtempSync(join(tmpdir(), "blueshift-nas-"));
  const missingTarget = join(tmpdir(), "blueshift-nas-missing-" + Date.now());
  try {
    await MirrorNonReproducible(ROOT, dest, false);
    assert.ok(existsSync(join(dest, "CLAUDE.md")), "CLAUDE.md copied");
    assert.ok(existsSync(join(dest, "platformio.ini")));
    assert.ok(existsSync(join(dest, "docs", "compatibility", "devices.json")));
    assert.ok(existsSync(join(dest, "dev", "docs", "readme", "de-DE.src.md")));
    assert.equal(existsSync(join(dest, "node_modules")), false);
    assert.equal(existsSync(join(dest, ".pio")), false);
    const check = VerifyBackupContents(dest);
    assert.equal(check.ok, true, check.missing.join(","));
    assert.equal(existsSync(missingTarget), false);
  } finally {
    rmSync(dest, { recursive: true, force: true });
  }
});

test("NAS dry-run with example targets does not require real NAS", async () => {
  const t1 = mkdtempSync(join(tmpdir(), "blueshift-nas1-"));
  const t2 = join(tmpdir(), "blueshift-nas2-missing-" + Date.now());
  const t3 = mkdtempSync(join(tmpdir(), "blueshift-nas3-"));
  const prev = {
    1: process.env.NAS_TARGET_1,
    2: process.env.NAS_TARGET_2,
    3: process.env.NAS_TARGET_3,
    dry: process.env.BLUESHIFT_NAS_DRY_RUN,
  };
  process.env.NAS_TARGET_1 = t1;
  process.env.NAS_TARGET_2 = t2;
  process.env.NAS_TARGET_3 = t3;
  process.env.BLUESHIFT_NAS_DRY_RUN = "1";
  try {
    const resolved = ResolveNasTargets(ROOT, null);
    assert.equal(resolved.destinations.length, 3);
    assert.equal(resolved.dryRun, true);
    assert.equal(existsSync(t2), false);
    // dry-run mirror against existing target only
    await MirrorNonReproducible(ROOT, t1, true);
  } finally {
    if (prev[1] !== undefined) process.env.NAS_TARGET_1 = prev[1];
    else delete process.env.NAS_TARGET_1;
    if (prev[2] !== undefined) process.env.NAS_TARGET_2 = prev[2];
    else delete process.env.NAS_TARGET_2;
    if (prev[3] !== undefined) process.env.NAS_TARGET_3 = prev[3];
    else delete process.env.NAS_TARGET_3;
    if (prev.dry !== undefined) process.env.BLUESHIFT_NAS_DRY_RUN = prev.dry;
    else delete process.env.BLUESHIFT_NAS_DRY_RUN;
    rmSync(t1, { recursive: true, force: true });
    rmSync(t3, { recursive: true, force: true });
  }
});

test("git backup dry-run does not commit", async () => {
  const result = await RunGitBackup(ROOT, {
    dryRun: true,
    log: () => {},
    warn: () => {},
  });
  assert.equal(result.dryRun, true);
  assert.ok(result.reason === "dry-run" || result.reason === "not-a-repo");
  assert.match(result.message, /^backup: BlueShift /);
});
