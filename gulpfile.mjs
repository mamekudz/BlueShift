//================================================================
// BlueShift — µGulp / Gulp task runner
// © 2026 Meinolf Amekudzi
//
// Groups (ESP][-aligned):
//   Firmware — build / rebuild / flash / upload / size / clean
//   Tools    — devices / monitor
//   Docs     — help / docs / format / format:check / check
//   Release  — releases:update / releases:history
//   Git      — backup:git  (publication checkpoint — NOT a backup)
//   Backup   — backup / backup:all  (NAS only; backup:nas = CLI alias only)
//
// Display names use en-US keys + i18x/gulp/{en-US,de-DE}.json.
// Version via µGulp <version/> (RELEASES.json / package.json).
// Default = help (safe — never builds firmware, commits, or touches NAS).
//================================================================

import { dirname } from "node:path";
import { fileURLToPath } from "node:url";
import { existsSync, mkdirSync } from "node:fs";
import { spawn } from "node:child_process";
import gulp from "gulp";
import {
  Log,
  Warn,
  ReportProgress,
  InstallStringExtensions,
  PlaySignal,
  GetParameter,
  RequestForm,
  IsMicroGulp,
  LogAccordion,
  NotifyTasksChanged,
  SetTaskEmphasis,
  GetLid,
} from "gulp-mu-gulp-api";
import { ComposeReadme, README_SOURCE_RELATIVE } from "./dev/tools/readme-compose.mjs";
import {
  AssertNasBackupTarget,
  LoadNasLocalDefaults,
  MirrorNonReproducible,
  NAS_BACKUP_INCLUDE_DIRS,
  NAS_BACKUP_INCLUDE_FILES,
  NAS_BACKUP_MAX_DESTINATIONS,
  ResolveNasTargets,
  VerifyBackupContents,
} from "./dev/tools/nas-backup.mjs";
import { RunGitBackup } from "./dev/tools/git-backup.mjs";
import { RunClangFormat } from "./dev/tools/format.mjs";
import { ReleasesPaths } from "./dev/tools/releases/paths.mjs";
import {
  CountPendingContributorMerges,
  MergeDeveloperReleases,
} from "./dev/tools/releases/release-merge.mjs";
import { BuildReleaseHistoryAccordion } from "./dev/tools/releases/release-history.mjs";
import { GetProjectVersionLabel } from "./dev/tools/project-version.mjs";
import {
  AskPort,
  ListPorts,
  MonitorArgs,
  Pio,
  PIO_ENVS,
  ResolvePioEnv,
  UploadArgs,
} from "./dev/tools/pio.mjs";

InstallStringExtensions();

const rootDir = dirname(fileURLToPath(import.meta.url));

/** i18x placeholders — version from RELEASES.json (ß when beta). */
export const µI18xContext = {
  project: "blueshift",
  product: "BlueShift",
  version: GetProjectVersionLabel(rootDir),
};

/** Dashboard start layout for µGroup sections. */
export const µGroups = {
  collapsed: false,
  groups: {
    'Firmware<context="µGroup"/>': "open",
    'Tools<context="µGroup"/>': "open",
    'Docs<context="µGroup"/>': "collapsed",
    'Release<context="µGroup"/>': "open",
    'Git<context="µGroup"/>': "open",
    'Backup<context="µGroup"/>': "open",
  },
};

/**
 * @param {Function} _task
 * @param {object} _meta
 */
function _Tag(_task, _meta) {
  if (_meta.gulpName) _task.displayName = _meta.gulpName;
  if (typeof _meta.µDisplayName === "function") {
    _task.µDisplayName = _meta.µDisplayName;
  } else if (_meta.µDisplayName) {
    _task.µDisplayName = _meta.µDisplayName.i18xRegister();
  }
  if (typeof _meta.µDescription === "function") {
    _task.µDescription = _meta.µDescription;
  } else if (_meta.µDescription) {
    _task.µDescription = _meta.µDescription.i18xRegister();
  }
  if (typeof _meta.µTooltip === "function") {
    _task.µTooltip = _meta.µTooltip;
  } else if (_meta.µTooltip) {
    _task.µTooltip = _meta.µTooltip.i18xRegister();
  }
  if (_meta.µGroup) _task.µGroup = _meta.µGroup.i18xRegister();
  if (_meta.µIcon != null) _task.µIcon = _meta.µIcon;
  if (_meta.µOrder != null) _task.µOrder = _meta.µOrder;
  if (_meta.µExecutionConcurrency != null) {
    _task.µExecutionConcurrency = _meta.µExecutionConcurrency;
  }
  if (_meta.µExecutionRestrictions) {
    _task.µExecutionRestrictions = _meta.µExecutionRestrictions;
  }
  if (_meta.µParameters) _task.µParameters = _meta.µParameters;
  if (_meta.µAttention != null) _task.µAttention = _meta.µAttention;
  if (_meta.µAttentionTooltip != null) {
    _task.µAttentionTooltip =
      typeof _meta.µAttentionTooltip === "function"
        ? _meta.µAttentionTooltip
        : _meta.µAttentionTooltip.i18xRegister();
  }
  if (_meta.µAttentionWatch != null) {
    _task.µAttentionWatch = _meta.µAttentionWatch;
  }
  return _task;
}

function _ReleaseMergeOptions() {
  const paths = ReleasesPaths(rootDir);
  return {
    developerDir: paths.developerDir,
    releasesPath: paths.releasesPath,
    maxAgeDays: 30,
  };
}

function _PendingReleaseMerges() {
  return CountPendingContributorMerges(_ReleaseMergeOptions());
}

/** Keep i18x keys stable — never bake numbers/locale into the registered phrase. */
function _ReleasesUpdateDisplayName() {
  if (_PendingReleaseMerges() <= 0) {
    return 'Release history up to date V<version/><context="µDisplayName"/>'.i18xRegister();
  }
  return 'Update release history — pending V<version/><context="µDisplayName"/>'.i18xRegister();
}

function firmwareEnv() {
  try {
    const p = GetParameter("env");
    if (p) return ResolvePioEnv(String(p));
  } catch {
    /* outside µGulp */
  }
  return ResolvePioEnv();
}

//================================================================
// Firmware
//================================================================

export async function build() {
  ReportProgress(0, "build");
  const env = firmwareEnv();
  Log('Building BlueShift firmware (env <env/>)…<context="task log"/>', { env });
  await Pio(rootDir, ["run", "-e", env], "pio build");
  ReportProgress(1, "build");
  Log('Build OK.<context="task log"/>');
  PlaySignal("success");
}
_Tag(build, {
  gulpName: "build",
  µDisplayName: 'Build Firmware V<version/><context="µDisplayName"/>',
  µDescription:
    'Compiles BlueShift with PlatformIO. Default env t-lion-idf-debug (BLUESHIFT_PIO_ENV).<context="µDescription"/>',
  µIcon: "\u2692",
  µGroup: 'Firmware<context="µGroup"/>',
  µOrder: 10,
  µExecutionConcurrency: false,
});

export async function flash() {
  ReportProgress(0, "flash-build");
  const env = firmwareEnv();
  const port = await AskPort(rootDir, "Flash port (build + upload)");
  Log('Build + upload → <port/> (env <env/>)…<context="task log"/>', {
    port: port ?? "auto/ini",
    env,
  });
  await Pio(rootDir, ["run", "-e", env], "pio build");
  ReportProgress(0.55, "flash-upload");
  await Pio(rootDir, UploadArgs(port, env), "pio upload");
  ReportProgress(1, "flash-upload");
  Log('Flash finished.<context="task log"/>');
  PlaySignal("success");
}
_Tag(flash, {
  gulpName: "flash",
  µDisplayName: 'Build & Upload V<version/><context="µDisplayName"/>',
  µDescription:
    'Rebuilds and flashes BlueShift firmware in one step.<context="µDescription"/>',
  µTooltip:
    'Set BLUESHIFT_PORT=COMx to skip the port dialog.<context="µTooltip"/>',
  µIcon: "\u26A1",
  µGroup: 'Firmware<context="µGroup"/>',
  µOrder: 15,
  µExecutionConcurrency: false,
  µExecutionRestrictions: { deny: ["upload", "build", "backup"] },
});

export async function upload() {
  ReportProgress(0, "upload");
  const env = firmwareEnv();
  const port = await AskPort(rootDir, "Upload port");
  Log('Uploading → <port/> (env <env/>)…<context="task log"/>', {
    port: port ?? "auto/ini",
    env,
  });
  await Pio(rootDir, UploadArgs(port, env), "pio upload");
  ReportProgress(1, "upload");
  Log('Upload finished.<context="task log"/>');
  PlaySignal("success");
}
_Tag(upload, {
  gulpName: "upload",
  µDisplayName: 'Upload Firmware V<version/><context="µDisplayName"/>',
  µDescription:
    'Flashes the last build. Asks for the COM port.<context="µDescription"/>',
  µTooltip:
    'Set BLUESHIFT_PORT=COMx to skip the port dialog.<context="µTooltip"/>',
  µIcon: "\u2191",
  µGroup: 'Firmware<context="µGroup"/>',
  µOrder: 20,
  µExecutionConcurrency: false,
  µExecutionRestrictions: { deny: ["flash", "monitor", "backup"] },
});

export async function size() {
  ReportProgress(0, "size");
  await Pio(rootDir, ["run", "-e", firmwareEnv(), "-t", "size"], "pio size");
  ReportProgress(1, "size");
}
_Tag(size, {
  gulpName: "size",
  µDisplayName: 'Memory Size V<version/><context="µDisplayName"/>',
  µDescription:
    'Shows RAM / Flash usage of the last firmware build.<context="µDescription"/>',
  µIcon: "\u25A6",
  µGroup: 'Firmware<context="µGroup"/>',
  µOrder: 30,
  µExecutionConcurrency: true,
});

export async function clean() {
  ReportProgress(0, "clean");
  await Pio(rootDir, ["run", "-e", firmwareEnv(), "-t", "clean"], "pio clean");
  ReportProgress(1, "clean");
  Log('Build artefacts removed.<context="task log"/>');
}
_Tag(clean, {
  gulpName: "clean",
  µDisplayName: 'Clean Build V<version/><context="µDisplayName"/>',
  µDescription:
    'Removes .pio/build artefacts for the active PlatformIO env.<context="µDescription"/>',
  µIcon: "\u239A",
  µGroup: 'Firmware<context="µGroup"/>',
  µOrder: 40,
  µExecutionConcurrency: true,
});

export const rebuild = gulp.series(clean, build);
_Tag(rebuild, {
  gulpName: "rebuild",
  µDisplayName: 'Rebuild Firmware V<version/><context="µDisplayName"/>',
  µDescription: 'clean → build for the active PlatformIO env.<context="µDescription"/>',
  µIcon: "\u21BB",
  µGroup: 'Firmware<context="µGroup"/>',
  µOrder: 12,
  µExecutionConcurrency: false,
  µExecutionRestrictions: { deny: ["flash", "upload", "backup"] },
});

export async function firmwareEnvTask() {
  const env = firmwareEnv();
  Log('Active PlatformIO env: <env/><context="task log"/>', { env });
  Log('Known envs: <list/><context="task log"/>', { list: PIO_ENVS.join(", ") });
}
_Tag(firmwareEnvTask, {
  gulpName: "firmware:env",
  µDisplayName: 'Show Firmware Env V<version/><context="µDisplayName"/>',
  µDescription:
    'Prints the active PlatformIO environment and known envs.<context="µDescription"/>',
  µGroup: 'Firmware<context="µGroup"/>',
  µIcon: "\u2139",
  µOrder: 5,
  µExecutionConcurrency: true,
});

//================================================================
// Tools
//================================================================

export async function devices() {
  ReportProgress(0, "devices");
  const ports = await ListPorts(rootDir);
  if (ports.length === 0) {
    Warn('No serial ports detected.<context="task log"/>');
  } else {
    Log('Serial ports:<context="task log"/>');
    for (const p of ports) {
      Log('  <port/><context="task log"/>', { port: p.label });
    }
  }
  await Pio(rootDir, ["device", "list"], "pio device list");
  ReportProgress(1, "devices");
}
_Tag(devices, {
  gulpName: "devices",
  µDisplayName: 'List Devices V<version/><context="µDisplayName"/>',
  µDescription: 'Lists connected serial ports.<context="µDescription"/>',
  µIcon: "\u2398",
  µGroup: 'Tools<context="µGroup"/>',
  µOrder: 10,
  µExecutionConcurrency: true,
});

export async function monitor() {
  const port = await AskPort(rootDir, "Serial monitor port");
  Log('Opening serial monitor (Ctrl+C to stop)…<context="task log"/>');
  await Pio(rootDir, MonitorArgs(port), "pio monitor");
}
_Tag(monitor, {
  gulpName: "monitor",
  µDisplayName: 'Serial Monitor V<version/><context="µDisplayName"/>',
  µDescription:
    'Opens the PlatformIO serial monitor at 115200 baud.<context="µDescription"/>',
  µTooltip:
    'Set BLUESHIFT_PORT=COMx to skip the port dialog.<context="µTooltip"/>',
  µIcon: "\u2399",
  µGroup: 'Tools<context="µGroup"/>',
  µOrder: 20,
  µExecutionConcurrency: false,
});

//================================================================
// Help / Docs
//================================================================

const TASK_HELP = [
  ["help", "Show this task list (default — safe)"],
  ["build / flash / upload", "Firmware (default env t-lion-idf-debug)"],
  ["devices / monitor", "Serial tools"],
  ["docs", "Compose README.md + README.de-DE.md"],
  ["format / format:check / check", "Format and optional pio check"],
  ["releases:update / releases:history", "RELEASES.json history"],
  ["backup:git", "Git publication checkpoint (not a backup)"],
  ["backup", "NAS backup (0–3 destinations)"],
  ["backup:all", "docs → Git publish → NAS"],
];

export async function help() {
  Log('BlueShift µGulp tasks (package: blueshift)<context="task log"/>');
  Log('Default task is help — no firmware build, commit, or NAS access.<context="task log"/>');
  for (const [name, desc] of TASK_HELP) {
    Log(`  ${name.padEnd(28)} ${desc}<context="task log"/>`);
  }
  PlaySignal("success");
}
_Tag(help, {
  gulpName: "help",
  µDisplayName: 'Help V<version/><context="µDisplayName"/>',
  µDescription:
    'Lists BlueShift Gulp tasks. Safe default — no build, commit, or NAS.<context="µDescription"/>',
  µGroup: 'Docs<context="µGroup"/>',
  µIcon: "\u2753",
  µOrder: 1,
  µExecutionConcurrency: true,
});

export async function docs() {
  ReportProgress(0, "docs");
  Log('Composing READMEs from locale sources (default <path/>)<context="task log"/>', {
    path: README_SOURCE_RELATIVE,
  });
  const result = ComposeReadme({ root: rootDir });
  for (const localeResult of result.results ?? []) {
    const rel = localeResult.output.replace(/\\/g, "/").split("/").slice(-1)[0];
    if (localeResult.changed) {
      Log('Wrote <path/> (<bytes format="int"/> bytes)<context="task log"/>', {
        path: rel,
        bytes: localeResult.bytes,
      });
    } else {
      Log(
        '<path/> already up to date (<bytes format="int"/> bytes).<context="task log"/>',
        { path: rel, bytes: localeResult.bytes }
      );
    }
  }
  ReportProgress(1, "docs");
  PlaySignal("success");
}
_Tag(docs, {
  gulpName: "docs",
  µDisplayName: 'Compose README V<version/><context="µDisplayName"/>',
  µDescription:
    'Generates README.md and README.de-DE.md from en-US/de-DE sources and injects the compatibility table from devices.json.<context="µDescription"/>',
  µGroup: 'Docs<context="µGroup"/>',
  µIcon: "\uE915",
  µOrder: 10,
  µExecutionConcurrency: false,
});

export async function format() {
  ReportProgress(0, "format");
  const result = await RunClangFormat(rootDir, {
    checkOnly: false,
    log: (m) => Log(m + '<context="task log"/>'),
    warn: (m) => Warn(m + '<context="task warning"/>'),
  });
  ReportProgress(1, "format");
  if (result.ok) PlaySignal("success");
  else if (result.reason === "missing-clang-format") {
    Warn(
      'Install clang-format (LLVM) or set CLANG_FORMAT to enable formatting.<context="task warning"/>'
    );
  }
  return result;
}
_Tag(format, {
  gulpName: "format",
  µDisplayName: 'Format C/C++ V<version/><context="µDisplayName"/>',
  µDescription:
    'Runs clang-format -i on src/, include/, components/, test/. Requires clang-format on PATH or CLANG_FORMAT.<context="µDescription"/>',
  µGroup: 'Docs<context="µGroup"/>',
  µIcon: "\u2728",
  µOrder: 20,
  µExecutionConcurrency: false,
});

export async function FORMAT_CHECK() {
  ReportProgress(0, "format-check");
  const result = await RunClangFormat(rootDir, {
    checkOnly: true,
    log: (m) => Log(m + '<context="task log"/>'),
    warn: (m) => Warn(m + '<context="task warning"/>'),
  });
  ReportProgress(1, "format-check");
  if (result.ok) PlaySignal("success");
  return result;
}
_Tag(FORMAT_CHECK, {
  gulpName: "format:check",
  µDisplayName: 'Check C/C++ format V<version/><context="µDisplayName"/>',
  µDescription:
    'clang-format --dry-run --Werror on project sources. Soft-skips if clang-format is missing.<context="µDescription"/>',
  µGroup: 'Docs<context="µGroup"/>',
  µIcon: "\u2714",
  µOrder: 21,
  µExecutionConcurrency: false,
});

function _PioAvailable() {
  return new Promise((resolve) => {
    const child = spawn("pio", ["--version"], {
      windowsHide: true,
      stdio: ["ignore", "pipe", "pipe"],
    });
    child.on("error", () => resolve(false));
    child.on("close", (code) => resolve(code === 0));
  });
}

export async function check() {
  ReportProgress(0, "check");
  const fmt = await FORMAT_CHECK();
  ReportProgress(0.5, "check");
  const pioOk = await _PioAvailable();
  if (!pioOk) {
    Warn(
      'PlatformIO CLI not on PATH — skipped pio check.<context="task warning"/>'
    );
  } else {
    Log('Running pio check -e skeleton…<context="task log"/>');
    await new Promise((resolve) => {
      const child = spawn("pio", ["check", "-e", "skeleton", "--skip-packages"], {
        cwd: rootDir,
        windowsHide: true,
        stdio: "inherit",
      });
      child.on("error", () => resolve());
      child.on("close", () => resolve());
    });
  }
  ReportProgress(1, "check");
  if (fmt.ok || fmt.reason === "missing-clang-format") PlaySignal("success");
  return fmt;
}
_Tag(check, {
  gulpName: "check",
  µDisplayName: 'Project checks V<version/><context="µDisplayName"/>',
  µDescription:
    'format:check plus optional PlatformIO cppcheck when available. Does not flash hardware.<context="µDescription"/>',
  µGroup: 'Docs<context="µGroup"/>',
  µIcon: "\u1F50D",
  µOrder: 30,
  µExecutionConcurrency: false,
});

//================================================================
// Release history
//================================================================

export async function RELEASES_UPDATE() {
  ReportProgress(0, "releases-update");
  Log(
    'Merging contributor notes (dev/releases/*.json) into RELEASES.json…<context="task log"/>'
  );
  const summary = MergeDeveloperReleases({
    ..._ReleaseMergeOptions(),
    write: true,
  });
  Log(
    'scanned=<scanned format="int"/> new=<merged format="int"/> duplicates=<duplicates format="int"/> expired=<expired format="int"/> invalid=<invalid format="int"/><context="task log"/>',
    {
      scanned: summary.scanned,
      merged: summary.merged,
      duplicates: summary.duplicates,
      expired: summary.expired,
      invalid: summary.invalid,
    }
  );
  µI18xContext.version = GetProjectVersionLabel(rootDir);
  SetTaskEmphasis("RELEASES_UPDATE", null);
  NotifyTasksChanged();
  ReportProgress(1, "releases-update");
  PlaySignal("success");
  return summary;
}
_Tag(RELEASES_UPDATE, {
  gulpName: "releases:update",
  µDisplayName: () => _ReleasesUpdateDisplayName(),
  µDescription:
    'Merges fresh contributor notes from dev/releases/*.json into RELEASES.json (30-day window, fingerprint duplicates, release-info context tags).<context="µDescription"/>',
  µTooltip:
    'ACTION_AVAILABLE when unmerged contributor notes exist — not a build failure. Safe to re-run (idempotent).<context="µTooltip"/>',
  µGroup: 'Release<context="µGroup"/>',
  µIcon: "\uE915",
  µOrder: 10,
  µExecutionConcurrency: false,
  µAttention: () => _PendingReleaseMerges() > 0,
  µAttentionTooltip:
    'Unmerged contributor release notes available.<context="µAttentionTooltip"/>',
  µAttentionWatch: {
    files: ["RELEASES.json", "dev/releases"],
    intervalMs: 10_000,
  },
});

export async function RELEASES_HISTORY() {
  ReportProgress(0, "releases-history");
  const paths = ReleasesPaths(rootDir);
  let lid = "en-US";
  try {
    const active = typeof GetLid === "function" ? GetLid() : null;
    if (active === "de-DE" || active === "en-US") lid = active;
  } catch {
    /* CLI */
  }
  const accordion = BuildReleaseHistoryAccordion({
    releasesPath: paths.releasesPath,
    root: rootDir,
    lid,
    maxReleases: 12,
  });
  Log(
    'Release history: <count format="int"/> version block(s), <entries format="int"/> info line(s).<context="task log"/>',
    { count: accordion.items.length, entries: accordion.entryCount }
  );
  LogAccordion(accordion);
  ReportProgress(1, "releases-history");
  PlaySignal("success");
  return accordion;
}
_Tag(RELEASES_HISTORY, {
  gulpName: "releases:history",
  µDisplayName: 'Release history V<version/><context="µDisplayName"/>',
  µDescription:
    'Shows localized BlueShift release history from RELEASES.json (date, version, info lines). Does not dump raw JSON.<context="µDescription"/>',
  µTooltip:
    'Read-only history view. Translations: i18x/gulp/releases/{en-US,de-DE}.json.<context="µTooltip"/>',
  µGroup: 'Release<context="µGroup"/>',
  µIcon: "\uE914",
  µOrder: 11,
  µExecutionConcurrency: true,
});

//================================================================
// Backup — NAS form (microGulp / Watchy / esp2)
//================================================================

function _NasBackupParameters() {
  const local = LoadNasLocalDefaults(rootDir);
  return {
    title: 'NAS backup destinations<context="task parameter"/>'.i18xRegister(),
    submitLabel: 'Start backup<context="button text"/>'.i18xRegister(),
    fields: [
      {
        id: "destination1",
        type: "folder",
        required: false,
        default: local.t1,
        label: 'Destination 1 (NAS_TARGET_1)<context="task parameter"/>'.i18xRegister(),
        description:
          'Primary backup folder. Leave empty to skip. Up to three destinations. Optional defaults from config/nas.targets.local.<context="task parameter"/>'.i18xRegister(),
      },
      {
        id: "destination2",
        type: "folder",
        required: false,
        default: local.t2,
        label: 'Destination 2 (NAS_TARGET_2)<context="task parameter"/>'.i18xRegister(),
        description:
          'Second folder when set. Leave empty to skip.<context="task parameter"/>'.i18xRegister(),
      },
      {
        id: "destination3",
        type: "folder",
        required: false,
        default: local.t3,
        label: 'Destination 3 (NAS_TARGET_3)<context="task parameter"/>'.i18xRegister(),
        description:
          'Third folder when set. Leave empty to skip.<context="task parameter"/>'.i18xRegister(),
      },
      {
        id: "dryRun",
        type: "boolean",
        default: local.dryRun,
        label: 'Preview only (robocopy /L)<context="task parameter"/>'.i18xRegister(),
        description:
          'List differences without copying or deleting.<context="task parameter"/>'.i18xRegister(),
      },
    ],
  };
}

async function _ResolveNasForRun() {
  if (
    process.env.NAS_TARGET_1 ||
    process.env.NAS_TARGET_2 ||
    process.env.NAS_TARGET_3 ||
    process.env.BLUESHIFT_NAS_BACKUP_PATHS
  ) {
    return ResolveNasTargets(rootDir, null);
  }

  if (process.env.MICROGULP_PARAMS || IsMicroGulp()) {
    return ResolveNasTargets(rootDir, {
      destination1: GetParameter("destination1", ""),
      destination2: GetParameter("destination2", ""),
      destination3: GetParameter("destination3", ""),
      dryRun: GetParameter("dryRun", false) === true,
    });
  }

  const local = LoadNasLocalDefaults(rootDir);
  if (local.t1 || local.t2 || local.t3 || process.env.BLUESHIFT_NAS_ALLOW_FORM === "1") {
    if (process.env.BLUESHIFT_NAS_ALLOW_FORM === "1") {
      const values = await RequestForm(_NasBackupParameters());
      return ResolveNasTargets(rootDir, {
        destination1: values?.destination1 ?? "",
        destination2: values?.destination2 ?? "",
        destination3: values?.destination3 ?? "",
        dryRun: values?.dryRun === true,
      });
    }
    return ResolveNasTargets(rootDir, {
      destination1: local.t1,
      destination2: local.t2,
      destination3: local.t3,
      dryRun: local.dryRun,
    });
  }

  return ResolveNasTargets(rootDir, null);
}

async function _RunNasBackup() {
  ReportProgress(0, "backup-nas");
  const resolved = await _ResolveNasForRun();

  if (resolved.rejectedExtra > 0) {
    throw new Error(
      `NAS backup allows at most ${NAS_BACKUP_MAX_DESTINATIONS} destinations; extra targets were configured.`
    );
  }

  Log('NAS config source: <source/><context="task log"/>', {
    source: resolved.source,
  });
  Log('Include dirs: <dirs/> | files: <files/><context="task log"/>', {
    dirs: NAS_BACKUP_INCLUDE_DIRS.join(", "),
    files: NAS_BACKUP_INCLUDE_FILES.join(", "),
  });

  if (resolved.destinations.length === 0) {
    Log(
      '[NAS] 0 destinations — valid. Set NAS_TARGET_1..3, config/nas.targets.local, or µGulp form.<context="task log"/>'
    );
    ReportProgress(1, "backup-nas");
    PlaySignal("success");
    return { ok: true, results: [], dryRun: resolved.dryRun };
  }

  if (resolved.dryRun) {
    Log('[NAS] DRY-RUN — preview only.<context="task log"/>');
  }

  /** @type {{ index: number, path: string, status: string, detail?: string }[]} */
  const results = [];

  for (let i = 0; i < resolved.destinations.length; i += 1) {
    const label = `NAS${i + 1}`;
    const destination = resolved.destinations[i];
    ReportProgress(i / resolved.destinations.length, "backup-nas");
    try {
      if (!existsSync(destination)) {
        Warn(`[${label}] unavailable — skipped (${destination})<context="task warning"/>`);
        results.push({
          index: i + 1,
          path: destination,
          status: "unavailable",
          detail: "path does not exist",
        });
        continue;
      }
      const destResolved = AssertNasBackupTarget(destination, rootDir);
      if (!resolved.dryRun) mkdirSync(destResolved, { recursive: true });
      Log(`[${label}] Copying → <path/><context="task log"/>`, { path: destResolved });
      await MirrorNonReproducible(rootDir, destResolved, resolved.dryRun);
      if (!resolved.dryRun) {
        const check = VerifyBackupContents(destResolved);
        if (!check.ok) {
          Warn(
            `[${label}] finished but missing: ${check.missing.join(", ")}<context="task warning"/>`
          );
          results.push({
            index: i + 1,
            path: destResolved,
            status: "incomplete",
            detail: check.missing.join(", "),
          });
          continue;
        }
      }
      Log(`[${label}] OK<context="task log"/>`);
      results.push({ index: i + 1, path: destResolved, status: "OK" });
    } catch (err) {
      Warn(
        `[${label}] unavailable/failed — skipped (${err.message})<context="task warning"/>`
      );
      results.push({
        index: i + 1,
        path: destination,
        status: "failed",
        detail: err.message,
      });
    }
  }

  Log('— NAS backup summary —<context="task log"/>');
  for (const r of results) {
    Log(
      `[NAS${r.index}] ${r.status}${r.detail ? " — " + r.detail : ""} — ${r.path}<context="task log"/>`
    );
  }
  const okCount = results.filter((r) => r.status === "OK").length;
  Log(
    `Done: <ok format="int"/> OK / <total format="int"/> configured (max ${NAS_BACKUP_MAX_DESTINATIONS}).<context="task log"/>`,
    { ok: okCount, total: results.length }
  );

  ReportProgress(1, "backup-nas");
  if (okCount > 0 || results.length === 0) PlaySignal("success");
  return {
    ok: okCount > 0 || results.every((r) => r.status === "unavailable"),
    results,
  };
}

//================================================================
// Git — publication checkpoint (NOT a backup)
//================================================================

export async function BACKUP_GIT() {
  ReportProgress(0, "git-publish");
  const result = await RunGitBackup(rootDir, {
    log: (m) => Log(m + '<context="task log"/>'),
    warn: (m) => Warn(m + '<context="task warning"/>'),
  });
  ReportProgress(1, "git-publish");
  if (result.ok) PlaySignal("success");
  else if (result.reason === "not-a-repo") {
    Warn(
      'Git publish skipped — no .git yet. Run git init when ready.<context="task warning"/>'
    );
  } else {
    Warn('Git publish status: <reason/><context="task warning"/>', {
      reason: result.reason,
    });
  }
  return result;
}
_Tag(BACKUP_GIT, {
  gulpName: "backup:git",
  µDisplayName: 'Publish Git checkpoint V<version/><context="µDisplayName"/>',
  µDescription:
    'Publishes a Git checkpoint commit (CLAUDE.md included). Shows staged files first. Pushes when a remote exists. Never force-pushes. This is publication, not backup — use NAS for private backup.<context="µDescription"/>',
  µGroup: 'Git<context="µGroup"/>',
  µIcon: "\uE902",
  µOrder: 10,
  µExecutionConcurrency: false,
});

//================================================================
// Backup — NAS only (no duplicate alias in the dashboard)
//================================================================

export async function backup() {
  return _RunNasBackup();
}
_Tag(backup, {
  gulpName: "backup",
  µDisplayName: 'Backup to NAS V<version/><context="µDisplayName"/>',
  µDescription:
    'Copies BlueShift sources and valuable local/reference assets (incl. gitignored 3dprint/vendor CAD when present) to up to three NAS folders. Zero targets is valid. Skips node_modules, .pio, disposable caches, .env.<context="µDescription"/>',
  µTooltip:
    'NAS_TARGET_1..3 / config/nas.targets.local / µGulp form. BLUESHIFT_NAS_DRY_RUN=1 for preview. Gitignore ≠ NAS policy — see docs/backup/nas-policy.md.<context="µTooltip"/>',
  µGroup: 'Backup<context="µGroup"/>',
  µIcon: "\uE902",
  µOrder: 30,
  µExecutionConcurrency: false,
  µParameters: _NasBackupParameters(),
});

export async function BACKUP_ALL() {
  ReportProgress(0, "backup-all");
  await docs();
  ReportProgress(0.33, "backup-all");
  await BACKUP_GIT();
  ReportProgress(0.66, "backup-all");
  await backup();
  ReportProgress(1, "backup-all");
  Log('[ALL] Finished.<context="task log"/>');
}
_Tag(BACKUP_ALL, {
  gulpName: "backup:all",
  µDisplayName: 'Docs + Git publish + NAS V<version/><context="µDisplayName"/>',
  µDescription:
    'Runs docs, Git publication checkpoint (backup:git), then NAS backup.<context="µDescription"/>',
  µGroup: 'Backup<context="µGroup"/>',
  µIcon: "\uE902",
  µOrder: 40,
  µExecutionConcurrency: false,
});

//================================================================
// Discovery — safe default
//================================================================

export default help;

gulp.task("default", help);
gulp.task("help", help);
gulp.task("format:check", FORMAT_CHECK);
gulp.task("firmware:env", firmwareEnvTask);
gulp.task("releases:update", RELEASES_UPDATE);
gulp.task("releases:history", RELEASES_HISTORY);
gulp.task("backup:git", BACKUP_GIT);
/** CLI/npm alias only — not exported, so it does not appear in the µGulp dashboard. */
gulp.task("backup:nas", backup);
gulp.task("backup:all", BACKUP_ALL);
