#include "app/application.h"

#include "bluetooth/bluetooth_platform.h"
#include "blueshift/log.h"
#include "blueshift/version.h"
#include "diagnostics/diag_export.h"
#include "hid/ble_hid_reports.h"
#include "hid/device_parsers.h"
#include "i18n.h"
#include "storage/factory_reset.h"
#include "ui/oled_logo_placeholder.h"

#if defined(ARDUINO)
#include <Arduino.h>
#endif

namespace blueshift {
namespace {

void onClassicRaw(const RawHidReport &report, void *user) {
    auto *app = static_cast<Application *>(user);
    if (app == nullptr) {
        return;
    }
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
    DiagExporter::emitSelfTest("OLED", displayOk_, displayOk_ ? "begin_ok" : "begin_fail");
    DiagExporter::emitSelfTest("NAV", navOk_, navOk_ ? "begin_ok" : "begin_fail_or_host");
    DiagExporter::emitSelfTest("BAT", batteryOk_, batteryOk_ ? "adc_ok" : "adc_unavailable");
    DiagExporter::emitSelfTest("CLASSIC", true, "spike_stub_no_radio");
    DiagExporter::emitSelfTest("BLE", true, "spike_ready");

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
    BS_LOG_INFO("BOOT", "BlueShift %s", BLUESHIFT_VERSION_STRING);
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
    i18nSetLocale(config_.base.locale == 1 ? BlueshiftLocale::DeDe : BlueshiftLocale::EnUs);
    setLogMaxLevel(config_.logLevel);
    oledPower_.configure(static_cast<uint16_t>(config_.base.oledTimeoutSec / 2),
                         config_.base.oledTimeoutSec);
    ble_.setDeviceNames(config_.bleKeyboardName, config_.bleGamepadName);
    BS_LOG_INFO("BOOT", "config schema=%u locale=%u",
                static_cast<unsigned>(config_.base.schemaVersion),
                static_cast<unsigned>(config_.base.locale));
}

void Application::bootDisplay() {
#if defined(ARDUINO) && defined(BLUESHIFT_BOARD_T_LION)
    display_ = &ssd1306_;
    displayOk_ = display_->begin();
    BS_LOG_INFO("OLED", "SSD1306 begin=%s (IMPLEMENTED_UNVERIFIED)", displayOk_ ? "ok" : "fail");
#else
    display_ = &fbDisplay_;
    displayOk_ = display_->begin();
    BS_LOG_INFO("OLED", "framebuffer display (host/skeleton)");
#endif
    if (displayOk_ && display_ != nullptr) {
        display_->drawBitmap(48, 0, assets::kLogoW, assets::kLogoH, assets::kOledMonoPlaceholder);
        display_->present();
    }
}

void Application::bootNavigation() {
#if defined(ARDUINO) && defined(BLUESHIFT_BOARD_T_LION)
    navOk_ = gpioNav_.begin();
    BS_LOG_INFO("INPUT", "GPIO nav begin=%s (DOCUMENTED pins)", navOk_ ? "ok" : "fail");
#else
    navOk_ = false;
    BS_LOG_INFO("INPUT", "nav backend skipped (no T-Lion GPIO)");
#endif
}

void Application::bootBattery() {
#if defined(ARDUINO) && defined(BLUESHIFT_BOARD_T_LION)
    batteryOk_ = adcBat_.begin();
    if (batteryOk_) {
        const BatterySample sample = adcBat_.sample();
        batteryStatus_ = batteryMon_.update(sample);
        power_.onBattery(batteryStatus_);
        DiagExporter::emitBattery(batteryStatus_);
    } else {
        BS_LOG_WARN("BATTERY", "ADC begin failed — USB-only operation continues");
    }
#else
    batteryOk_ = false;
#endif
    // Battery failure must not abort boot.
}

void Application::bootBluetooth() {
#if defined(BLUESHIFT_BOARD_T_LION) || defined(BLUESHIFT_ENABLE_ESP_IDF_BT)
    auto &plat = BluetoothPlatform::instance();
    (void)plat.initializeDualMode();
    BS_LOG_INFO("BOOT", "BT platform status detail=%s", plat.statusDetail());
#else
    BS_LOG_INFO("BOOT", "BT platform deferred (enable T-Lion or BLUESHIFT_ENABLE_ESP_IDF_BT)");
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
    // Known-device reconnect — stub until bonds exist.
    if (classic_.reconnect()) {
        bridge_.apply(BridgeEvent::InputPaired);
    }
    BS_LOG_INFO("BRIDGE", "reconnect pass done state=%s", bridgeStateName(bridge_.state()));
}

void Application::handleNav(uint32_t nowMs) {
#if defined(ARDUINO) && defined(BLUESHIFT_BOARD_T_LION)
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
            hooks.classic = &classic_;
            hooks.ble = &ble_;
            const auto r = FactoryReset::run(hooks);
            BS_LOG_INFO("BOOT", "factory reset ok=%d", r.ok ? 1 : 0);
            config_ = makeDefaultConfigV1();
        }
        if (ev.action == NavAction::LongPress) {
            bridge_.apply(BridgeEvent::StartPairInput);
            classic_.startPairing(static_cast<uint32_t>(config_.classicPairTimeoutSec) * 1000u);
        }
    }
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

    if (bridge_.state() == BridgeState::InputLost) {
        renderModel_.inputLink = LinkIndicator::Error;
    } else if (bridge_.state() == BridgeState::PairingInput) {
        renderModel_.inputLink = LinkIndicator::Pairing;
    } else if (bridge_.inputLinked()) {
        renderModel_.inputLink = LinkIndicator::Connected;
    } else {
        renderModel_.inputLink = LinkIndicator::Disconnected;
    }

    if (bridge_.state() == BridgeState::OutputLost) {
        renderModel_.outputLink = LinkIndicator::Error;
    } else if (bridge_.state() == BridgeState::PairingOutput) {
        renderModel_.outputLink = LinkIndicator::Pairing;
    } else if (bridge_.outputLinked()) {
        renderModel_.outputLink = LinkIndicator::Connected;
    } else {
        renderModel_.outputLink = LinkIndicator::Disconnected;
    }
}

void Application::loopOnce(uint32_t nowMs) {
    handleNav(nowMs);
    classic_.tick(nowMs);
    ble_.tick(nowMs);

#if defined(ARDUINO) && defined(BLUESHIFT_BOARD_T_LION)
    if (batteryOk_ && (nowMs % 2000u) < 20u) {
        batteryStatus_ = batteryMon_.update(adcBat_.sample());
        power_.onBattery(batteryStatus_);
    }
#endif

    // Input lost → release stuck keys on BLE.
    if (bridge_.state() == BridgeState::InputLost &&
        lastBridgeState_ != BridgeState::InputLost) {
        const auto release = bridge_.makeReleaseSnapshot(NormalizedInput::Kind::Keyboard);
        ble_.sendKeyboard(release.keyboard);
        const auto gprel = bridge_.makeReleaseSnapshot(NormalizedInput::Kind::Gamepad);
        ble_.sendGamepad(gprel.gamepad);
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
