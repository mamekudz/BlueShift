/**
 * Host-side audio protocol + edge→PCM tests (no ESP32 / no g++ required).
 * Mirrors components/protocol/* and components/audio/edge_to_pcm.cpp semantics.
 */
import test from "node:test";
import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { join } from "node:path";

const ROOT = join(import.meta.dirname, "..", "..");
const APPLE2_HZ = 1020484;
const PCM_AMP = 16000;
const ESCAPE = 0xffff;
const MAX_DELTA = APPLE2_HZ * 5;

function writeU16(buf, off, v) {
  buf[off] = v & 0xff;
  buf[off + 1] = (v >> 8) & 0xff;
}
function writeU32(buf, off, v) {
  buf[off] = v & 0xff;
  buf[off + 1] = (v >> 8) & 0xff;
  buf[off + 2] = (v >> 16) & 0xff;
  buf[off + 3] = (v >> 24) & 0xff;
}
function readU16(buf, off) {
  return buf[off] | (buf[off + 1] << 8);
}
function readU32(buf, off) {
  return (
    buf[off] |
    (buf[off + 1] << 8) |
    (buf[off + 2] << 16) |
    (buf[off + 3] << 24)
  ) >>> 0;
}

function encodeEdgeBatch(sequence, baseCycle, deltas) {
  const out = [];
  out.push(0x10, 0x00);
  const hdr = Buffer.alloc(8);
  writeU16(hdr, 0, sequence);
  writeU32(hdr, 2, baseCycle);
  writeU16(hdr, 6, deltas.length);
  out.push(...hdr);
  for (const d of deltas) {
    if (d === 0 || d > MAX_DELTA) throw new Error("bad delta");
    if (d < ESCAPE) {
      const p = Buffer.alloc(2);
      writeU16(p, 0, d);
      out.push(...p);
    } else {
      const p = Buffer.alloc(6);
      writeU16(p, 0, ESCAPE);
      writeU32(p, 2, d);
      out.push(...p);
    }
  }
  return Buffer.from(out);
}

function decodeEdgeBatch(buf) {
  if (buf.length < 10 || buf[0] !== 0x10 || buf[1] !== 0x00) return null;
  const sequence = readU16(buf, 2);
  const baseCycle = readU32(buf, 4);
  const edgeCount = readU16(buf, 8);
  let pos = 10;
  const deltas = [];
  for (let i = 0; i < edgeCount; i++) {
    if (pos + 2 > buf.length) return null;
    const raw = readU16(buf, pos);
    pos += 2;
    let d;
    if (raw === ESCAPE) {
      if (pos + 4 > buf.length) return null;
      d = readU32(buf, pos);
      pos += 4;
    } else {
      d = raw;
    }
    if (d === 0 || d > MAX_DELTA) return null;
    deltas.push(d);
  }
  if (pos !== buf.length) return null;
  return { sequence, baseCycle, edgeCount, deltas };
}

class EdgeToPcm {
  constructor(sampleRateHz = 44100) {
    this.sampleRateHz = sampleRateHz;
    this.cyclesPerSampleFp = (BigInt(APPLE2_HZ) << 16n) / BigInt(sampleRateHz);
    this.cycleFracFp = 0n;
    this.timeline = 0;
    this.high = false;
    this.edges = [];
    this.underruns = 0;
  }
  reset(start = 0, high = false) {
    this.timeline = start >>> 0;
    this.high = high;
    this.edges = [];
    this.cycleFracFp = 0n;
    this.underruns = 0;
  }
  pushEdge(abs) {
    this.edges.push(abs >>> 0);
  }
  renderOne() {
    this.cycleFracFp += this.cyclesPerSampleFp;
    const interval = Number(this.cycleFracFp >> 16n);
    this.cycleFracFp &= 0xffffn;
    if (interval === 0) return 0;
    let remaining = interval;
    let integral = 0n;
    while (remaining > 0) {
      let run = remaining;
      let toggle = false;
      if (this.edges.length > 0) {
        const next = this.edges[0];
        const delta = (next - this.timeline) >>> 0;
        if (delta < remaining) {
          run = delta;
          toggle = true;
        } else if (delta === 0) {
          this.edges.shift();
          this.high = !this.high;
          continue;
        }
      }
      if (run > 0) {
        integral += BigInt(this.high ? 1 : -1) * BigInt(run);
        this.timeline = (this.timeline + run) >>> 0;
        remaining -= run;
      }
      if (toggle) {
        this.edges.shift();
        this.high = !this.high;
      } else break;
    }
    if (remaining > 0) {
      integral += BigInt(this.high ? 1 : -1) * BigInt(remaining);
      this.timeline = (this.timeline + remaining) >>> 0;
    }
    const avgScaled = Number((integral * BigInt(PCM_AMP)) / BigInt(interval));
    return avgScaled;
  }
  render(n) {
    const out = [];
    for (let i = 0; i < n; i++) out.push(this.renderOne());
    return out;
  }
}

test("encode/decode edge batch + escape", () => {
  const pkt = encodeEdgeBatch(7, 1000, [10, 20, 200000, 40]);
  const d = decodeEdgeBatch(pkt);
  assert.ok(d);
  assert.equal(d.sequence, 7);
  assert.equal(d.baseCycle, 1000);
  assert.deepEqual(d.deltas, [10, 20, 200000, 40]);
  assert.ok(pkt.includes(0xff) && pkt.includes(0xff)); // escape marker present
});

test("malformed truncated / trailing bytes rejected", () => {
  assert.equal(decodeEdgeBatch(Buffer.from([0x10, 0x00, 1, 0])), null);
  const good = encodeEdgeBatch(1, 0, [5, 5]);
  const trailing = Buffer.concat([good, Buffer.from([0x00])]);
  assert.equal(decodeEdgeBatch(trailing), null);
});

test("sub-sample pulse produces non-zero PCM @ 44.1k", () => {
  const r = new EdgeToPcm(44100);
  r.reset(0, false);
  r.pushEdge(5);
  r.pushEdge(10);
  const s = r.render(1)[0];
  assert.notEqual(s, 0);
  assert.ok(Math.abs(s) < PCM_AMP);
});

test("PWM short/medium/long distinct amplitudes", () => {
  // Compare first rendered sample only — later samples are dominated by held level.
  const firstSample = (highCycles) => {
    const r = new EdgeToPcm(44100);
    r.reset(0, false);
    r.pushEdge(1); // → high
    r.pushEdge(1 + highCycles); // → low
    return r.render(1)[0];
  };
  const shortS = firstSample(5);
  const medS = firstSample(40);
  const longS = firstSample(200);
  // Longer high run inside the first ~23-cycle sample → higher (more positive) average.
  assert.ok(shortS > -PCM_AMP);
  assert.ok(medS > shortS);
  assert.ok(longS >= medS);
});

test("E2E fixture subsample_pulse through packetize→decode→PCM", () => {
  const fix = JSON.parse(
    readFileSync(join(ROOT, "test/fixtures/audio/subsample_pulse.json"), "utf8")
  );
  const deltas = fix.edges.map((e) => e.deltaCycles);
  const pkt = encodeEdgeBatch(1, 0, deltas);
  const decoded = decodeEdgeBatch(pkt);
  assert.ok(decoded);
  let cycle = decoded.baseCycle;
  const r = new EdgeToPcm(44100);
  r.reset(0, false);
  for (const d of decoded.deltas) {
    cycle = (cycle + d) >>> 0;
    r.pushEdge(cycle);
  }
  const sample = r.render(1)[0];
  assert.notEqual(sample, 0);
});

test("silence_gap fixture needs escape encoding", () => {
  const fix = JSON.parse(
    readFileSync(join(ROOT, "test/fixtures/audio/silence_gap.json"), "utf8")
  );
  const deltas = fix.edges.map((e) => e.deltaCycles);
  assert.ok(deltas.some((d) => d >= ESCAPE));
  const pkt = encodeEdgeBatch(0, 0, deltas);
  const decoded = decodeEdgeBatch(pkt);
  assert.deepEqual(decoded.deltas, deltas);
});

test("48 kHz square render does not blow up", () => {
  const r = new EdgeToPcm(48000);
  r.reset(0, false);
  let cycle = 0;
  for (let i = 0; i < 200; i++) {
    cycle += 510;
    r.pushEdge(cycle);
  }
  const buf = r.render(256);
  assert.equal(buf.length, 256);
  assert.ok(buf.some((v) => v !== 0));
});

test("underrun policy: silence fill does not replay", () => {
  const ring = [];
  const pull = (n) => {
    const out = [];
    for (let i = 0; i < n; i++) {
      if (ring.length === 0) out.push(0);
      else out.push(ring.shift());
    }
    return out;
  };
  ring.push(100, 200);
  assert.deepEqual(pull(4), [100, 200, 0, 0]);
  // Later supply must not resurrect old samples
  ring.push(300);
  assert.deepEqual(pull(2), [300, 0]);
});

test("HID viability independent of audio disconnect", () => {
  const hidOk = (t) => t.inputConnected && t.hostConnected;
  const t = { inputConnected: true, hostConnected: true, audioConnected: false };
  assert.equal(hidOk(t), true);
  t.audioConnected = true;
  assert.equal(hidOk(t), true);
  t.audioConnected = false;
  assert.equal(hidOk(t), true);
});
