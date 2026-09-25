// ===========================================
// compatibility-table.mjs — devices.json → Markdown
// ===========================================

import { existsSync, readFileSync } from "node:fs";
import { join } from "node:path";

export const COMPAT_DB_RELATIVE = "docs/compatibility/devices.json";
export const COMPAT_TABLE_MARKER = "<!-- COMPATIBILITY_TABLE -->";

/**
 * @param {string} _root
 */
export function LoadCompatibilityDb(_root) {
  const path = join(_root, COMPAT_DB_RELATIVE);
  if (!existsSync(path)) {
    throw new Error(`Compatibility database missing: ${path}`);
  }
  return JSON.parse(readFileSync(path, "utf8"));
}

/**
 * Escape pipe characters for Markdown tables.
 * @param {unknown} _value
 */
function _Cell(_value) {
  return String(_value ?? "")
    .replace(/\|/g, "\\|")
    .replace(/\r?\n/g, " ")
    .trim();
}

/**
 * Build Markdown tables distinguishing BlueShift vs direct ESP][ status.
 * @param {object} _db
 */
export function RenderCompatibilityTables(_db) {
  const devices = Array.isArray(_db.devices) ? _db.devices : [];
  const lines = [];

  lines.push(
    "Generated from [`docs/compatibility/devices.json`](docs/compatibility/devices.json). Do not edit this table by hand."
  );
  lines.push("");
  lines.push(
    "**BlueShift** = Classic HID input → BLE HID output bridge candidacy. **ESP][** = direct wireless use on the ESP32-S3 ESP][ project. These are different questions."
  );
  lines.push("");
  lines.push(
    "| Device | Transport | BlueShift | ESP][ direct | Physical test | Notes |"
  );
  lines.push("| --- | --- | --- | --- | --- | --- |");

  for (const d of devices) {
    const name = [d.manufacturer, d.model, d.modelNumber]
      .filter(Boolean)
      .join(" ");
    const transport = Array.isArray(d.transport)
      ? d.transport.join(", ")
      : d.transport;
    lines.push(
      `| ${_Cell(name)} | ${_Cell(transport)} | ${_Cell(d.blueShiftStatus)} | ${_Cell(d.esp2Status)} | ${_Cell(d.physicalTest)} | ${_Cell(d.notes)} |`
    );
  }

  lines.push("");
  lines.push("### Evidence / references");
  lines.push("");
  if (_db.references) {
    for (const [key, url] of Object.entries(_db.references)) {
      lines.push(`- ${key}: ${url}`);
    }
  }
  for (const d of devices) {
    for (const ev of d.evidence ?? []) {
      if (!ev?.summary) continue;
      const link = ev.url ? ` ([link](${ev.url}))` : "";
      lines.push(
        `- **${_Cell([d.manufacturer, d.model].filter(Boolean).join(" "))}**: ${_Cell(ev.summary)}${link}`
      );
    }
  }

  return lines.join("\n").replace(/\n*$/, "\n");
}

/**
 * Replace the compatibility marker in composed README text.
 * @param {string} _markdown
 * @param {string} _root
 */
export function InjectCompatibilityTable(_markdown, _root) {
  const db = LoadCompatibilityDb(_root);
  const table = RenderCompatibilityTables(db);
  if (!_markdown.includes(COMPAT_TABLE_MARKER)) {
    return _markdown;
  }
  return _markdown.split(COMPAT_TABLE_MARKER).join(table);
}
