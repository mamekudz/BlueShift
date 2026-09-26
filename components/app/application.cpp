#include "app/application.h"

#include "bluetooth/bluetooth_error.h"
#include "bluetooth/bluetooth_platform.h"
#include "blueshift/log.h"
#include "blueshift/version.h"
#include "diagnostics/chip_info.h"
#include "diagnostics/diag_export.h"
#include "diagnostics/hw_revision.h"
#include "diagnostics/self_test.h"
#include "hid/ble_hid_reports.h"
#include "hid/device_parsers.h"
#include "i18n.h"
#include "storage/factory_reset.h"
#include "ui/oled_logo_placeholder.h"

#if defined(ARDUINO)
#include <Arduino.h>
#endif
#if defined(BLUESHIFT_NATIVE_ESP_IDF) && !defined(ARDUINO)
#include "esp_idf_version.h"
#endif

namespace blueshift {
namespace {

void onClassicRaw(const RawHidReport &report, void *user) {
    auto *app = static_cast<Application *>(user);
    if (app == nullptr) {
        return;
    }
    // Callback path: parse + queue into bridge only — no OLED / NVS / heavy logs.
    GenericKeyboardParser kb;
    KeyboardState ks;
    if (kb.parse(report, ks)) {
        NormalizedInput in{};
        in.kind = NormalizedInput::Kind::Keyboard;
        in.keyboard = ks;
        NormalizedInput out{};
        if (app->bridge().onNormalizedInput(in, out)) {
            app->ble().sendKeyboard(out.keyboard);
        }
        return;
    }
    Sn30ProProfile sn30;
    GamepadState gs;
    if (sn30.parseGamepad(report, gs)) {
        NormalizedInput in{};
        in.kind = NormalizedInput::Kind::Gamepad;
        in.gamepad = gs;
        NormalizedInput out{};
        if (app->bridge().onNormalizedInput(in, out)) {
            app->ble().sendGamepad(out.gamepad);
        }
    }
}

LinkIndicator classicToLink(ClassicHostState s) {
    switch (s) {
    case ClassicHostState::Connected:
        return LinkIndicator::Connected;
    case ClassicHostState::Pairing:
        return LinkIndicator::Pairing;
    case ClassicHostState::Reconnecting:
        return LinkIndicator::Scanning;
    case ClassicHostState::Error:
        return LinkIndicator::Error;
    case ClassicHostState::Idle:
    default:
        return LinkIndicator::Disconnected;
    }
}

LinkIndicator bleToLink(BlePeripheralState s) {
    switch (s) {
    case BlePeripheralState::Connected:
        return LinkIndicator::Connected;
    case BlePeripheralState::Advertising:
        return LinkIndicator::Pairing;
    case BlePeripheralState::Error:
        return LinkIndicator::Error;
    case BlePeripheralState::Idle:
    default:
        return LinkIndicator::Disconnected;
    }
}

} // namespace

bool Application::begin() {
    bootSerial();
    bootConfig();
    bootDisplay();
    bootNavigation();
    bootBattery();
    bootBluetooth();
    bootUi();
    attemptReconnect();

    // Self-test: only mark OK after actual begin() success — never fake OK.
    SelfTestReport selfTest{};
    selfTestSet(selfTest, SelfTestId::Oled, displayOk_ ? SelfTestStatus::Ok : SelfTestStatus::Fail,
                displayOk_ ? "begin_ok" : "begin_fail");
    selfTestSet(selfTest, SelfTestId::Nav, navOk_ ? SelfTestStatus::Ok : SelfTestStatus::NotTested,
                navOk_ ? "begin_ok" : "skipped_or_fail");
    selfTestSet(selfTest, SelfTestId::Battery,
                batteryOk_ ? SelfTestStatus::Ok : SelfTestStatus::Warn,
                batteryOk_ ? "adc_ok" : "battery_unknown");
    selfTestSet(selfTest, SelfTestId::Classic,
                classicOk_ ? SelfTestStatus::Ok : SelfTestStatus::Fail,
                classic_.hardwareApiPresent() ? "api_present" : "stub");
    selfTestSet(selfTest, SelfTestId::Ble, bleOk_ ? SelfTestStatus::Ok : SelfTestStatus::Fail,
                ble_.hardwareApiPresent() ? "api_present" : "stub");
    for (unsigned i = 0; i < static_cast<unsigned>(SelfTestId::Count); ++i) {
        const auto &e = selfTest.entries[i];
        BS_LOG_INFO("DIAG", "selftest %s=%s %s", selfTestIdName(e.id), selfTestStatusName(e.status),
                    e.detail);
    }
    DiagExporter::emitSelfTest("OLED", displayOk_, displayOk_ ? "begin_ok" : "begin_fail");
    DiagExporter::emitSelfTest("NAV", navOk_, navOk_ ? "begin_ok" : "begin_fail_or_host");
    DiagExporter::emitSelfTest("BAT", batteryOk_, batteryOk_ ? "adc_ok" : "adc_unavailable");
    DiagExporter::emitSelfTest("CLASSIC", classicOk_, classicOk_ ? "started" : "fail");
    DiagExporter::emitSelfTest("BLE", bleOk_, bleOk_ ? "started" : "fail");

    const ChipInfoReport chip = captureChipInfo();
    BS_LOG_INFO("BOOT", "chip=%s rev=%u cores=%u flash=%u psram=%u heap=%u min_heap=%u",
                chip.model, static_cast<unsigned>(chip.revision),
                static_cast<unsigned>(chip.cores), static_cast<unsigned>(chip.flashSizeBytes),
                static_cast<unsigned>(chip.psramSizeBytes), static_cast<unsigned>(chip.freeHeap),
                static_cast<unsigned>(chip.minFreeHeap));
    emitHwRevisionReport(chip);

    bridge_.apply(BridgeEvent::BootDone);
    power_.onBootDone();
    refreshUiModel();
    if (display_ != nullptr && displayOk_) {
        renderer_.render(*display_, renderModel_);
    }
    return true;
}

void Application::bootSerial() {
#if defined(ARDUINO)
    Serial.begin(115200);
    delay(200);
#endif
    BS_LOG_INFO("BOOT", "BlueShift version=%s", BLUESHIFT_VERSION_STRING);
#if defined(BLUESHIFT_NATIVE_ESP_IDF)
    BS_LOG_INFO("BOOT", "framework=ESP-IDF %d.%d.%d", ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR,
                ESP_IDF_VERSION_PATCH);
#elif defined(ARDUINO)
    BS_LOG_INFO("BOOT", "framework=Arduino (legacy baseline)");
#endif
#if BLUESHIFT_BUILD_TYPE_DEBUG
    BS_LOG_INFO("BOOT", "build=debug");
#else
    BS_LOG_INFO("BOOT", "build=release");
#endif
}

void Application::bootConfig() {
    if (!configStore_.begin()) {
        BS_LOG_WARN("BOOT", "config store begin failed — defaults");
    }
    if (!configStore_.load(config_)) {
        config_ = makeDefaultConfigV1();
        BS_LOG_WARN("BOOT", "config invalid/missing — defaults");
    }
    if (!deviceStore_.begin()) {
        BS_LOG_WARN("BOOT", "device store begin failed");
    } else {
        DeviceStoreBlob devices{};
        (void)deviceStore_.load(devices);
    }
    i18nSetLocale(config_.base.locale == 1 ? BlueshiftLocale::DeDe : BlueshiftLocale::EnUs);
    setLogMaxLevel(config_.logLevel);
    oledPower_.configure(static_cast<uint16_t>(config_.base.oledTimeoutSec / 2),
                         config_.base.oledTimeoutSec);
    ble_.setDeviceNames(config_.bleKeyboardName, config_.bleGamepadName);

    ReconnectPolicyConfig rc;
    rc.pairingTimeoutMs = static_cast<uint32_t>(config_.classicPairTimeoutSec) * 1000u;
    reconnect_ = ReconnectPolicy(rc);

    BS_LOG_INFO("BOOT", "config schema=%u locale=%u output=%u",
                static_cast<unsigned>(config_.base.schemaVersion),
                static_cast<unsigned>(config_.base.locale),
                static_cast<unsigned>(config_.base.preferredOutput));
}

void Application::bootDisplay() {
#if defined(ARDUINO) && defined(BLUESHIFT_BOARD_T_LION)
    display_ = &ssd1306_;
    displayOk_ = display_->begin();
    BS_LOG_INFO("OLED", "SSD1306 begin=%s (IMPLEMENTED_UNVERIFIED)", displayOk_ ? "ok" : "fail");
#elif defined(ESP_PLATFORM) && !defined(ARDUINO) && defined(BLUESHIFT_NATIVE_ESP_IDF)
    display_ = &idfOled_;
    displayOk_ = display_->begin();
    BS_LOG_INFO("OLED", "IDF SSD1306 begin=%s (IMPLEMENTED_UNVERIFIED)", displayOk_ ? "ok" : "fail");
#else
    display_ = &fbDisplay_;
    displayOk_ = display_->begin();
    BS_LOG_INFO("OLED", "framebuffer display (host/skeleton)");
#endif
    if (displayOk_ && display_ != nullptr) {
        display_->clear();
        display_->drawText(0, 0, "BlueShift");
        display_->drawText(0, 12, "Booting...");
        display_->present();
    }
}

void Application::bootNavigation() {
#if (defined(ARDUINO) || defined(BLUESHIFT_NATIVE_ESP_IDF)) && defined(BLUESHIFT_BOARD_T_LION)
    navOk_ = gpioNav_.begin();
    BS_LOG_INFO("INPUT", "GPIO nav begin=%s (DOCUMENTED pins)", navOk_ ? "ok" : "fail");
#else
    navOk_ = false;
    BS_LOG_INFO("INPUT", "nav backend skipped (no T-Lion GPIO)");
#endif
}

void Application::bootBattery() {
#if (defined(ARDUINO) || defined(BLUESHIFT_NATIVE_ESP_IDF)) && defined(BLUESHIFT_BOARD_T_LION)
    batteryOk_ = adcBat_.begin();
    if (batteryOk_) {
        const BatterySample sample = adcBat_.sample();
        batteryStatus_ = batteryMon_.update(sample);
        power_.onBattery(batteryStatus_);
        DiagExporter::emitBattery(batteryStatus_);
    } else {
        batteryStatus_.valid = false;
        batteryStatus_.health = BatteryHealth::Unknown;
        batteryStatus_.percent = 0xFF;
        BS_LOG_WARN("BATTERY", "ADC begin failed — USB-only / BATTERY_UNKNOWN continues");
    }
#else
    batteryOk_ = false;
    batteryStatus_.health = BatteryHealth::Unknown;
    batteryStatus_.percent = 0xFF;
#endif
}

void Application::bootBluetooth() {
#if defined(BLUESHIFT_BOARD_T_LION) || defined(BLUESHIFT_ENABLE_ESP_IDF_BT) || \
    defined(BLUESHIFT_NATIVE_ESP_IDF)
    auto &plat = BluetoothPlatform::instance();
    (void)plat.initializeDualMode();
    BS_LOG_INFO("BOOT", "BT platform detail=%s err=%s", plat.statusDetail(),
                bluetoothErrorName(plat.lastError()));
#else
    BS_LOG_INFO("BOOT", "BT platform deferred (enable T-Lion or native ESP-IDF)");
#endif
    classic_.setRawCallback(onClassicRaw, this);
    classicOk_ = classic_.start();
    bleOk_ = ble_.start();
    BS_LOG_INFO("BT-CLASSIC", "EspIdfClassicHidHost start=%s api=%s", classicOk_ ? "ok" : "fail",
                classic_.hardwareApiPresent() ? "yes" : "no");
    BS_LOG_INFO("BLE", "EspIdfBleHidPeripheral start=%s api=%s", bleOk_ ? "ok" : "fail",
                ble_.hardwareApiPresent() ? "yes" : "no");
}

void Application::bootUi() {
    renderModel_.selfTestPending = false;
    BS_LOG_INFO("BOOT", "%s", i18nMsg(BlueshiftMsgId::StatusUnverified));
}

void Application::attemptReconnect() {
    // Boot policy: stack init → known Classic reconnect → BLE expose → UI.
    // No endless rapid loops — ReconnectPolicy bounds retries.
    const KnownDeviceMeta *known = deviceStore_.findClassicInput();
    if (known == nullptr) {
        BS_LOG_INFO("BRIDGE", "no known Classic input — skip reconnect");
        return;
    }
    BS_LOG_INFO("BRIDGE", "known input '%s' profile=%s", known->friendlyName, known->profileId);
    if (classic_.reconnect()) {
        reconnect_.recordSuccess();
        bridge_.apply(BridgeEvent::InputPaired);
    } else {
        reconnect_.recordFailure(0);
    }
    BS_LOG_INFO("BRIDGE", "reconnect pass done state=%s", bridgeStateName(bridge_.state()));
}

void Application::tickReconnect(uint32_t nowMs) {
    if (bridge_.inputLinked() || reconnect_.exhausted()) {
        return;
    }
    if (deviceStore_.findClassicInput() == nullptr) {
        return;
    }
    if (!reconnect_.shouldAttempt(nowMs)) {
        return;
    }
    if (classic_.reconnect()) {
        reconnect_.recordSuccess();
        bridge_.apply(BridgeEvent::InputPaired);
        oledPower_.notifyActivity(nowMs);
    } else {
        reconnect_.recordFailure(nowMs);
        BS_LOG_INFO("BRIDGE", "reconnect attempt=%u exhausted=%d",
                    static_cast<unsigned>(reconnect_.attempt()), reconnect_.exhausted() ? 1 : 0);
    }
}

void Application::handleNav(uint32_t nowMs) {
#if (defined(ARDUINO) || defined(BLUESHIFT_NATIVE_ESP_IDF)) && defined(BLUESHIFT_BOARD_T_LION)
    if (navOk_) {
        gpioNav_.poll(nowMs, nav_);
    }
#endif
    NavEvent ev{};
    while (nav_.poll(ev)) {
        oledPower_.notifyActivity(nowMs);
        ui_.onNav(ev.action);
        if (ui_.consumeFactoryResetRequest()) {
            FactoryResetHooks hooks;
            hooks.config = &configStore_;
            hooks.devices = &deviceStore_;
            hooks.classic = &classic_;
            hooks.ble = &ble_;
            const auto r = FactoryReset::run(hooks, FactoryResetScope::FullBlueShift);
            BS_LOG_INFO("BOOT", "factory reset ok=%d", r.ok ? 1 : 0);
            config_ = makeDefaultConfigV1();
            reconnect_.reset();
        }
        if (ev.action == NavAction::LongPress) {
            bridge_.apply(BridgeEvent::StartPairInput);
            classic_.startPairing(static_cast<uint32_t>(config_.classicPairTimeoutSec) * 1000u);
        }
    }
}

void Application::sampleBattery(uint32_t nowMs) {
#if (defined(ARDUINO) || defined(BLUESHIFT_NATIVE_ESP_IDF)) && defined(BLUESHIFT_BOARD_T_LION)
    if (!batteryOk_) {
        return;
    }
    if (nowMs - lastBatteryMs_ < 2000u) {
        return;
    }
    lastBatteryMs_ = nowMs;
    batteryStatus_ = batteryMon_.update(adcBat_.sample());
    power_.onBattery(batteryStatus_);
    if (power_.state() == PowerState::CriticalBattery) {
        // Safe warning only — no automatic NVS wipe / hard cutoff until physical thresholds.
        BS_LOG_WARN("POWER", "critical battery — warn only (cutoff DISABLED until measured)");
    }
#else
    (void)nowMs;
#endif
}

void Application::refreshUiModel() {
    renderModel_.screen = ui_.screen();
    renderModel_.selection = ui_.selection();
    renderModel_.status.bridge = bridge_.state();
    renderModel_.status.battery = batteryStatus_;
    renderModel_.status.inputOk = bridge_.inputLinked();
    renderModel_.status.outputOk = bridge_.outputLinked();
    renderModel_.status.inputName = bridge_.inputLinked() ? "Classic" : "";
    renderModel_.status.outputName = bridge_.outputLinked() ? "BLE" : "";

    // Prefer transport FSM for PAIR screens; bridge state for STATUS.
    const LinkIndicator classicLink = classicToLink(classic_.getState());
    const LinkIndicator bleLink = bleToLink(ble_.getState());

    if (renderModel_.screen == UiScreen::PairInput) {
        renderModel_.inputLink = classicLink;
    } else if (bridge_.state() == BridgeState::InputLost) {
        renderModel_.inputLink = LinkIndicator::Error;
    } else if (bridge_.state() == BridgeState::PairingInput) {
        renderModel_.inputLink = LinkIndicator::Pairing;
    } else if (bridge_.inputLinked()) {
        renderModel_.inputLink = LinkIndicator::Connected;
    } else {
        renderModel_.inputLink = classicLink;
    }

    if (renderModel_.screen == UiScreen::PairHost) {
        renderModel_.outputLink = bleLink;
    } else if (bridge_.state() == BridgeState::OutputLost) {
        renderModel_.outputLink = LinkIndicator::Error;
    } else if (bridge_.state() == BridgeState::PairingOutput) {
        renderModel_.outputLink = LinkIndicator::Pairing;
    } else if (bridge_.outputLinked()) {
        renderModel_.outputLink = LinkIndicator::Connected;
    } else {
        renderModel_.outputLink = bleLink;
    }

    // Sync bridge when transport leaves pairing on timeout → stable idle/connected.
    if (bridge_.state() == BridgeState::PairingInput &&
        classic_.getState() == ClassicHostState::Idle) {
        bridge_.apply(BridgeEvent::BackToIdle);
    }
    if (bridge_.state() == BridgeState::PairingOutput &&
        ble_.getState() == BlePeripheralState::Idle) {
        bridge_.apply(BridgeEvent::BackToIdle);
    }
}

void Application::loopOnce(uint32_t nowMs) {
    handleNav(nowMs);
    classic_.tick(nowMs);
    ble_.tick(nowMs);
    sampleBattery(nowMs);
    tickReconnect(nowMs);

    // Input lost → release stuck keys on BLE.
    if (bridge_.state() == BridgeState::InputLost &&
        lastBridgeState_ != BridgeState::InputLost) {
        const auto release = bridge_.makeReleaseSnapshot(NormalizedInput::Kind::Keyboard);
        ble_.sendKeyboard(release.keyboard);
        const auto gprel = bridge_.makeReleaseSnapshot(NormalizedInput::Kind::Gamepad);
        ble_.sendGamepad(gprel.gamepad);
        oledPower_.notifyActivity(nowMs);
    }
    if (lastBridgeState_ != bridge_.state()) {
        oledPower_.notifyActivity(nowMs);
    }
    lastBridgeState_ = bridge_.state();

    oledPower_.update(nowMs);
    if (display_ != nullptr) {
        oledPower_.apply(*display_);
    }

    if (nowMs - lastUiMs_ >= 100u) {
        lastUiMs_ = nowMs;
        refreshUiModel();
        if (display_ != nullptr && displayOk_ && oledPower_.mode() != OledPowerMode::Off) {
            renderer_.render(*display_, renderModel_);
        }
    }
}

} // namespace blueshift
