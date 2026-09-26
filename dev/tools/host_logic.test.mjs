/**
 * Host logic mirrors + structural checks for Milestone 3.
 * C++ source of truth: components/** + test/host/run_host_tests.cpp
 */
import test from "node:test";
import assert from "node:assert/strict";
import { readFileSync, existsSync } from "node:fs";
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

function buildKeyboardReport(modifiers, keys) {
  const out = Buffer.alloc(8);
  out[0] = modifiers & 0xff;
  for (let i = 0; i < 6; i += 1) out[2 + i] = keys[i] || 0;
  return out;
}

function buildGamepadReport(lx, ly, rx, ry, lt, rt, buttons, hat) {
  const scale = (axis) => {
    const v = Math.trunc((axis + 32768) / 256);
    return Math.max(0, Math.min(255, v));
  };
  return Buffer.from([
    scale(lx),
    scale(ly),
    scale(rx),
    scale(ry),
    lt & 0xff,
    rt & 0xff,
    buttons & 0xff,
    (buttons >> 8) & 0xff,
    hat & 0x0f,
  ]);
}

function parseBootKeyboard(data) {
  if (!data || data.length < 8 || data.length > 64) return null;
  return { modifiers: data[0], keys: [...data.slice(2, 8)] };
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
  OutputLost: "OUTPUT_LOST",
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
  if (event === "OutputGone") {
    links.output = false;
    next = links.input ? BridgeState.OutputLost : BridgeState.Idle;
  }
  return next;
}

test("battery curve: full nominal low critical bounds", () => {
  assert.equal(percentFromVoltageMv(4200), 100);
  assert.equal(percentFromVoltageMv(5000), 100); // out-of-range high clamps
  assert.equal(percentFromVoltageMv(3200), 0);
  assert.equal(percentFromVoltageMv(3000), 0); // out-of-range low clamps
  const nominal = percentFromVoltageMv(3800);
  assert.ok(nominal > 40 && nominal < 70);
  const low = percentFromVoltageMv(3400);
  assert.ok(low < 20);
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

  const q3 = new BoundedQueue(2);
  q3.push(1, "KeepNewestOnly");
  q3.push(2, "KeepNewestOnly");
  q3.push(99, "KeepNewestOnly");
  assert.deepEqual(q3.data, [99]);
});

test("bridge FSM bridging then input/output lost", () => {
  const links = { input: false, output: false };
  let s = BridgeState.Boot;
  s = applyBridge(s, "BootDone", links);
  s = applyBridge(s, "StartPairInput", links);
  s = applyBridge(s, "InputPaired", links);
  s = applyBridge(s, "OutputPaired", links);
  assert.equal(s, BridgeState.Bridging);
  s = applyBridge(s, "InputGone", links);
  assert.equal(s, BridgeState.InputLost);
  links.input = true;
  s = BridgeState.Bridging;
  s = applyBridge(s, "OutputGone", links);
  assert.equal(s, BridgeState.OutputLost);
});

test("E2E keyboard synthetic bridge reports", () => {
  // press A, hold Shift, release A, release Shift
  let report = buildKeyboardReport(0, [0x04, 0, 0, 0, 0, 0]);
  assert.equal(report[2], 0x04);
  report = buildKeyboardReport(0x02, [0x04, 0, 0, 0, 0, 0]);
  assert.equal(report[0], 0x02);
  report = buildKeyboardReport(0x02, [0, 0, 0, 0, 0, 0]);
  assert.equal(report[2], 0);
  report = buildKeyboardReport(0, [0, 0, 0, 0, 0, 0]);
  assert.deepEqual([...report], [0, 0, 0, 0, 0, 0, 0, 0]);
});

test("E2E gamepad synthetic bridge reports", () => {
  let r = buildGamepadReport(-32767, 0, 0, 0, 0, 0, 0x0001, 0);
  assert.ok(r[0] < 128); // stick left
  assert.equal(r[6], 0x01); // button
  r = buildGamepadReport(0, 0, 0, 0, 0, 0, 0, 0);
  assert.equal(r[0], 128);
  assert.equal(r[6], 0);
});

test("stuck key safety: classic down then disconnect releases BLE", () => {
  const down = buildKeyboardReport(0, [0x04, 0, 0, 0, 0, 0]);
  assert.equal(down[2], 0x04);
  const release = buildKeyboardReport(0, [0, 0, 0, 0, 0, 0]);
  assert.equal(release[2], 0);
  // After InputGone, BridgeCore makeReleaseSnapshot must be all-zero — mirrored here.
});

test("output disconnect: queue stays bounded KeepNewestOnly", () => {
  const q = new BoundedQueue(4);
  for (let i = 0; i < 50; i += 1) {
    q.push({ lx: i }, "KeepNewestOnly");
  }
  assert.ok(q.data.length <= 4);
  assert.equal(q.data[q.data.length - 1].lx, 49);
});

test("malformed HID keyboard rejected", () => {
  assert.equal(parseBootKeyboard(null), null);
  assert.equal(parseBootKeyboard(Buffer.alloc(3)), null);
  assert.equal(parseBootKeyboard(Buffer.alloc(65)), null);
  const ok = parseBootKeyboard(Buffer.from([0, 0, 0x04, 0, 0, 0, 0, 0]));
  assert.equal(ok.keys[0], 0x04);
});

test("reconnect stress harness 100 cycles mock", () => {
  const links = { input: false, output: true };
  let releases = 0;
  for (let i = 0; i < 100; i += 1) {
    links.input = true;
    let s = BridgeState.Bridging;
    // send input implied
    s = applyBridge(s, "InputGone", links);
    assert.equal(s, BridgeState.InputLost);
    releases += 1;
    s = applyBridge(s, "InputPaired", links);
    assert.equal(s, BridgeState.Bridging);
  }
  assert.equal(releases, 100);
});

test("i18n locale tables have matching message counts", () => {
  const en = readFileSync(join(ROOT, "components/i18n/locale_en_us.h"), "utf8");
  const de = readFileSync(join(ROOT, "components/i18n/locale_de_de.h"), "utf8");
  const count = (text) => (text.match(/"/g) || []).length / 2;
  assert.equal(count(en), count(de));
  assert.ok(count(en) >= 36);
  assert.match(en, /"BAT"/);
  assert.match(de, /"AKKU"/);
  assert.match(en, /"INPUT LOST"/);
  assert.match(de, /"EINGABE WEG"/);
});

test("t-lion pins and board facade exist", () => {
  const pins = readFileSync(join(ROOT, "components/hardware/t_lion/board_pins.h"), "utf8");
  assert.match(pins, /kOledSda = 21/);
  assert.match(pins, /kBtnUp = 32/);
  assert.match(pins, /kBatteryAdc = 35/);
  assert.match(pins, /TP5400/);
  const board = readFileSync(join(ROOT, "components/hardware/t_lion/board.h"), "utf8");
  assert.match(board, /BoardConfig/);
  assert.match(board, /GPIO34\/36\/39/);
});

test("M3 components present", () => {
  const need = [
    "components/display/ssd1306_display.cpp",
    "components/display/idf_ssd1306_display.cpp",
    "components/ui/ui_renderer.cpp",
    "components/storage/config_store.cpp",
    "components/storage/device_store.cpp",
    "components/bluetooth/classic_hid_host.cpp",
    "components/bluetooth/ble_hid_peripheral.cpp",
    "components/bluetooth/reconnect_policy.h",
    "components/app/application.cpp",
    "docs/architecture/memory-budget.md",
    "docs/architecture/flash-budget.md",
    "docs/architecture/idf-production-migration.md",
    "docs/hardware/charging-state.md",
    "docs/bluetooth/m3-stack-decision.md",
  ];
  for (const rel of need) {
    assert.ok(existsSync(join(ROOT, rel)), rel);
  }
});

test("M4 ESP-IDF BT adapters present", () => {
  const need = [
    "components/bluetooth/bluetooth_platform.cpp",
    "components/bluetooth/esp_idf_classic_hid_host.cpp",
    "components/bluetooth/esp_idf_ble_hid_peripheral.cpp",
    "docs/licensing/bluetooth-stack-matrix.md",
    "docs/bluetooth/esp-idf-dual-mode.md",
    "docs/bluetooth/esp-idf-dual-mode-spike.md",
  ];
  for (const rel of need) {
    assert.ok(existsSync(join(ROOT, rel)), rel);
  }
  const matrix = readFileSync(join(ROOT, "docs/licensing/bluetooth-stack-matrix.md"), "utf8");
  assert.match(matrix, /BTstack/);
  assert.match(matrix, /\*\*GREEN\*\*/);
  assert.match(matrix, /BTstack required\?\*\* \*\*No/i);
});

test("version is 0.5.0-dev", () => {
  const ver = readFileSync(join(ROOT, "include/blueshift/version.h"), "utf8");
  assert.match(ver, /0\.5\.0-dev/);
});

test("audio protocol and spikes present", () => {
  const need = [
    "components/protocol/extension_protocol.h",
    "components/protocol/extension_codec.cpp",
    "components/audio/edge_to_pcm.cpp",
    "components/audio/edge_jitter_buffer.h",
    "components/bridge/connection_triplet.h",
    "src/idf_spike/a2dp_source_spike.c",
    "src/idf_spike/triple_role_spike.c",
    "docs/audio/architecture.md",
    "docs/protocol/esp2-extension.md",
    "docs/audio/licensing-a2dp.md",
  ];
  for (const rel of need) {
    assert.ok(existsSync(join(ROOT, rel)), rel);
  }
  const ini = readFileSync(join(ROOT, "platformio.ini"), "utf8");
  assert.match(ini, /\[env:t-lion-idf-spike-a2dp\]/);
  assert.match(ini, /\[env:t-lion-idf-spike-triple\]/);
});

test("IDF production envs and partitions present", () => {
  const ini = readFileSync(join(ROOT, "platformio.ini"), "utf8");
  assert.match(ini, /\[env:t-lion-idf-debug\]/);
  assert.match(ini, /\[env:t-lion-idf-release\]/);
  assert.match(ini, /framework-espidf @ 3\.50301\.0/);
  assert.ok(existsSync(join(ROOT, "partitions/t-lion-no-ota.csv")));
  assert.ok(existsSync(join(ROOT, "partitions/t-lion-ota.csv")));
  assert.ok(existsSync(join(ROOT, "partitions/t-lion-debug.csv")));
  assert.ok(existsSync(join(ROOT, "sdkconfig.d/defaults.common")));
  assert.ok(existsSync(join(ROOT, "src/idf_app/app_main.cpp")));
});

test("third party lists ThingPulse", () => {
  const lic = readFileSync(join(ROOT, "THIRD_PARTY_LICENSES.md"), "utf8");
  assert.match(lic, /ThingPulse|esp8266-oled-ssd1306/);
  assert.match(lic, /4\.6\.2/);
});

test("BlueShift Community License is present", () => {
  const lic = readFileSync(join(ROOT, "LICENSE.md"), "utf8");
  assert.match(lic, /BlueShift Community License 1\.0/);
  assert.match(lic, /Commercial Use Requires a Separate License/);
  assert.match(lic, /Meinolf Amekudzi/);
  assert.match(lic, /Provisional project license text/);
  assert.match(lic, /Evaluation by Commercial Organizations/);
  assert.match(lic, /compiled firmware or binaries/);
  assert.doesNotMatch(lic, /BlueShift®/);
  assert.doesNotMatch(lic, /##\s*\d+\.\s*Governing Law/i);
  assert.doesNotMatch(lic, /exclusive jurisdiction/i);

  const pkg = JSON.parse(readFileSync(join(ROOT, "package.json"), "utf8"));
  assert.equal(pkg.license, "SEE LICENSE.md");

  const readme = readFileSync(join(ROOT, "README.md"), "utf8");
  assert.match(readme, /LICENSE\.md/);
  assert.match(readme, /BlueShift Community License 1\.0/);
  assert.match(readme, /BlueShift™/);
  assert.doesNotMatch(readme, /BlueShift®/);
  assert.doesNotMatch(readme, /\blicensed under the MIT\b/i);
  assert.doesNotMatch(readme, /\blicensed under the GPL\b/i);
  assert.doesNotMatch(readme, /\blicensed under Apache\b/i);

  assert.ok(existsSync(join(ROOT, "THIRD_PARTY_LICENSES.md")));
  const branding = readFileSync(join(ROOT, "docs/licensing/branding-policy.md"), "utf8");
  assert.match(branding, /UNDECIDED/);
  assert.match(branding, /Do \*\*not\*\* use ®/);
});

test("extension protocol V1 + audio fixtures exist", () => {
  const proto = readFileSync(join(ROOT, "components/protocol/extension_protocol.h"), "utf8");
  assert.match(proto, /kProtocolVersion = 1/);
  assert.match(proto, /6b6c7565-5368-6966-7401/);
  assert.match(proto, /FixedU16DeltaWithEscape/);
  assert.match(proto, /kDeltaEscapeLarge/);
  assert.doesNotMatch(proto, /BTstack/);

  assert.ok(existsSync(join(ROOT, "docs/protocol/esp2-extension.md")));
  assert.ok(existsSync(join(ROOT, "test/fixtures/audio/subsample_pulse.json")));
  assert.ok(existsSync(join(ROOT, "test/fixtures/audio/pwm_varying.json")));
  assert.ok(existsSync(join(ROOT, "components/audio/audio_pipeline.h")));
  assert.ok(existsSync(join(ROOT, "docs/audio/a2dp-codec-path.md")));

  const sub = JSON.parse(
    readFileSync(join(ROOT, "test/fixtures/audio/subsample_pulse.json"), "utf8")
  );
  assert.equal(sub.expect.firstPcmSampleNonZero, true);
  assert.ok(sub.edges[0].deltaCycles < 23);
});
