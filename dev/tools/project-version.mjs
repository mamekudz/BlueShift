// ===========================================
// project-version.mjs — display version for µGulp
// ===========================================
//
// Prefer RELEASES.json (canonical history); fall back to package.json.

import { existsSync, readFileSync } from "node:fs";
import { join } from "node:path";
import { FormatVersionString } from "./releases/release-version.mjs";
import { LoadCentralReleases } from "./releases/release-merge.mjs";

/**
 * @param {string} _root
 * @returns {{ main: number, minor: number, revision: number, beta: boolean, label: string, source: string }}
 */
export function GetProjectVersion(_root) {
  const releasesPath = join(_root, "RELEASES.json");
  if (existsSync(releasesPath)) {
    const { releases } = LoadCentralReleases(releasesPath);
    if (releases.length > 0) {
      const newest = releases[0];
      const base = FormatVersionString(newest);
      const beta = newest.beta === true;
      return {
        main: newest.main,
        minor: newest.minor,
        revision: newest.revision,
        beta,
        label: beta ? `${base}ß` : base,
        source: "RELEASES.json",
      };
    }
  }

  const pkgPath = join(_root, "package.json");
  if (existsSync(pkgPath)) {
    const pkg = JSON.parse(readFileSync(pkgPath, "utf8"));
    const raw = String(pkg.version ?? "0.0.0");
    const match = raw.match(/^(\d+)\.(\d+)\.(\d+)/);
    const main = match ? Number(match[1]) : 0;
    const minor = match ? Number(match[2]) : 0;
    const revision = match ? Number(match[3]) : 0;
    const beta = /-dev|-beta|ß/i.test(raw);
    const base = `${main}.${minor}.${revision}`;
    return {
      main,
      minor,
      revision,
      beta,
      label: beta && !raw.includes("ß") ? `${base}ß` : raw.replace(/-dev$/i, "ß"),
      source: "package.json",
    };
  }

  return {
    main: 0,
    minor: 0,
    revision: 0,
    beta: true,
    label: "0.0.0ß",
    source: "fallback",
  };
}

/**
 * Short label for task display names (e.g. "0.5.0ß").
 * @param {string} _root
 */
export function GetProjectVersionLabel(_root) {
  return GetProjectVersion(_root).label;
}
