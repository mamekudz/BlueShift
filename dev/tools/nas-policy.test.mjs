/**
 * NAS vs Git independence — valuable gitignored assets still NAS-backed.
 */
import test from "node:test";
import assert from "node:assert/strict";
import {
  mkdtempSync,
  mkdirSync,
  writeFileSync,
  existsSync,
  rmSync,
  readFileSync,
} from "node:fs";
import { join } from "node:path";
import { tmpdir } from "node:os";
import { spawnSync } from "node:child_process";
import {
  NAS_BACKUP_EXCLUDE_DIRS,
  NAS_BACKUP_INCLUDE_DIRS,
  NAS_VS_GIT_POLICY,
  ClassifyBackupRelativePath,
  MirrorNonReproducible,
  RestoreNasTrees,
  IsNasDisposableDirName,
  WriteBackupManifest,
} from "./nas-backup.mjs";

const ROOT = join(import.meta.dirname, "..", "..");

test("NAS policy separates Git from NAS from redistribution", () => {
  assert.equal(NAS_VS_GIT_POLICY.gitignoreIsNotNasPolicy, true);
  assert.match(NAS_VS_GIT_POLICY.principle, /independent/i);
  assert.ok(NAS_BACKUP_INCLUDE_DIRS.includes("_refs"));
  assert.ok(NAS_BACKUP_INCLUDE_DIRS.includes("_physical"));
  assert.ok(NAS_BACKUP_INCLUDE_DIRS.includes("3dprint"));
  assert.ok(NAS_BACKUP_INCLUDE_DIRS.includes("releases"));
  assert.equal(NAS_BACKUP_EXCLUDE_DIRS.includes("_refs"), false);
  assert.ok(IsNasDisposableDirName("node_modules"));
  assert.ok(IsNasDisposableDirName(".pio"));
  assert.equal(IsNasDisposableDirName("3dprint"), false);
});

test("asset class classification covers local CAD and evidence", () => {
  assert.equal(ClassifyBackupRelativePath("_refs/t-lion/board.step"), "LOCAL_REFERENCE");
  assert.equal(ClassifyBackupRelativePath("3dprint/cad/step/board.step"), "CAD_SOURCE");
  assert.equal(ClassifyBackupRelativePath("3dprint/plasticity/case.plasticity"), "CAD_SOURCE");
  assert.equal(
    ClassifyBackupRelativePath("docs/hardware/evidence/t18_v2.3.pdf"),
    "HARDWARE_EVIDENCE"
  );
  assert.equal(ClassifyBackupRelativePath("_physical/bringup/photo.jpg"), "USER_PRIVATE");
  assert.equal(ClassifyBackupRelativePath("releases/0.5.0/firmware.bin"), "VERIFIED_ARTIFACT");
  assert.equal(ClassifyBackupRelativePath("src/main.cpp"), "SOURCE");
});

test("gitignored vendor STEP is not staged but is NAS-backed", async () => {
  const fixtureRoot = mkdtempSync(join(tmpdir(), "bs-nas-pol-"));
  const dest = mkdtempSync(join(tmpdir(), "bs-nas-dest-"));
  try {
    // Minimal project skeleton for mirror
    for (const d of ["src", "docs/compatibility", "3dprint/cad/step", "dev"]) {
      mkdirSync(join(fixtureRoot, d), { recursive: true });
    }
    writeFileSync(join(fixtureRoot, "CLAUDE.md"), "# test\n");
    writeFileSync(join(fixtureRoot, "platformio.ini"), ";test\n");
    writeFileSync(join(fixtureRoot, "package.json"), "{}\n");
    writeFileSync(join(fixtureRoot, "gulpfile.mjs"), "//\n");
    writeFileSync(join(fixtureRoot, "src/main.cpp"), "int main(){}\n");
    writeFileSync(join(fixtureRoot, "docs/compatibility/devices.json"), "[]\n");
    writeFileSync(join(fixtureRoot, "3dprint/ENCLOSURE-SPEC.md"), "# spec\n");
    writeFileSync(join(fixtureRoot, ".gitignore"), "3dprint/cad/step/**\n!3dprint/cad/step/README.md\n");
    writeFileSync(join(fixtureRoot, "3dprint/cad/step/README.md"), "keep\n");
    writeFileSync(join(fixtureRoot, "3dprint/cad/step/vendor-board.step"), "STEP-FAKE\n");

    mkdirSync(join(fixtureRoot, "3dprint/plasticity"), { recursive: true });
    writeFileSync(join(fixtureRoot, "3dprint/plasticity/case.plasticity"), "PLAS\n");

    mkdirSync(join(fixtureRoot, "_refs/t-lion"), { recursive: true });
    writeFileSync(join(fixtureRoot, "_refs/t-lion/mech.pdf"), "PDF\n");

    mkdirSync(join(fixtureRoot, "docs/hardware/evidence"), { recursive: true });
    writeFileSync(join(fixtureRoot, "docs/hardware/evidence/note.txt"), "ev\n");

    // Disposable dirs must not be mirrored
    mkdirSync(join(fixtureRoot, "node_modules/pkg"), { recursive: true });
    writeFileSync(join(fixtureRoot, "node_modules/pkg/x.js"), "x\n");
    mkdirSync(join(fixtureRoot, ".pio/build"), { recursive: true });
    writeFileSync(join(fixtureRoot, ".pio/build/a.o"), "o\n");
    mkdirSync(join(fixtureRoot, "tmp"), { recursive: true });
    writeFileSync(join(fixtureRoot, "tmp/junk.txt"), "j\n");

    await MirrorNonReproducible(fixtureRoot, dest, false);

    assert.ok(existsSync(join(dest, "3dprint/cad/step/vendor-board.step")), "STEP on NAS");
    assert.ok(existsSync(join(dest, "3dprint/plasticity/case.plasticity")), "Plasticity on NAS");
    assert.ok(existsSync(join(dest, "_refs/t-lion/mech.pdf")), "_refs on NAS");
    assert.ok(existsSync(join(dest, "docs/hardware/evidence/note.txt")), "evidence on NAS");
    assert.equal(existsSync(join(dest, "node_modules")), false);
    assert.equal(existsSync(join(dest, ".pio")), false);
    assert.equal(existsSync(join(dest, "tmp")), false);

    const manifest = JSON.parse(readFileSync(join(dest, "BACKUP_MANIFEST.json"), "utf8"));
    assert.ok(manifest.categoryCounts.CAD_SOURCE >= 1);
    assert.ok(manifest.policy.gitignoreIsNotNasPolicy);

    // Init git in fixture: ignored STEP must not be listed as untracked after check-ignore
    spawnSync("git", ["init"], { cwd: fixtureRoot, windowsHide: true });
    spawnSync("git", ["add", "-A"], { cwd: fixtureRoot, windowsHide: true });
    const ignored = spawnSync(
      "git",
      ["check-ignore", "-v", "3dprint/cad/step/vendor-board.step"],
      { cwd: fixtureRoot, encoding: "utf8", windowsHide: true }
    );
    assert.equal(ignored.status, 0, "STEP must be gitignored");
    const ls = spawnSync("git", ["ls-files", "3dprint/cad/step/vendor-board.step"], {
      cwd: fixtureRoot,
      encoding: "utf8",
      windowsHide: true,
    });
    assert.equal(ls.stdout.trim(), "", "STEP not Git-tracked");
  } finally {
    rmSync(fixtureRoot, { recursive: true, force: true });
    rmSync(dest, { recursive: true, force: true });
  }
});

test("restore of local 3dprint assets leaves Git clean for ignores", async () => {
  const project = mkdtempSync(join(tmpdir(), "bs-nas-proj-"));
  const backup = mkdtempSync(join(tmpdir(), "bs-nas-bak-"));
  try {
    mkdirSync(join(backup, "3dprint/cad/step"), { recursive: true });
    mkdirSync(join(backup, "3dprint/plasticity"), { recursive: true });
    mkdirSync(join(backup, "_refs/vendor"), { recursive: true });
    writeFileSync(join(backup, "3dprint/cad/step/board.step"), "STEP\n");
    writeFileSync(join(backup, "3dprint/plasticity/a.plasticity"), "P\n");
    writeFileSync(join(backup, "_refs/vendor/x.zip"), "Z\n");

    writeFileSync(
      join(project, ".gitignore"),
      [
        "3dprint/cad/step/**",
        "!3dprint/cad/step/README.md",
        "3dprint/plasticity/**",
        "!3dprint/plasticity/README.md",
        "_refs/",
        "",
      ].join("\n")
    );
    writeFileSync(join(project, "README.md"), "r\n");
    spawnSync("git", ["init"], { cwd: project, windowsHide: true });
    spawnSync("git", ["add", "README.md", ".gitignore"], { cwd: project, windowsHide: true });
    spawnSync("git", ["-c", "user.email=t@t", "-c", "user.name=t", "commit", "-m", "i"], {
      cwd: project,
      windowsHide: true,
    });

    const result = RestoreNasTrees(backup, project, { dryRun: false });
    assert.ok(result.restored.includes("3dprint"));
    assert.ok(existsSync(join(project, "3dprint/cad/step/board.step")));
    assert.ok(existsSync(join(project, "3dprint/plasticity/a.plasticity")));
    assert.ok(existsSync(join(project, "_refs/vendor/x.zip")));

    const status = spawnSync("git", ["status", "--porcelain"], {
      cwd: project,
      encoding: "utf8",
      windowsHide: true,
    });
    assert.equal(status.stdout.trim(), "", `git status not clean: ${status.stdout}`);
  } finally {
    rmSync(project, { recursive: true, force: true });
    rmSync(backup, { recursive: true, force: true });
  }
});

test("live repo: tracked hardware evidence exists; nas-policy doc present", () => {
  assert.ok(existsSync(join(ROOT, "docs/backup/nas-policy.md")));
  assert.ok(existsSync(join(ROOT, "docs/hardware/evidence/t18_v2.3.pdf")));
  assert.ok(existsSync(join(ROOT, "3dprint/ENCLOSURE-SPEC.md")));
  const gi = readFileSync(join(ROOT, ".gitignore"), "utf8");
  assert.match(gi, /_refs\//);
  assert.match(gi, /3dprint\/\*\*\/vendor\//);
  assert.match(gi, /\*\.plasticity/);
  assert.match(gi, /NAS-backed|NAS backup/i);
});

test("WriteBackupManifest category helper", () => {
  const dest = mkdtempSync(join(tmpdir(), "bs-man-"));
  try {
    const m = WriteBackupManifest(ROOT, dest, [
      "src/main.cpp",
      "_refs/a.step",
      "3dprint/x.scad",
      "docs/hardware/evidence/t.pdf",
    ]);
    assert.equal(m.categoryCounts.SOURCE, 1);
    assert.equal(m.categoryCounts.LOCAL_REFERENCE, 1);
    assert.equal(m.categoryCounts.CAD_SOURCE, 1);
    assert.equal(m.categoryCounts.HARDWARE_EVIDENCE, 1);
  } finally {
    rmSync(dest, { recursive: true, force: true });
  }
});
