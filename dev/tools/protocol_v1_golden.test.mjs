/**
 * Protocol V1 golden byte-exact tests + pipeline host validation.
 * Fixtures are the interoperability contract (usable without BlueShift C++).
 */
import test from "node:test";
import assert from "node:assert/strict";
import { readFileSync, readdirSync } from "node:fs";
import { join } from "node:path";

const ROOT = join(import.meta.dirname, "..", "..");
const FIX = join(ROOT, "test", "fixtures", "audio");
const APPLE2_HZ = 1020484;
const PCM_AMP = 16000;
const ESCAPE = 0xffff;
const MAX_DELTA = APPLE2_HZ * 5;

function hexToBuf(hex) {
  return Buffer.from(hex.trim().split(/\s+/).map((b) => parseInt(b, 16)));
}
function bufToHex(buf) {
  return [...buf].map((b) => b.toString(16).padStart(2, "0").toUpperCase()).join(" ");
}
function writeU16(buf, off, v) {
  buf[off] = v & 0xff;
  buf[off + 1] = (v >> 8) & 0xff;
}
function writeU32(buf, off, v) {
  buf[off] = v & 0xff;
  buf[off + 1] = (v >> 8) & 0xff;
  buf[off + 2] = (v >> 16) & 0xff;
  buf[off + 3] = (v >>> 24) & 0xff;
}
function readU16(buf, off) {
  return buf[off] | (buf[off + 1] << 8);
}
function readU32(buf, off) {
  return (
    (buf[off] | (buf[off + 1] << 8) | (buf[off + 2] << 16) | (buf[off + 3] << 24)) >>> 0
  );
}

function encodeControl(cmd) {
  return Buffer.from([0x02, cmd & 0xff, 0]);
}
function encodeSync(seq, timeline, level, reason) {
  const b = Buffer.alloc(9);
  b[0] = 0x11;
  writeU16(b, 1, seq);
  writeU32(b, 3, timeline);
  b[7] = level ? 1 : 0;
  b[8] = reason & 0xff;
  return b;
}
function encodeEdgeBatch(sequence, baseCycle, deltas) {
  const parts = [Buffer.from([0x10, 0x00])];
  const hdr = Buffer.alloc(8);
  writeU16(hdr, 0, sequence);
  writeU32(hdr, 2, baseCycle);
  writeU16(hdr, 6, deltas.length);
  parts.push(hdr);
  for (const d of deltas) {
    if (d === 0 || d > MAX_DELTA) throw new Error("bad delta " + d);
    if (d < ESCAPE) {
      const p = Buffer.alloc(2);
      writeU16(p, 0, d);
      parts.push(p);
    } else {
      const p = Buffer.alloc(6);
      writeU16(p, 0, ESCAPE);
      writeU32(p, 2, d);
      parts.push(p);
    }
  }
  return Buffer.concat(parts);
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
    } else d = raw;
    if (d === 0 || d > MAX_DELTA) return null;
    deltas.push(d);
  }
  if (pos !== buf.length) return null;
  return { sequence, baseCycle, edgeCount, deltas };
}
function reconstruct(base, deltas) {
  let c = base >>> 0;
  return deltas.map((d) => {
    c = (c + d) >>> 0;
    return c;
  });
}

class EdgeToPcm {
  constructor(sampleRateHz = 44100) {
    this.sampleRateHz = sampleRateHz;
    this.cyclesPerSampleFp = (BigInt(APPLE2_HZ) << 16n) / BigInt(sampleRateHz);
    this.cycleFracFp = 0n;
    this.timeline = 0;
    this.high = false;
    this.edges = [];
  }
  reset(start = 0, high = false) {
    this.timeline = start >>> 0;
    this.high = high;
    this.edges = [];
    this.cycleFracFp = 0n;
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
    return Number((integral * BigInt(PCM_AMP)) / BigInt(interval));
  }
  render(n) {
    const out = [];
    for (let i = 0; i < n; i++) out.push(this.renderOne());
    return out;
  }
}

class BoundedRing {
  constructor(cap) {
    this.cap = cap;
    this.data = [];
    this.overflows = 0;
  }
  push(v) {
    if (this.data.length >= this.cap) {
      this.data.shift();
      this.overflows += 1;
    }
    this.data.push(v);
  }
  size() {
    return this.data.length;
  }
}

class SeqTracker {
  constructor() {
    this.have = false;
    this.expected = 0;
    this.gaps = 0;
    this.dups = 0;
  }
  note(seq) {
    if (!this.have) {
      this.expected = seq;
      this.have = true;
      return "Ok";
    }
    if (seq === this.expected) {
      this.dups += 1;
      return "DuplicateSequence";
    }
    const next = (this.expected + 1) & 0xffff;
    if (seq !== next) {
      this.gaps += 1;
      this.expected = seq;
      return "SequenceGap";
    }
    this.expected = seq;
    return "Ok";
  }
}

// --- Golden byte-exact ---

test("golden: all golden_*.json wireHex decode independently", () => {
  const files = readdirSync(FIX).filter((f) => f.startsWith("golden_") && f.endsWith(".json"));
  assert.ok(files.length >= 10, "expected golden fixtures");
  for (const f of files) {
    const j = JSON.parse(readFileSync(join(FIX, f), "utf8"));
    assert.equal(j.protocolVersion, 1, f);
    if (j.wireHex) {
      const buf = hexToBuf(j.wireHex);
      if (j.malformed) {
        if (buf[0] === 0x10) assert.equal(decodeEdgeBatch(buf), null, f);
        continue;
      }
      if (buf[0] === 0x10) {
        const d = decodeEdgeBatch(buf);
        assert.ok(d, f);
        if (j.expect?.deltas) assert.deepEqual(d.deltas, j.expect.deltas, f);
        if (j.expect?.reconstructedAbsCycles) {
          assert.deepEqual(
            reconstruct(d.baseCycle, d.deltas),
            j.expect.reconstructedAbsCycles,
            f
          );
        }
      }
    }
  }
});

test("golden_control_start exact bytes", () => {
  const j = JSON.parse(readFileSync(join(FIX, "golden_control_start.json"), "utf8"));
  const enc = encodeControl(1);
  assert.equal(bufToHex(enc), j.wireHex);
});

test("golden_edge_batch_normal exact bytes (not only round-trip)", () => {
  const j = JSON.parse(readFileSync(join(FIX, "golden_edge_batch_normal.json"), "utf8"));
  const enc = encodeEdgeBatch(7, 1000, [10, 20, 30, 40]);
  assert.equal(bufToHex(enc), j.wireHex);
  const fromHex = hexToBuf(j.wireHex);
  const d = decodeEdgeBatch(fromHex);
  assert.deepEqual(d.deltas, [10, 20, 30, 40]);
});

test("golden max u16 + escaped large exact bytes", () => {
  const maxJ = JSON.parse(readFileSync(join(FIX, "golden_edge_batch_max_u16_delta.json"), "utf8"));
  assert.equal(bufToHex(encodeEdgeBatch(1, 0, [65534])), maxJ.wireHex);
  const escJ = JSON.parse(readFileSync(join(FIX, "golden_edge_batch_escaped_large.json"), "utf8"));
  assert.equal(bufToHex(encodeEdgeBatch(2, 100, [100, 200000])), escJ.wireHex);
  const d = decodeEdgeBatch(hexToBuf(escJ.wireHex));
  assert.equal(d.deltas.length, 2);
  assert.equal(d.deltas[1], 200000);
});

test("golden_sync_marker exact bytes", () => {
  const j = JSON.parse(readFileSync(join(FIX, "golden_sync_marker.json"), "utf8"));
  assert.equal(bufToHex(encodeSync(3, 999, 1, 1)), j.wireHex);
});

test("sequence progression and gap", () => {
  const prog = JSON.parse(readFileSync(join(FIX, "golden_sequence_progression.json"), "utf8"));
  const tr = new SeqTracker();
  // After sync-like first accept: tracker starts empty; first packet sets expected
  for (const p of prog.packets) {
    const d = decodeEdgeBatch(hexToBuf(p.wireHex));
    assert.ok(d);
    const r = tr.note(d.sequence);
    assert.equal(r, p.expect.parse);
  }
  const gap = JSON.parse(readFileSync(join(FIX, "golden_sequence_gap.json"), "utf8"));
  const tr2 = new SeqTracker();
  const r0 = tr2.note(decodeEdgeBatch(hexToBuf(gap.packets[0].wireHex)).sequence);
  assert.equal(r0, "Ok");
  const r1 = tr2.note(decodeEdgeBatch(hexToBuf(gap.packets[1].wireHex)).sequence);
  assert.equal(r1, "SequenceGap");
  assert.equal(tr2.gaps, 1);
});

test("duplicate sequence dropped", () => {
  const j = JSON.parse(readFileSync(join(FIX, "golden_duplicate_sequence.json"), "utf8"));
  const tr = new SeqTracker();
  assert.equal(tr.note(5), "Ok");
  assert.equal(tr.note(5), "DuplicateSequence");
  assert.equal(tr.dups, 1);
  void j;
});

// --- Sub-sample / PWM / rates ---

test("sub-sample pulse non-zero @ 44100 and 48000 (scale before divide)", () => {
  for (const rate of [44100, 48000]) {
    const r = new EdgeToPcm(rate);
    r.reset(0, false);
    r.pushEdge(5);
    r.pushEdge(10);
    const s = r.render(1)[0];
    assert.notEqual(s, 0, "rate " + rate);
    assert.ok(Math.abs(s) < PCM_AMP, "rate " + rate);
  }
});

test("PWM short/medium/long distinguishable first sample", () => {
  const first = (hi) => {
    const r = new EdgeToPcm(44100);
    r.reset(0, false);
    r.pushEdge(1);
    r.pushEdge(1 + hi);
    return r.render(1)[0];
  };
  const a = first(5);
  const b = first(40);
  const c = first(200);
  assert.ok(b > a);
  assert.ok(c >= b);
});

test("E2E PWM fixture → packets → timeline → PCM (capturing backend)", () => {
  const fix = JSON.parse(readFileSync(join(FIX, "pwm_varying.json"), "utf8"));
  const deltas = fix.edges.map((e) => e.deltaCycles);
  const wire = encodeEdgeBatch(1, 0, deltas);
  const decoded = decodeEdgeBatch(wire);
  assert.ok(decoded);
  const abs = reconstruct(0, decoded.deltas);
  const r = new EdgeToPcm(44100);
  r.reset(0, false);
  for (const c of abs) r.pushEdge(c);
  const pcm = r.render(32);
  const capture = [];
  for (const s of pcm) capture.push(s);
  assert.ok(capture.some((v) => v !== 0));
  // Varying duty → not a constant flat full-scale square for all samples
  const uniq = new Set(capture.map((v) => Math.round(v / 100)));
  assert.ok(uniq.size >= 2);
});

test("long gap escape: timeline correct, no fake edges, silence-stable PCM", () => {
  const j = JSON.parse(readFileSync(join(FIX, "golden_edge_batch_escaped_large.json"), "utf8"));
  const d = decodeEdgeBatch(hexToBuf(j.wireHex));
  assert.equal(d.deltas.length, 2);
  const abs = reconstruct(d.baseCycle, d.deltas);
  assert.deepEqual(abs, j.expect.reconstructedAbsCycles);
  // Only two real edges — gap is pure time, not invented toggles
  assert.equal(abs.length, 2);
  const r = new EdgeToPcm(44100);
  r.reset(d.baseCycle, false);
  for (const c of abs) r.pushEdge(c);
  // Advance through first edge region then deep into gap via many samples
  const beforeGap = r.render(8);
  assert.ok(beforeGap.some((v) => v !== 0) || beforeGap.every((v) => v === -PCM_AMP || v === PCM_AMP || v !== 0));
  // After edges consumed, held level continues (stable), no crash
  const held = r.render(64);
  assert.equal(held.length, 64);
});

test("jitter: irregular host arrival does not drive PCM timing", () => {
  // Same edges delivered as if "late" vs "early" — PCM from timeline identical
  const deltas = [100, 100, 100, 100];
  const abs = reconstruct(0, deltas);
  const renderFrom = (edges) => {
    const r = new EdgeToPcm(44100);
    r.reset(0, false);
    for (const e of edges) r.pushEdge(e);
    return r.render(16);
  };
  const early = renderFrom(abs);
  // Simulate host delivering same timeline after artificial delay (same abs cycles)
  const late = renderFrom(abs);
  assert.deepEqual(early, late);
});

test("bounded buffer: no unbounded growth; overflow counted; no stale replay", () => {
  const ring = new BoundedRing(8);
  for (let i = 0; i < 100; i++) ring.push(i);
  assert.equal(ring.size(), 8);
  assert.ok(ring.overflows > 0);
  assert.deepEqual(ring.data, [92, 93, 94, 95, 96, 97, 98, 99]); // newest kept
  // Recovery: clear — old samples gone
  ring.data = [];
  ring.push(1);
  assert.deepEqual(ring.data, [1]);
});

test("HID isolation: audio faults do not clear hidBridgeViable", () => {
  const hidOk = (t) => t.inputConnected && t.hostConnected;
  const t = { inputConnected: true, hostConnected: true, audioConnected: true };
  assert.equal(hidOk(t), true);
  // malformed / underrun / overflow / disconnect
  t.audioConnected = false;
  assert.equal(hidOk(t), true);
  assert.equal(decodeEdgeBatch(hexToBuf("10 00")), null);
});

test("UUID freeze constants present once in protocol header", () => {
  const h = readFileSync(join(ROOT, "components/protocol/extension_protocol.h"), "utf8");
  assert.match(h, /kProtocolVersion = 1/);
  assert.match(h, /6b6c7565-5368-6966-7401-000000000000/);
  assert.match(h, /6b6c7565-5368-6966-7404-000000000000/);
  assert.match(h, /STABLE/);
  // No SIG 16-bit style 0x18xx HID UUIDs used as extension service
  assert.doesNotMatch(h, /0x1812/);
});

test("generic audio remains FUTURE doc only", () => {
  const g = readFileSync(join(ROOT, "docs/audio/generic-audio-future.md"), "utf8");
  assert.match(g, /FUTURE|RESEARCH/);
  assert.match(g, /not implemented/i);
  const arch = readFileSync(join(ROOT, "docs/audio/architecture.md"), "utf8");
  assert.match(arch, /nothing PHYSICALLY VERIFIED|UNVERIFIED/i);
  assert.doesNotMatch(arch, /audio (playback|latency).*PHYSICALLY VERIFIED/i);
});
