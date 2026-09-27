// ===========================================
// nas-backup.mjs — up to 3 NAS destinations for BlueShift
// ===========================================
//
// Permanent principle:
//   GIT eligibility, NAS backup eligibility, and redistribution permission
//   are THREE INDEPENDENT properties.
//
// .gitignore protects the repository.
// NAS backup protects the working environment and valuable local assets.
// Do NOT use .gitignore as the NAS include/exclude policy.
//
// Policy: explicit include lists (not "git ls-files", not "everything not ignored").

import { spawn } from "node:child_process";
import {
  copyFileSync,
  existsSync,
  mkdirSync,
  readFileSync,
  readdirSync,
  statSync,
  writeFileSync,
} from "node:fs";
import { dirname, join, resolve } from "node:path";

export const NAS_BACKUP_MAX_DESTINATIONS = 3;

/**
 * Asset class labels for BACKUP_MANIFEST.json (private; not published).
 * @typedef {'SOURCE'|'LOCAL_REFERENCE'|'HARDWARE_EVIDENCE'|'CAD_SOURCE'|'USER_PRIVATE'|'VERIFIED_ARTIFACT'} NasAssetClass
 */

/** Disposable / regenerable trees — never NAS-backed. */
export const NAS_BACKUP_EXCLUDE_DIRS = Object.freeze([
  "node_modules",
  ".pio",
  ".cache",
  ".microgulp",
  "tmp",
  "temp",
  "__pycache__",
]);

/** Disposable / secret filenames — never NAS-backed. */
export const NAS_BACKUP_EXCLUDE_FILES = Object.freeze([
  "Thumbs.db",
  ".DS_Store",
  "desktop.ini",
  "compile_commands.json",
  ".env",
]);

/**
 * Explicit project trees to mirror.
 * Includes gitignored content inside these trees (vendor STEP, Plasticity, etc.).
 */
export const NAS_BACKUP_INCLUDE_DIRS = Object.freeze([
  "src",
  "include",
  "components",
  "boards",
  "docs",
  "dev",
  "i18x",
  "config",
  "test",
  "3dprint",
  "spike",
  "partitions",
  "sdkconfig.d",
  "_refs",
  "_physical",
  "releases",
  ".git",
]);

export const NAS_BACKUP_INCLUDE_FILES = Object.freeze([
  "CLAUDE.md",
  "README.md",
  "README.de-DE.md",
  "THIRD_PARTY_LICENSES.md",
  "LICENSE.md",
  "RELEASES.json",
  "platformio.ini",
  "package.json",
  "package-lock.json",
  "gulpfile.mjs",
  ".gitignore",
  ".editorconfig",
  ".clang-format",
  "CMakeLists.txt",
]);

/** Relative paths that must exist after a successful backup for verify. */
export const NAS_BACKUP_REQUIRED_RELATIVE = Object.freeze([
  "CLAUDE.md",
  "platformio.ini",
  "package.json",
  "gulpfile.mjs",
  "RELEASES.json",
  "src/main.cpp",
  "docs/compatibility/devices.json",
  "3dprint/ENCLOSURE-SPEC.md",
]);

/**
 * Classification hints for the private manifest (path prefix → class).
 * First match wins.
 */
export const NAS_BACKUP_CLASS_RULES = Object.freeze([
  { prefix: "_refs/", className: "LOCAL_REFERENCE" },
  { prefix: "_physical/", className: "USER_PRIVATE" },
  { prefix: "releases/", className: "VERIFIED_ARTIFACT" },
  { prefix: "docs/hardware/evidence/", className: "HARDWARE_EVIDENCE" },
  { prefix: "3dprint/", className: "CAD_SOURCE" },
  { prefix: "config/", className: "USER_PRIVATE" },
  { prefix: ".git/", className: "SOURCE" },
]);

/**
 * @param {string} _relPosix
 * @returns {NasAssetClass}
 */
export function ClassifyBackupRelativePath(_relPosix) {
  const rel = String(_relPosix ?? "").replace(/\\/g, "/");
  for (const rule of NAS_BACKUP_CLASS_RULES) {
    if (rel === rule.prefix.slice(0, -1) || rel.startsWith(rule.prefix)) {
      return /** @type {NasAssetClass} */ (rule.className);
    }
  }
  return "SOURCE";
}

/**
 * Independent property model (documentation helper for tests/docs).
 */
export const NAS_VS_GIT_POLICY = Object.freeze({
  principle:
    "GIT_TRACKED, NAS_BACKED_UP, and REDISTRIBUTABLE are independent properties.",
  gitignoreIsNotNasPolicy: true,
  valuableLocalAssets: {
    GIT_TRACKED: "NO when redistribution unclear",
    NAS_BACKED_UP: "YES",
    REDISTRIBUTABLE: "UNKNOWN/NO until audited",
  },
  disposableBuild: {
    GIT_TRACKED: "NO",
    NAS_BACKED_UP: "NO",
    examples: ["node_modules/", ".pio/", ".cache/", "tmp/", "temp/"],
  },
});

/**
 * @param {Iterable<string>} _paths
 * @returns {{ destinations: string[], rejectedExtra: number }}
 */
export function UniqueNasTargets(_paths) {
  const seen = new Set();
  const list = [];
  let rejectedExtra = 0;
  for (const raw of _paths) {
    const path = String(raw ?? "").trim();
    if (!path) continue;
    const key = path.toLowerCase();
    if (seen.has(key)) continue;
    seen.add(key);
    if (list.length >= NAS_BACKUP_MAX_DESTINATIONS) {
      rejectedExtra += 1;
      continue;
    }
    list.push(path);
  }
  return { destinations: list, rejectedExtra };
}

/**
 * @param {string} _text
 */
function _ParseEnvFile(_text) {
  /** @type {Record<string, string>} */
  const map = {};
  for (const line of String(_text).split(/\r?\n/)) {
    const trimmed = line.trim();
    if (!trimmed || trimmed.startsWith("#")) continue;
    const eq = trimmed.indexOf("=");
    if (eq <= 0) continue;
    const key = trimmed.slice(0, eq).trim();
    let value = trimmed.slice(eq + 1).trim();
    if (
      (value.startsWith('"') && value.endsWith('"')) ||
      (value.startsWith("'") && value.endsWith("'"))
    ) {
      value = value.slice(1, -1);
    }
    map[key] = value;
  }
  return map;
}

/**
 * @param {string} _root
 * @returns {{ t1: string, t2: string, t3: string, dryRun: boolean }}
 */
export function LoadNasLocalDefaults(_root) {
  const localPath = join(_root, "config", "nas.targets.local");
  if (!existsSync(localPath)) {
    return { t1: "", t2: "", t3: "", dryRun: false };
  }
  const map = _ParseEnvFile(readFileSync(localPath, "utf8"));
  return {
    t1: String(map.NAS_TARGET_1 ?? "").trim(),
    t2: String(map.NAS_TARGET_2 ?? "").trim(),
    t3: String(map.NAS_TARGET_3 ?? "").trim(),
    dryRun: map.BLUESHIFT_NAS_DRY_RUN === "1",
  };
}

/**
 * Resolve 0–3 NAS targets.
 * Precedence:
 *   1. env NAS_TARGET_1..3
 *   2. env BLUESHIFT_NAS_BACKUP_PATHS (|/;)
 *   3. µParameters / form slots destination1..3
 *   4. config/nas.targets.local
 *
 * @param {string} _root
 * @param {{
 *   destination1?: string,
 *   destination2?: string,
 *   destination3?: string,
 *   dryRun?: boolean,
 * } | null} [_form]
 */
export function ResolveNasTargets(_root, _form = null) {
  const envDry =
    process.env.BLUESHIFT_NAS_DRY_RUN === "1" ||
    process.env.BLUESHIFT_NAS_BACKUP_DRY_RUN === "1";

  const fromEnv = [
    process.env.NAS_TARGET_1,
    process.env.NAS_TARGET_2,
    process.env.NAS_TARGET_3,
  ].map((v) => String(v ?? "").trim());

  if (fromEnv.some(Boolean)) {
    const { destinations, rejectedExtra } = UniqueNasTargets(fromEnv);
    return { destinations, dryRun: envDry, rejectedExtra, source: "env" };
  }

  const pathsEnv = String(process.env.BLUESHIFT_NAS_BACKUP_PATHS ?? "").trim();
  if (pathsEnv) {
    const { destinations, rejectedExtra } = UniqueNasTargets(
      pathsEnv.split(/[|;]/)
    );
    return {
      destinations,
      dryRun: envDry,
      rejectedExtra,
      source: "BLUESHIFT_NAS_BACKUP_PATHS",
    };
  }

  if (_form) {
    const { destinations, rejectedExtra } = UniqueNasTargets([
      _form.destination1,
      _form.destination2,
      _form.destination3,
    ]);
    return {
      destinations,
      dryRun: envDry || _form.dryRun === true,
      rejectedExtra,
      source: "form",
    };
  }

  const local = LoadNasLocalDefaults(_root);
  if (local.t1 || local.t2 || local.t3) {
    const { destinations, rejectedExtra } = UniqueNasTargets([
      local.t1,
      local.t2,
      local.t3,
    ]);
    return {
      destinations,
      dryRun: envDry || local.dryRun,
      rejectedExtra,
      source: "config/nas.targets.local",
    };
  }

  return {
    destinations: [],
    dryRun: envDry,
    rejectedExtra: 0,
    source: "none",
  };
}

/**
 * @param {string} _destination
 * @param {string} _sourceResolved
 */
export function AssertNasBackupTarget(_destination, _sourceResolved) {
  if (!existsSync(_destination)) {
    throw new Error(
      `destination missing: "${_destination}" (create the folder first; no silent local fallback)`
    );
  }
  const destResolved = resolve(_destination);
  const src = _sourceResolved.toLowerCase();
  const dest = destResolved.toLowerCase();
  const srcPrefix = src.replace(/[\\/]+$/, "") + "\\";
  const destPrefix = dest.replace(/[\\/]+$/, "") + "\\";
  if (
    src === dest ||
    src.startsWith(destPrefix) ||
    dest.startsWith(srcPrefix) ||
    /^[a-z]:[\\/]?$/i.test(destResolved) ||
    /^\\\\[^\\]+\\[^\\]+[\\]?$/.test(destResolved)
  ) {
    throw new Error(
      "destination must be a dedicated, non-overlapping backup folder, not a drive or share root"
    );
  }
  return destResolved;
}

/**
 * @param {string} _cmd
 * @param {string[]} _args
 * @returns {Promise<number>}
 */
function _RunRobocopy(_cmd, _args) {
  return new Promise((_resolve, _reject) => {
    const child = spawn(_cmd, _args, {
      windowsHide: true,
      stdio: ["ignore", "pipe", "pipe"],
    });
    let err = "";
    child.stderr.on("data", (c) => {
      err += c.toString();
    });
    child.on("error", _reject);
    child.on("close", (_code) => {
      const code = _code ?? 1;
      if (code >= 8) {
        _reject(new Error(`robocopy failed (${code}): ${err || _args.join(" ")}`));
      } else {
        _resolve(code);
      }
    });
  });
}

/**
 * @param {string} _src
 * @param {string} _dest
 * @param {{ dryRun?: boolean, excludeDirs?: Set<string> }} _opts
 */
function _CopyTree(_src, _dest, _opts) {
  const excludeDirs = _opts.excludeDirs ?? new Set(NAS_BACKUP_EXCLUDE_DIRS);
  if (!existsSync(_src)) return 0;
  const st = statSync(_src);
  if (st.isFile()) {
    if (!_opts.dryRun) {
      mkdirSync(dirname(_dest), { recursive: true });
      copyFileSync(_src, _dest);
    }
    return 1;
  }
  let count = 0;
  if (!_opts.dryRun) mkdirSync(_dest, { recursive: true });
  for (const name of readdirSync(_src)) {
    if (excludeDirs.has(name)) continue;
    if (NAS_BACKUP_EXCLUDE_FILES.includes(name)) continue;
    count += _CopyTree(join(_src, name), join(_dest, name), _opts);
  }
  return count;
}

/**
 * Walk included trees and collect relative paths for classification (private).
 * @param {string} _sourceResolved
 * @returns {string[]} posix-relative paths
 */
export function ListNasBackupRelativePaths(_sourceResolved) {
  /** @type {string[]} */
  const out = [];
  const excludeDirs = new Set(NAS_BACKUP_EXCLUDE_DIRS);

  /**
   * @param {string} abs
   * @param {string} relPosix
   */
  function walk(abs, relPosix) {
    if (!existsSync(abs)) return;
    const st = statSync(abs);
    if (st.isFile()) {
      out.push(relPosix);
      return;
    }
    for (const name of readdirSync(abs)) {
      if (excludeDirs.has(name)) continue;
      if (NAS_BACKUP_EXCLUDE_FILES.includes(name)) continue;
      const childRel = relPosix ? `${relPosix}/${name}` : name;
      walk(join(abs, name), childRel.replace(/\\/g, "/"));
    }
  }

  for (const rel of NAS_BACKUP_INCLUDE_DIRS) {
    walk(join(_sourceResolved, rel), rel.replace(/\\/g, "/"));
  }
  for (const rel of NAS_BACKUP_INCLUDE_FILES) {
    const abs = join(_sourceResolved, rel);
    if (existsSync(abs) && statSync(abs).isFile()) {
      out.push(rel.replace(/\\/g, "/"));
    }
  }
  return out;
}

/**
 * @param {string[]} _relPaths
 */
export function BuildCategorySummary(_relPaths) {
  /** @type {Record<string, number>} */
  const counts = {
    SOURCE: 0,
    LOCAL_REFERENCE: 0,
    HARDWARE_EVIDENCE: 0,
    CAD_SOURCE: 0,
    USER_PRIVATE: 0,
    VERIFIED_ARTIFACT: 0,
  };
  for (const rel of _relPaths) {
    const c = ClassifyBackupRelativePath(rel);
    counts[c] = (counts[c] ?? 0) + 1;
  }
  return counts;
}

/**
 * Write private BACKUP_MANIFEST.json (not for publication).
 * @param {string} _sourceResolved
 * @param {string} _destResolved
 * @param {string[]} [_relPaths]
 */
export function WriteBackupManifest(_sourceResolved, _destResolved, _relPaths) {
  const paths = _relPaths ?? ListNasBackupRelativePaths(_sourceResolved);
  const manifest = {
    project: "BlueShift",
    package: "blueshift",
    createdAt: new Date().toISOString(),
    source: _sourceResolved,
    destination: _destResolved,
    policy: {
      ...NAS_VS_GIT_POLICY,
      note: "Private manifest — do not publish automatically.",
    },
    includeDirs: [...NAS_BACKUP_INCLUDE_DIRS],
    includeFiles: [...NAS_BACKUP_INCLUDE_FILES],
    excludeDirs: [...NAS_BACKUP_EXCLUDE_DIRS],
    excludeFiles: [...NAS_BACKUP_EXCLUDE_FILES],
    categoryCounts: BuildCategorySummary(paths),
    fileCount: paths.length,
  };
  writeFileSync(
    join(_destResolved, "BACKUP_MANIFEST.json"),
    JSON.stringify(manifest, null, 2) + "\n",
    "utf8"
  );
  return manifest;
}

/**
 * Mirror project trees to a NAS destination.
 * Copies gitignored files inside include dirs (vendor STEP, Plasticity, etc.).
 *
 * @param {string} _sourceResolved
 * @param {string} _destResolved
 * @param {boolean} _dryRun
 */
export async function MirrorNonReproducible(
  _sourceResolved,
  _destResolved,
  _dryRun
) {
  if (process.platform === "win32") {
    let lastExit = 0;
    const common = [
      "/FFT",
      "/DST",
      "/R:1",
      "/W:1",
      "/MT:4",
      "/NFL",
      "/NDL",
      "/NP",
      "/XD",
      ...NAS_BACKUP_EXCLUDE_DIRS,
      "/XF",
      ...NAS_BACKUP_EXCLUDE_FILES,
    ];
    if (_dryRun) common.push("/L");

    for (const rel of NAS_BACKUP_INCLUDE_DIRS) {
      const src = join(_sourceResolved, rel);
      if (!existsSync(src)) continue;
      const dest = join(_destResolved, rel);
      lastExit = await _RunRobocopy("robocopy.exe", [src, dest, "/E", ...common]);
    }

    const rootFiles = NAS_BACKUP_INCLUDE_FILES.filter(
      (rel) =>
        !rel.includes("/") &&
        !rel.includes("\\") &&
        existsSync(join(_sourceResolved, rel))
    );
    if (rootFiles.length > 0) {
      lastExit = await _RunRobocopy("robocopy.exe", [
        _sourceResolved,
        _destResolved,
        ...rootFiles,
        "/XO",
        ...common.filter((a) => a !== "/MIR"),
      ]);
    }

    if (!_dryRun) {
      WriteBackupManifest(_sourceResolved, _destResolved);
    }
    return lastExit;
  }

  let files = 0;
  for (const rel of NAS_BACKUP_INCLUDE_DIRS) {
    const src = join(_sourceResolved, rel);
    if (!existsSync(src)) continue;
    files += _CopyTree(src, join(_destResolved, rel), {
      dryRun: _dryRun,
      excludeDirs: new Set(NAS_BACKUP_EXCLUDE_DIRS),
    });
  }
  for (const rel of NAS_BACKUP_INCLUDE_FILES) {
    const src = join(_sourceResolved, rel);
    if (!existsSync(src)) continue;
    files += _CopyTree(src, join(_destResolved, rel), { dryRun: _dryRun });
  }
  if (!_dryRun) {
    WriteBackupManifest(_sourceResolved, _destResolved);
  }
  return files;
}

/**
 * Restore selected trees from a NAS backup into the project root.
 * Intentionally gitignored files remain untracked after restore.
 *
 * @param {string} _backupRoot
 * @param {string} _projectRoot
 * @param {{ dryRun?: boolean, trees?: string[] }} [_opts]
 */
export function RestoreNasTrees(_backupRoot, _projectRoot, _opts = {}) {
  const trees = _opts.trees ?? [
    "3dprint",
    "_refs",
    "_physical",
    "docs/hardware/evidence",
    "releases",
  ];
  const dryRun = _opts.dryRun === true;
  /** @type {string[]} */
  const restored = [];
  for (const rel of trees) {
    const src = join(_backupRoot, rel);
    if (!existsSync(src)) continue;
    const dest = join(_projectRoot, rel);
    if (!dryRun) {
      _CopyTree(src, dest, {
        dryRun: false,
        excludeDirs: new Set(NAS_BACKUP_EXCLUDE_DIRS),
      });
    }
    restored.push(rel.replace(/\\/g, "/"));
  }
  return { restored, dryRun };
}

/**
 * @param {string} _backupRoot
 */
export function VerifyBackupContents(_backupRoot) {
  const missing = [];
  for (const rel of NAS_BACKUP_REQUIRED_RELATIVE) {
    if (!existsSync(join(_backupRoot, rel))) missing.push(rel);
  }
  return { ok: missing.length === 0, missing };
}

/**
 * True if a directory name is treated as disposable for NAS.
 * @param {string} _name
 */
export function IsNasDisposableDirName(_name) {
  return NAS_BACKUP_EXCLUDE_DIRS.includes(_name);
}
