// ===========================================
// readme-compose.mjs — de-DE.src.md → README.md
// BlueShift — µGulp-style compose (aligned with esp2 / microGulp)
// ===========================================
//
// Source (edit this):     dev/docs/readme/de-DE.src.md
// Optional baseline:      dev/docs/readme/de-DE.md
// Later translations:     dev/docs/readme/<lid>.md
// Public Git file:        README.md
//
// Channel markers (outside fenced code):
//   unmarked text       → published
//   <!-- git … -->      → Git README only
//   <!-- website … -->  → skipped for Git README
//   <!-- note … -->     → maintainer only (never published)

import { existsSync, mkdirSync, readFileSync, writeFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import { InjectCompatibilityTable } from "./compatibility-table.mjs";

const PROJECT_ROOT = join(dirname(fileURLToPath(import.meta.url)), "..", "..");
const README_DIR = join(PROJECT_ROOT, "dev", "docs", "readme");
const SOURCE = join(README_DIR, "de-DE.src.md");
const BASELINE = join(README_DIR, "de-DE.md");
const PUBLIC_README = join(PROJECT_ROOT, "README.md");

export const README_SOURCE_RELATIVE = "dev/docs/readme/de-DE.src.md";

/**
 * @param {string} _text
 * @param {"git" | "website"} _channel
 */
export function FilterChannels(_text, _channel = "git") {
  const lines = String(_text).replace(/\r\n/g, "\n").split("\n");
  const out = [];
  let inFence = false;
  let skipDepth = 0;
  let skipKind = null;

  for (const line of lines) {
    const fence = line.trimStart().startsWith("```");
    if (fence) inFence = !inFence;

    if (!inFence) {
      const open = line.match(/^\s*<!--\s*(note|git|website)\b/i);
      if (open && !line.includes("-->")) {
        const kind = open[1].toLowerCase();
        skipKind = kind;
        skipDepth = 1;
        continue;
      }
      if (open && line.includes("-->")) {
        const kind = open[1].toLowerCase();
        if (kind === "note") continue;
        if (kind === "website" && _channel === "git") continue;
        if (kind === "git" && _channel === "website") continue;
        const inner = line.replace(
          /^\s*<!--\s*(note|git|website)\b[^>]*-->\s*/i,
          ""
        );
        if (inner) out.push(inner);
        continue;
      }
      if (skipDepth > 0) {
        if (/^\s*<!--/.test(line)) skipDepth += 1;
        if (/-->/.test(line)) {
          skipDepth -= 1;
          if (skipDepth <= 0) {
            const drop =
              skipKind === "note" ||
              (skipKind === "website" && _channel === "git") ||
              (skipKind === "git" && _channel === "website");
            skipDepth = 0;
            skipKind = null;
            if (drop) continue;
          }
        } else if (
          skipKind === "note" ||
          (skipKind === "website" && _channel === "git") ||
          (skipKind === "git" && _channel === "website")
        ) {
          continue;
        }
        if (skipDepth > 0) {
          if (
            skipKind === "note" ||
            (skipKind === "website" && _channel === "git") ||
            (skipKind === "git" && _channel === "website")
          ) {
            continue;
          }
        }
      }
    }

    out.push(line);
  }

  let body = out.join("\n").replace(/[ \t]+$/gm, "");
  body = body.replace(/\n{3,}/g, "\n\n").replace(/^\n+/, "").replace(/\n*$/, "\n");
  return body;
}

/**
 * @param {{ root?: string }} [_opts]
 */
export function ComposeReadme(_opts = {}) {
  const root = _opts.root ?? PROJECT_ROOT;
  const sourcePath = join(root, "dev", "docs", "readme", "de-DE.src.md");
  const baselinePath = join(root, "dev", "docs", "readme", "de-DE.md");
  const outPath = join(root, "README.md");

  if (!existsSync(sourcePath)) {
    throw new Error(`README source missing: ${sourcePath}`);
  }

  const raw = readFileSync(sourcePath, "utf8");
  let composed = FilterChannels(raw, "git");
  composed = InjectCompatibilityTable(composed, root);

  mkdirSync(dirname(baselinePath), { recursive: true });
  let changed = false;
  for (const [path, next] of [
    [baselinePath, composed],
    [outPath, composed],
  ]) {
    const prev = existsSync(path) ? readFileSync(path, "utf8") : null;
    if (prev !== next) {
      writeFileSync(path, next, "utf8");
      changed = true;
    }
  }

  return {
    source: sourcePath,
    output: outPath,
    bytes: Buffer.byteLength(composed, "utf8"),
    changed,
  };
}

export { SOURCE, BASELINE, PUBLIC_README, README_DIR, PROJECT_ROOT };
