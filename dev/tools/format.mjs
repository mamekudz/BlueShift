// ===========================================
// format.mjs — clang-format helpers for BlueShift
// ===========================================

import { spawn } from "node:child_process";
import { existsSync, readdirSync, statSync } from "node:fs";
import { join } from "node:path";

const SOURCE_EXTS = new Set([".c", ".cpp", ".h", ".hpp", ".cc", ".cxx"]);

/**
 * @param {string} _root
 * @param {string} _dir
 * @param {string[]} _out
 */
function _Walk(_root, _dir, _out) {
  if (!existsSync(_dir)) return;
  for (const name of readdirSync(_dir)) {
    if (name === "node_modules" || name === ".pio" || name === ".git") continue;
    const full = join(_dir, name);
    const st = statSync(full);
    if (st.isDirectory()) {
      _Walk(_root, full, _out);
      continue;
    }
    const dot = name.lastIndexOf(".");
    if (dot < 0) continue;
    const ext = name.slice(dot).toLowerCase();
    if (SOURCE_EXTS.has(ext)) _out.push(full);
  }
}

/**
 * Project-owned C/C++ paths only (never third-party / .pio).
 * @param {string} _root
 */
export function ListFormatTargets(_root) {
  const out = [];
  for (const rel of ["src", "include", "components", "test"]) {
    _Walk(_root, join(_root, rel), out);
  }
  return out.sort();
}

/**
 * @param {string} _cwd
 * @param {string[]} _args
 * @returns {Promise<{ code: number, stdout: string, stderr: string }>}
 */
function _Run(_cwd, _args) {
  return new Promise((_resolve, _reject) => {
    const child = spawn(_args[0], _args.slice(1), {
      cwd: _cwd,
      windowsHide: true,
      stdio: ["ignore", "pipe", "pipe"],
      shell: false,
    });
    let stdout = "";
    let stderr = "";
    child.stdout.on("data", (c) => {
      stdout += c.toString();
    });
    child.stderr.on("data", (c) => {
      stderr += c.toString();
    });
    child.on("error", _reject);
    child.on("close", (code) => {
      _resolve({ code: code ?? 1, stdout, stderr });
    });
  });
}

/**
 * Resolve clang-format binary. Does not require a global install if PlatformIO
 * or LLVM tools provide one later — documents missing tool clearly.
 * @param {string} _root
 */
export async function ResolveClangFormat(_root) {
  const candidates = [
    process.env.CLANG_FORMAT,
    "clang-format",
    "clang-format.exe",
  ].filter(Boolean);

  for (const bin of candidates) {
    try {
      const probe = await _Run(_root, [bin, "--version"]);
      if (probe.code === 0) {
        return { ok: true, bin, version: probe.stdout.trim() || probe.stderr.trim() };
      }
    } catch {
      // try next
    }
  }
  return {
    ok: false,
    bin: null,
    version: null,
    message:
      "clang-format not found on PATH. Install LLVM clang-format or set CLANG_FORMAT. See docs in README (format tasks).",
  };
}

/**
 * @param {string} _root
 * @param {{ checkOnly?: boolean, log?: (m: string) => void, warn?: (m: string) => void }} [_opts]
 */
export async function RunClangFormat(_root, _opts = {}) {
  const log = _opts.log ?? console.log;
  const warn = _opts.warn ?? console.warn;
  const checkOnly = _opts.checkOnly === true;
  const resolved = await ResolveClangFormat(_root);
  if (!resolved.ok) {
    warn(`[FORMAT] ${resolved.message}`);
    return { ok: false, reason: "missing-clang-format", files: [] };
  }

  const files = ListFormatTargets(_root);
  log(`[FORMAT] ${resolved.version}`);
  log(`[FORMAT] ${files.length} file(s); mode=${checkOnly ? "check" : "write"}`);

  if (files.length === 0) {
    return { ok: true, reason: "no-files", files };
  }

  const args = checkOnly
    ? [resolved.bin, "--dry-run", "--Werror", ...files]
    : [resolved.bin, "-i", ...files];

  // Windows command-line length: batch if needed
  const batchSize = 40;
  for (let i = 0; i < files.length; i += batchSize) {
    const slice = files.slice(i, i + batchSize);
    const batchArgs = checkOnly
      ? [resolved.bin, "--dry-run", "--Werror", ...slice]
      : [resolved.bin, "-i", ...slice];
    const result = await _Run(_root, batchArgs);
    if (result.code !== 0) {
      warn(result.stderr || result.stdout || `clang-format exit ${result.code}`);
      return {
        ok: false,
        reason: checkOnly ? "format-drift" : "format-failed",
        files,
      };
    }
  }

  void args;
  return { ok: true, reason: checkOnly ? "check-ok" : "formatted", files };
}
