/**
 * Host logic mirrors for environments without g++/clang++.
 * C++ source of truth: components/** + test/host/run_host_tests.cpp
 * When a host toolchain exists: `pio run -e native`
 */
import test from "node:test";
import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { join } from "node:path";

const ROOT = join(import.meta.dirname, "..", "..");

function percentFromVoltageMv(voltageMv) {
  const curve = [
    [4200, 100],
    [4100, 90],
    [4000, 80],
    [3900, 70],
    [3800, 55],
    [3700, 40],
    [3600, 25],
    [3500, 15],
    [3400, 8],
    [3300, 3],
    [3200, 0],
  ];
  if (voltageMv >= curve[0][0]) return 100;
  if (voltageMv <= curve[curve.length - 1][0]) return 0;
  for (let i = 0; i + 1 < curve.length; i += 1) {
    if (voltageMv <= curve[i][0] && voltageMv >= curve[i + 1][0]) {
      const span = curve[i][0] - curve[i + 1][0];
      const t = span > 0 ? (curve[i][0] - voltageMv) / span : 0;
      return Math.round(curve[i][1] + t * (curve[i + 1][1] - curve[i][1]));
    }
  }
  return 255;
}

class BoundedQueue {
  constructor(capacity) {
    this.capacity = capacity;
    this.data = [];
    this.overflows = 0;
  }
  push(item, policy) {
    if (this.data.length < this.capacity) {
      this.data.push(item);
      return true;
    }
    this.overflows += 1;
    if (policy === "DropNewest") return false;
    if (policy === "DropOldest") {
      this.data.shift();
      this.data.push(item);
      return false;
    }
    if (policy === "KeepNewestOnly") {
      this.data = [item];
      return false;
    }
    return false;
  }
}

const BridgeState = {
  Boot: "BOOT",
  Idle: "IDLE",
  PairingInput: "PAIRING_INPUT",
  InputConnected: "INPUT_CONNECTED",
  Bridging: "BRIDGING",
  InputLost: "INPUT_LOST",
};

function applyBridge(state, event, links) {
  let next = state;
  if (event === "BootDone" && state === BridgeState.Boot) next = BridgeState.Idle;
  if (event === "StartPairInput") next = BridgeState.PairingInput;
  if (event === "InputPaired") {
    links.input = true;
    next = links.output ? BridgeState.Bridging : BridgeState.InputConnected;
  }
  if (event === "OutputPaired") {
    links.output = true;
    next = links.input ? BridgeState.Bridging : "OUTPUT_CONNECTED";
  }
  if (event === "InputGone") {
    links.input = false;
    next = links.output ? BridgeState.InputLost : BridgeState.Idle;
  }
  return next;
}

test("battery curve is non-linear and bounded", () => {
  assert.equal(percentFromVoltageMv(4200), 100);
  assert.equal(percentFromVoltageMv(3200), 0);
  const mid = percentFromVoltageMv(3800);
  assert.ok(mid > 0 && mid < 100);
});

test("bounded queue drop policies", () => {
  const q = new BoundedQueue(2);
  assert.equal(q.push(1, "DropNewest"), true);
  assert.equal(q.push(2, "DropNewest"), true);
  assert.equal(q.push(3, "DropNewest"), false);
  assert.equal(q.overflows, 1);
  assert.deepEqual(q.data, [1, 2]);

  const q2 = new BoundedQueue(2);
  q2.push(1, "DropOldest");
  q2.push(2, "DropOldest");
  q2.push(3, "DropOldest");
  assert.deepEqual(q2.data, [2, 3]);
});

test("bridge FSM reaches bridging then input lost", () => {
  const links = { input: false, output: false };
  let s = BridgeState.Boot;
  s = applyBridge(s, "BootDone", links);
  assert.equal(s, BridgeState.Idle);
  s = applyBridge(s, "StartPairInput", links);
  s = applyBridge(s, "InputPaired", links);
  s = applyBridge(s, "OutputPaired", links);
  assert.equal(s, BridgeState.Bridging);
  s = applyBridge(s, "InputGone", links);
  assert.equal(s, BridgeState.InputLost);
});

test("i18n locale tables have matching message counts", () => {
  const en = readFileSync(join(ROOT, "components/i18n/locale_en_us.h"), "utf8");
  const de = readFileSync(join(ROOT, "components/i18n/locale_de_de.h"), "utf8");
  const count = (text) => (text.match(/"/g) || []).length / 2;
  assert.equal(count(en), count(de));
  assert.ok(count(en) >= 20);
  assert.match(en, /"BAT"/);
  assert.match(de, /"AKKU"/);
});

test("t-lion pins documented in board header", () => {
  const pins = readFileSync(
    join(ROOT, "components/hardware/t_lion/board_pins.h"),
    "utf8"
  );
  assert.match(pins, /kOledSda = 21/);
  assert.match(pins, /kBtnUp = 32/);
  assert.match(pins, /kBatteryAdc = 35/);
  assert.match(pins, /TP5400/);
});
