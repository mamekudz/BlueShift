#pragma once

#include "battery/adc_battery.h"
#include "battery/battery_monitor.h"
#include "bluetooth/ble_hid_peripheral.h"
#include "bluetooth/classic_hid_host.h"
#include "bridge/bridge_core.h"
#include "display/display.h"
#include "display/framebuffer_display.h"
#include "display/oled_power.h"
#include "hardware/t_lion/board.h"
#include "input/gpio_navigation.h"
#include "input/navigation_input.h"
#include "power/power_policy.h"
#include "storage/config_store.h"
#include "ui/ui_controller.h"
#include "ui/ui_renderer.h"

#if defined(ARDUINO) && defined(BLUESHIFT_BOARD_T_LION)
#include "display/ssd1306_display.h"
#endif

namespace blueshift {

class Application {
public:
    bool begin();
    void loopOnce(uint32_t nowMs);

    BridgeCore &bridge() {
        return bridge_;
    }
    UiController &ui() {
        return ui_;
    }
    ClassicHidHostSpike &classic() {
        return classic_;
    }
    BleHidPeripheralSpike &ble() {
        return ble_;
    }
    ConfigStore &configStore() {
        return configStore_;
    }

private:
    void bootSerial();
    void bootConfig();
    void bootDisplay();
    void bootNavigation();
    void bootBattery();
    void bootBluetooth();
    void bootUi();
    void attemptReconnect();
    void handleNav(uint32_t nowMs);
    void refreshUiModel();

    t_lion::BoardConfig board_{};
    ConfigStore configStore_{};
    AppConfigV1 config_{};
    BridgeCore bridge_{};
    PowerPolicy power_{};
    UiController ui_{};
    UiRenderer renderer_{};
    UiRenderModel renderModel_{};
    NavigationInput nav_{};
    OledPowerManager oledPower_{};
    ClassicHidHostSpike classic_{};
    BleHidPeripheralSpike ble_{};
    BatteryMonitor batteryMon_{};
    BatteryStatus batteryStatus_{};
    FramebufferDisplay fbDisplay_{};
#if defined(ARDUINO) && defined(BLUESHIFT_BOARD_T_LION)
    Ssd1306Display ssd1306_{board_};
    GpioNavigationBackend gpioNav_{board_};
    AdcBatteryBackend adcBat_{board_};
#endif
    Display *display_ = nullptr;
    bool displayOk_ = false;
    bool navOk_ = false;
    bool batteryOk_ = false;
    bool classicOk_ = false;
    bool bleOk_ = false;
    uint32_t lastUiMs_ = 0;
    BridgeState lastBridgeState_ = BridgeState::Boot;
};

} // namespace blueshift
