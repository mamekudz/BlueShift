import { readFileSync, writeFileSync, copyFileSync } from "fs";

/**
 * Walk SVG path data and collect absolute points (Illustrator-friendly).
 */
function collectPathPoints(d, xs, ys) {
  const tokens =
    d.match(/[MmLlHhVvCcSsQqTtAaZz]|[-+]?(?:\d*\.\d+|\d+)(?:[eE][-+]?\d+)?/g) ||
    [];
  let i = 0;
  let cmd = null;
  let cx = 0;
  let cy = 0;
  let startX = 0;
  let startY = 0;

  const push = (x, y) => {
    xs.push(x);
    ys.push(y);
  };
  const num = () => parseFloat(tokens[i++]);

  while (i < tokens.length) {
    const t = tokens[i];
    if (/^[MmLlHhVvCcSsQqTtAaZz]$/.test(t)) {
      cmd = t;
      i += 1;
      if (cmd === "Z" || cmd === "z") {
        cx = startX;
        cy = startY;
        continue;
      }
    }
    if (!cmd) break;

    const rel = cmd === cmd.toLowerCase();
    const c = cmd.toUpperCase();

    if (c === "M" || c === "L" || c === "T") {
      const x = num();
      const y = num();
      cx = rel ? cx + x : x;
      cy = rel ? cy + y : y;
      if (c === "M") {
        startX = cx;
        startY = cy;
        cmd = rel ? "l" : "L";
      }
      push(cx, cy);
    } else if (c === "H") {
      const x = num();
      cx = rel ? cx + x : x;
      push(cx, cy);
    } else if (c === "V") {
      const y = num();
      cy = rel ? cy + y : y;
      push(cx, cy);
    } else if (c === "C") {
      const x1 = num();
      const y1 = num();
      const x2 = num();
      const y2 = num();
      const x = num();
      const y = num();
      push(rel ? cx + x1 : x1, rel ? cy + y1 : y1);
      push(rel ? cx + x2 : x2, rel ? cy + y2 : y2);
      cx = rel ? cx + x : x;
      cy = rel ? cy + y : y;
      push(cx, cy);
    } else if (c === "S" || c === "Q") {
      const x1 = num();
      const y1 = num();
      const x = num();
      const y = num();
      push(rel ? cx + x1 : x1, rel ? cy + y1 : y1);
      cx = rel ? cx + x : x;
      cy = rel ? cy + y : y;
      push(cx, cy);
    } else if (c === "A") {
      num();
      num();
      num();
      num();
      num();
      const x = num();
      const y = num();
      cx = rel ? cx + x : x;
      cy = rel ? cy + y : y;
      push(cx, cy);
    } else {
      break;
    }
  }
}

function cropSvg(draftPath, outPath, ariaLabel, { pad = 8, square = false } = {}) {
  copyFileSync(draftPath, outPath);
  const src = readFileSync(outPath, "utf8");
  const xs = [];
  const ys = [];

  for (const m of src.matchAll(/\bd="([^"]+)"/g)) {
    collectPathPoints(m[1], xs, ys);
  }
  if (xs.length === 0) {
    throw new Error(`No path points in ${draftPath}`);
  }

  const minX = Math.min(...xs);
  const maxX = Math.max(...xs);
  const minY = Math.min(...ys);
  const maxY = Math.max(...ys);
  let vbX = Math.floor(minX - pad);
  let vbY = Math.floor(minY - pad);
  let vbW = Math.ceil(maxX + pad) - vbX;
  let vbH = Math.ceil(maxY + pad) - vbY;

  // µGulp identity badge is ~40×28 with object-fit:contain — a square mark
  // fills the height better than a wide A4 crop remnant.
  if (square) {
    const side = Math.max(vbW, vbH);
    const cx = vbX + vbW / 2;
    const cy = vbY + vbH / 2;
    vbX = Math.floor(cx - side / 2);
    vbY = Math.floor(cy - side / 2);
    vbW = side;
    vbH = side;
  }

  let body = src
    .replace(/<\?xml[\s\S]*?\?>/g, "")
    .replace(/<!DOCTYPE[\s\S]*?>/g, "")
    .replace(/<!--[\s\S]*?-->/g, "");
  const open = body.match(/<svg\b[^>]*>/i)[0];
  const inner = body
    .slice(body.indexOf(open) + open.length)
    .replace(/<\/svg>\s*$/i, "");

  writeFileSync(
    outPath,
    `<svg xmlns="http://www.w3.org/2000/svg" viewBox="${vbX} ${vbY} ${vbW} ${vbH}" width="${vbW}" height="${vbH}" role="img" aria-label="${ariaLabel}">\n${inner.trim()}\n</svg>\n`,
  );

  console.log(outPath, {
    points: xs.length,
    viewBox: `${vbX} ${vbY} ${vbW} ${vbH}`,
    square,
  });
}

// Drafts sit centered on an A4 artboard — crop to path bounds, not the page.
cropSvg("dev/drafts/logo.svg", "docs/assets/blueshift-logo.svg", "BlueShift");
cropSvg(
  "dev/drafts/logo_only_name.svg",
  "docs/assets/blueshift-logo-name.svg",
  "BlueShift",
);
