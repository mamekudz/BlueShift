// Host-side unit tests for Milestone 3 pure logic (no ESP32 required).
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "battery/battery_monitor.h"
#include "bluetooth/ble_hid_peripheral.h"
#include "bluetooth/classic_hid_host.h"
#include "bridge/bounded_queue.h"
#include "bridge/bridge_core.h"
#include "display/framebuffer_display.h"
#include "hid/ble_hid_reports.h"
#include "hid/device_parsers.h"
#include "hid/normalized_hid.h"
#include "i18n.h"
#include "input/navigation_input.h"
#include "storage/app_config.h"
#include "storage/config_store.h"
#include "storage/device_store.h"
#include "storage/factory_reset.h"
#include "ui/ui_controller.h"
#include "ui/ui_renderer.h"
#include "bluetooth/bluetooth_error.h"
#include "bluetooth/reconnect_policy.h"
#include "diagnostics/hw_revision.h"
#include "diagnostics/self_test.h"

static int g_failures = 0;

#define CHECK(cond)                                                                 \
    do {                                                                            \
        if (!(cond)) {                                                              \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);              \
            ++g_failures;                                                           \
        }                                                                           \
    } while (0)

static void testBridgeFsm() {
    blueshift::BridgeCore bridge;
    CHECK(bridge.state() == blueshift::BridgeState::Boot);
    CHECK(bridge.apply(blueshift::BridgeEvent::BootDone));
    CHECK(bridge.state() == blueshift::BridgeState::Idle);

    bridge.apply(blueshift::BridgeEvent::StartPairInput);
    CHECK(bridge.state() == blueshift::BridgeState::PairingInput);
    bridge.apply(blueshift::BridgeEvent::InputPaired);
    CHECK(bridge.inputLinked());
    CHECK(bridge.state() == blueshift::BridgeState::InputConnected);

    bridge.apply(blueshift::BridgeEvent::StartPairOutput);
    bridge.apply(blueshift::BridgeEvent::OutputPaired);
    CHECK(bridge.state() == blueshift::BridgeState::Bridging);

    blueshift::NormalizedInput in{};
    in.kind = blueshift::NormalizedInput::Kind::Gamepad;
    in.gamepad.leftX = 1000;
    blueshift::NormalizedInput out{};
    CHECK(bridge.onNormalizedInput(in, out));
    CHECK(out.gamepad.leftX == 1000);

    bridge.apply(blueshift::BridgeEvent::InputGone);
    CHECK(bridge.state() == blueshift::BridgeState::InputLost);
    CHECK(bridge.releaseCount() >= 1);
    auto release = bridge.makeReleaseSnapshot(blueshift::NormalizedInput::Kind::Gamepad);
    CHECK(release.gamepad.leftX == 0);
    CHECK(!bridge.onNormalizedInput(in, out));
}

static void testStuckKeySafety() {
    blueshift::BridgeCore bridge;
    blueshift::BleHidPeripheralSpike ble;
    ble.start();
    ble.mockConnect();
    bridge.apply(blueshift::BridgeEvent::BootDone);
    bridge.apply(blueshift::BridgeEvent::InputPaired);
    bridge.apply(blueshift::BridgeEvent::OutputPaired);
    CHECK(bridge.state() == blueshift::BridgeState::Bridging);

    blueshift::NormalizedInput in{};
    in.kind = blueshift::NormalizedInput::Kind::Keyboard;
    in.keyboard.keys[0] = 0x04; // A
    blueshift::NormalizedInput out{};
    CHECK(bridge.onNormalizedInput(in, out));
    CHECK(ble.sendKeyboard(out.keyboard));
    CHECK(ble.lastKeyboardReport()[2] == 0x04);

    bridge.apply(blueshift::BridgeEvent::InputGone);
    auto release = bridge.makeReleaseSnapshot(blueshift::NormalizedInput::Kind::Keyboard);
    CHECK(!release.keyboard.anyKeyDown());
    CHECK(ble.sendKeyboard(release.keyboard));
    CHECK(ble.lastKeyboardReport()[2] == 0);
}

static void testOutputDisconnectBackpressure() {
    blueshift::BoundedQueue<int, 4> q;
    blueshift::BleHidPeripheralSpike ble;
    ble.start(); // not connected
    for (int i = 0; i < 100; ++i) {
        q.push(i, blueshift::QueueOverflowPolicy::KeepNewestOnly);
        blueshift::KeyboardState ks;
        ks.keys[0] = 0x04;
        ble.sendKeyboard(ks); // dropped while disconnected
    }
    CHECK(q.size() == 1);
    CHECK(ble.droppedWhileDisconnected() >= 100);
}

static void testE2EKeyboardGamepad() {
    blueshift::BridgeCore bridge;
    blueshift::BleHidPeripheralSpike ble;
    blueshift::GenericKeyboardParser kbParser;
    ble.start();
    ble.mockConnect();
    bridge.apply(blueshift::BridgeEvent::BootDone);
    bridge.apply(blueshift::BridgeEvent::InputPaired);
    bridge.apply(blueshift::BridgeEvent::OutputPaired);

    uint8_t raw[8] = {0x02, 0, 0x04, 0, 0, 0, 0, 0}; // Shift+A
    blueshift::RawHidReport rr{0, raw, 8};
    blueshift::KeyboardState ks;
    CHECK(kbParser.parse(rr, ks));
    blueshift::NormalizedInput in{};
    in.kind = blueshift::NormalizedInput::Kind::Keyboard;
    in.keyboard = ks;
    blueshift::NormalizedInput out{};
    CHECK(bridge.onNormalizedInput(in, out));
    CHECK(ble.sendKeyboard(out.keyboard));
    CHECK(ble.lastKeyboardReport()[0] == 0x02);
    CHECK(ble.lastKeyboardReport()[2] == 0x04);

    blueshift::Sn30ProProfile sn30;
    uint8_t grow[8] = {0x01, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80}; // A + centered axes
    blueshift::RawHidReport gr{0, grow, 8};
    blueshift::GamepadState gs;
    CHECK(sn30.parseGamepad(gr, gs));
    in.kind = blueshift::NormalizedInput::Kind::Gamepad;
    in.gamepad = gs;
    CHECK(bridge.onNormalizedInput(in, out));
    CHECK(ble.sendGamepad(out.gamepad));
    CHECK(ble.lastGamepadReport()[6] == 0x01);
}

static void testMalformedHid() {
    blueshift::GenericKeyboardParser kb;
    blueshift::KeyboardState ks;
    blueshift::RawHidReport empty{0, nullptr, 0};
    CHECK(!kb.parse(empty, ks));
    uint8_t tiny[3] = {0, 0, 0};
    blueshift::RawHidReport trunc{0, tiny, 3};
    CHECK(!kb.parse(trunc, ks));
    uint8_t huge[65] = {};
    blueshift::RawHidReport over{0, huge, 65};
    CHECK(!kb.parse(over, ks));

    blueshift::Sn30ProProfile sn30;
    blueshift::GamepadState gs;
    CHECK(!sn30.parseGamepad(trunc, gs));
}

static void testPairingTimeout() {
    blueshift::ClassicHidHostSpike classic;
    classic.start();
    classic.startPairing(50);
    CHECK(classic.getState() == blueshift::ClassicHostState::Pairing);
    classic.tick(1);
    classic.tick(100);
    CHECK(classic.getState() == blueshift::ClassicHostState::Idle);

    blueshift::BleHidPeripheralSpike ble;
    ble.start();
    ble.startPairing(50);
    CHECK(ble.getState() == blueshift::BlePeripheralState::Advertising);
    ble.tick(1);
    ble.tick(100);
    CHECK(ble.getState() == blueshift::BlePeripheralState::Idle);
}

static void testReconnectStress() {
    blueshift::BridgeCore bridge;
    blueshift::BleHidPeripheralSpike ble;
    ble.start();
    ble.mockConnect();
    bridge.apply(blueshift::BridgeEvent::BootDone);
    bridge.apply(blueshift::BridgeEvent::OutputPaired);
    for (int i = 0; i < 100; ++i) {
        bridge.apply(blueshift::BridgeEvent::InputPaired);
        CHECK(bridge.state() == blueshift::BridgeState::Bridging);
        blueshift::NormalizedInput in{};
        in.kind = blueshift::NormalizedInput::Kind::Keyboard;
        in.keyboard.keys[0] = 0x04;
        blueshift::NormalizedInput out{};
        CHECK(bridge.onNormalizedInput(in, out));
        ble.sendKeyboard(out.keyboard);
        bridge.apply(blueshift::BridgeEvent::InputGone);
        auto rel = bridge.makeReleaseSnapshot(blueshift::NormalizedInput::Kind::Keyboard);
        ble.sendKeyboard(rel.keyboard);
        CHECK(bridge.state() == blueshift::BridgeState::InputLost);
    }
    CHECK(bridge.releaseCount() >= 100);
}

static void testQueueOverflow() {
    blueshift::BoundedQueue<int, 2> q;
    CHECK(q.push(1, blueshift::QueueOverflowPolicy::DropNewest));
    CHECK(q.push(2, blueshift::QueueOverflowPolicy::DropNewest));
    CHECK(!q.push(3, blueshift::QueueOverflowPolicy::DropNewest));
    CHECK(q.overflowCount() == 1);
}

static void testBatteryCurve() {
    CHECK(blueshift::BatteryMonitor::percentFromVoltageMv(4200) == 100);
    CHECK(blueshift::BatteryMonitor::percentFromVoltageMv(5000) == 100);
    CHECK(blueshift::BatteryMonitor::percentFromVoltageMv(3200) == 0);
    CHECK(blueshift::BatteryMonitor::percentFromVoltageMv(3000) == 0);
    const auto mid = blueshift::BatteryMonitor::percentFromVoltageMv(3800);
    CHECK(mid > 0 && mid < 100);

    blueshift::BatteryMonitor mon;
    blueshift::BatterySample s;
    s.valid = false;
    auto st = mon.update(s);
    CHECK(!st.valid);

    s.valid = true;
    s.voltageMv = 3250;
    st = mon.update(s);
    CHECK(st.health == blueshift::BatteryHealth::Critical);
    s.voltageMv = 3450;
    st = mon.update(s);
    CHECK(st.health == blueshift::BatteryHealth::Low);
    s.voltageMv = 3800;
    st = mon.update(s);
    CHECK(st.health == blueshift::BatteryHealth::Ok);
    s.charging = true;
    st = mon.update(s);
    CHECK(st.health == blueshift::BatteryHealth::Charging);
}

static void testNavigation() {
    blueshift::NavigationInput nav;
    nav.setTiming(10, 100, 50);
    nav.setRaw(true, false, false, false, false, 0);
    nav.setRaw(true, false, false, false, false, 20);
    blueshift::NavEvent ev{};
    CHECK(nav.poll(ev));
    CHECK(ev.action == blueshift::NavAction::Up);
}

static void testUiRenderer() {
    blueshift::FramebufferDisplay fb;
    fb.begin();
    blueshift::UiRenderer renderer;
    blueshift::UiRenderModel model;
    model.screen = blueshift::UiScreen::Status;
    model.status.battery.valid = true;
    model.status.battery.percent = 82;
    model.inputLink = blueshift::LinkIndicator::Connected;
    model.outputLink = blueshift::LinkIndicator::Scanning;
    renderer.render(fb, model);
    CHECK(fb.litCount() > 0);

    model.screen = blueshift::UiScreen::ConfirmFactoryReset;
    renderer.render(fb, model);
    CHECK(fb.litCount() > 0);
}

static void testConfigAndFactoryReset() {
    blueshift::ConfigStore store;
    CHECK(store.begin());
    blueshift::AppConfigV1 cfg = blueshift::makeDefaultConfigV1();
    CHECK(store.save(cfg));
    blueshift::AppConfigV1 loaded;
    CHECK(store.load(loaded));
    cfg.base.locale = 9;
    CHECK(!blueshift::validateConfigV1(cfg));

    blueshift::DeviceStore devices;
    CHECK(devices.begin());
    blueshift::KnownDeviceMeta meta{};
    meta.used = true;
    meta.kind = blueshift::KnownDeviceKind::ClassicInput;
    std::snprintf(meta.friendlyName, sizeof(meta.friendlyName), "SN30");
    std::snprintf(meta.profileId, sizeof(meta.profileId), "%s",
                  blueshift::profiles::kSn30Pro.id);
    CHECK(devices.upsert(meta));
    CHECK(devices.findClassicInput() != nullptr);

    blueshift::ClassicHidHostSpike classic;
    blueshift::BleHidPeripheralSpike ble;
    classic.start();
    ble.start();
    blueshift::FactoryResetHooks hooks;
    hooks.config = &store;
    hooks.devices = &devices;
    hooks.classic = &classic;
    hooks.ble = &ble;
    auto r = blueshift::FactoryReset::run(hooks, blueshift::FactoryResetScope::FullBlueShift);
    CHECK(r.ok);
    CHECK(devices.findClassicInput() == nullptr);
}

static void testReconnectPolicy() {
    blueshift::ReconnectPolicyConfig cfg;
    cfg.maxAttempts = 3;
    cfg.initialDelayMs = 100;
    cfg.maxDelayMs = 400;
    blueshift::ReconnectPolicy p(cfg);
    CHECK(p.shouldAttempt(0));
    p.recordFailure(0);
    CHECK(!p.shouldAttempt(50));
    CHECK(p.shouldAttempt(100));
    p.recordFailure(100);
    p.recordFailure(300);
    CHECK(p.exhausted());
    p.reset();
    CHECK(!p.exhausted());
}

static void testBluetoothErrorNames() {
    CHECK(std::strcmp(blueshift::bluetoothErrorName(blueshift::BluetoothError::None), "None") ==
          0);
    CHECK(std::strcmp(blueshift::bluetoothErrorName(
                          blueshift::BluetoothError::ControllerInitFailed),
                      "ControllerInitFailed") == 0);
}

static void testSelfTestAndHwCompare() {
    blueshift::SelfTestReport r{};
    blueshift::selfTestSet(r, blueshift::SelfTestId::Oled, blueshift::SelfTestStatus::Ok, "ok");
    CHECK(r.entries[0].status == blueshift::SelfTestStatus::Ok);
    blueshift::ChipInfoReport chip{};
    chip.model = "ESP32";
    chip.flashSizeBytes = 4u * 1024u * 1024u;
    auto cmp = blueshift::compareDocumentedVsDetected(chip);
    CHECK(cmp.socLooksEsp32);
    CHECK(cmp.flashMatchesAssumed);
}

static void testI18n() {
    CHECK(i18nMessageCount() == static_cast<unsigned>(BlueshiftMsgId::MsgCount));
    for (unsigned i = 0; i < i18nMessageCount(); ++i) {
        CHECK(i18nHasFallback(static_cast<BlueshiftMsgId>(i)));
    }
    i18nSetLocale(BlueshiftLocale::DeDe);
    CHECK(std::strcmp(i18nMsg(BlueshiftMsgId::LabelBat), "AKKU") == 0);
    i18nSetLocale(BlueshiftLocale::EnUs);
    CHECK(std::strcmp(i18nMsg(BlueshiftMsgId::StatusInputLost), "INPUT LOST") == 0);
}

int main() {
    testBridgeFsm();
    testStuckKeySafety();
    testOutputDisconnectBackpressure();
    testE2EKeyboardGamepad();
    testMalformedHid();
    testPairingTimeout();
    testReconnectStress();
    testQueueOverflow();
    testBatteryCurve();
    testNavigation();
    testUiRenderer();
    testConfigAndFactoryReset();
    testReconnectPolicy();
    testBluetoothErrorNames();
    testSelfTestAndHwCompare();
    testI18n();
    if (g_failures != 0) {
        std::printf("%d failure(s)\n", g_failures);
        return 1;
    }
    std::printf("All host tests passed\n");
    return 0;
}
