// Host-side unit tests for Milestone 2 pure logic (no ESP32 required).
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "battery/battery_monitor.h"
#include "bridge/bounded_queue.h"
#include "bridge/bridge_core.h"
#include "hid/ble_hid_reports.h"
#include "hid/normalized_hid.h"
#include "i18n.h"
#include "input/navigation_input.h"
#include "storage/app_config.h"
#include "ui/ui_controller.h"

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

static void testQueueOverflow() {
    blueshift::BoundedQueue<int, 2> q;
    CHECK(q.push(1, blueshift::QueueOverflowPolicy::DropNewest));
    CHECK(q.push(2, blueshift::QueueOverflowPolicy::DropNewest));
    CHECK(!q.push(3, blueshift::QueueOverflowPolicy::DropNewest));
    CHECK(q.overflowCount() == 1);
    int v = 0;
    CHECK(q.pop(v) && v == 1);
    CHECK(q.pop(v) && v == 2);

    blueshift::BoundedQueue<int, 2> q2;
    q2.push(1, blueshift::QueueOverflowPolicy::DropOldest);
    q2.push(2, blueshift::QueueOverflowPolicy::DropOldest);
    q2.push(3, blueshift::QueueOverflowPolicy::DropOldest);
    CHECK(q2.pop(v) && v == 2);
    CHECK(q2.pop(v) && v == 3);

    blueshift::BoundedQueue<int, 4> q3;
    q3.push(10, blueshift::QueueOverflowPolicy::KeepNewestOnly);
    q3.push(11, blueshift::QueueOverflowPolicy::KeepNewestOnly);
    q3.push(12, blueshift::QueueOverflowPolicy::KeepNewestOnly);
    // force overflow path
    q3.push(1, blueshift::QueueOverflowPolicy::DropNewest);
    q3.push(2, blueshift::QueueOverflowPolicy::DropNewest);
    q3.push(3, blueshift::QueueOverflowPolicy::DropNewest);
    q3.push(4, blueshift::QueueOverflowPolicy::DropNewest);
    CHECK(!q3.push(99, blueshift::QueueOverflowPolicy::KeepNewestOnly));
    CHECK(q3.size() == 1);
    CHECK(q3.pop(v) && v == 99);
}

static void testBatteryCurve() {
    CHECK(blueshift::BatteryMonitor::percentFromVoltageMv(4200) == 100);
    CHECK(blueshift::BatteryMonitor::percentFromVoltageMv(3200) == 0);
    const auto mid = blueshift::BatteryMonitor::percentFromVoltageMv(3800);
    CHECK(mid > 0 && mid < 100);

    blueshift::BatteryMonitor mon;
    blueshift::BatterySample s;
    s.valid = true;
    s.voltageMv = 3250;
    auto st = mon.update(s);
    CHECK(st.health == blueshift::BatteryHealth::Critical);
    s.voltageMv = 3600;
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

    nav.setRaw(false, false, false, false, true, 200);
    nav.setRaw(false, false, false, false, true, 220);
    CHECK(nav.poll(ev));
    CHECK(ev.action == blueshift::NavAction::Confirm);
    nav.setRaw(false, false, false, false, true, 400);
    CHECK(nav.poll(ev));
    CHECK(ev.action == blueshift::NavAction::LongPress);
}

static void testHidReports() {
    uint8_t keys[6] = {0x04, 0, 0, 0, 0, 0};
    uint8_t report[8] = {};
    CHECK(blueshift::buildKeyboardReport(0x02, keys, report));
    CHECK(report[0] == 0x02);
    CHECK(report[2] == 0x04);
    CHECK(blueshift::bleKeyboardDescriptor().size > 0);
    CHECK(blueshift::bleMouseDescriptor().size > 0);
    CHECK(blueshift::bleGamepadDescriptor().size > 0);
    CHECK(blueshift::scaleAxisToUint8(0) == 128);
}

static void testI18n() {
    CHECK(i18nMessageCount() == static_cast<unsigned>(BlueshiftMsgId::MsgCount));
    for (unsigned i = 0; i < i18nMessageCount(); ++i) {
        CHECK(i18nHasFallback(static_cast<BlueshiftMsgId>(i)));
    }
    i18nSetLocale(BlueshiftLocale::DeDe);
    CHECK(std::strcmp(i18nMsg(BlueshiftMsgId::LabelBat), "AKKU") == 0);
    i18nSetLocale(BlueshiftLocale::EnUs);
    CHECK(std::strcmp(i18nMsg(BlueshiftMsgId::LabelBat), "BAT") == 0);
    CHECK(std::strcmp(i18nMsg(static_cast<BlueshiftMsgId>(999)), "") == 0);
}

static void testUiFactoryReset() {
    blueshift::UiController ui;
    ui.onNav(blueshift::NavAction::LongPress);
    CHECK(ui.screen() == blueshift::UiScreen::PairInput);
    // Navigate to settings then open factory reset confirm — selection path simplified:
    ui.onNav(blueshift::NavAction::Left);
    CHECK(ui.screen() == blueshift::UiScreen::Status);
    // Direct confirm should not factory-reset from status.
    CHECK(!ui.consumeFactoryResetRequest());
}

static void testConfig() {
    auto cfg = blueshift::makeDefaultConfig();
    CHECK(blueshift::validateConfig(cfg));
    cfg.locale = 9;
    CHECK(!blueshift::validateConfig(cfg));
    auto fixed = blueshift::migrateOrDefault(cfg);
    CHECK(fixed.locale == 0);
}

int main() {
    testBridgeFsm();
    testQueueOverflow();
    testBatteryCurve();
    testNavigation();
    testHidReports();
    testI18n();
    testUiFactoryReset();
    testConfig();
    if (g_failures != 0) {
        std::printf("%d failure(s)\n", g_failures);
        return 1;
    }
    std::printf("All host tests passed\n");
    return 0;
}
