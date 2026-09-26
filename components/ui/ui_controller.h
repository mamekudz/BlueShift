#pragma once

#include "battery/battery_monitor.h"
#include "bridge/bridge_core.h"
#include "input/navigation_input.h"

namespace blueshift {

enum class UiScreen : uint8_t {
    Status = 0,
    PairInput,
    PairHost,
    Devices,
    Profiles,
    Diagnostics,
    Settings,
    About,
    ConfirmFactoryReset,
    Audio // optional experimental — ESP][ speaker → A2DP
};

struct UiStatusModel {
    const char *inputName = "";
    const char *outputName = "";
    bool inputOk = false;
    bool outputOk = false;
    BatteryStatus battery{};
    BridgeState bridge = BridgeState::Boot;
};

class UiController {
public:
    UiScreen screen() const {
        return screen_;
    }

    void setStatus(const UiStatusModel &model) {
        status_ = model;
    }

    const UiStatusModel &status() const {
        return status_;
    }

    // UP/DOWN move selection; CONFIRM/RIGHT open; LEFT back;
    // LONG_PRESS pairing shortcut — never immediate factory reset.
    void onNav(NavAction action);

    int selection() const {
        return selection_;
    }

    bool factoryResetArmed() const {
        return factoryResetArmed_;
    }

    bool consumeFactoryResetRequest() {
        if (!factoryResetRequest_) {
            return false;
        }
        factoryResetRequest_ = false;
        factoryResetArmed_ = false;
        return true;
    }

private:
    void moveSelection(int delta);
    void openSelected();
    void goBack();

    UiScreen screen_ = UiScreen::Status;
    UiScreen previous_ = UiScreen::Status;
    int selection_ = 0;
    UiStatusModel status_{};
    bool factoryResetArmed_ = false;
    bool factoryResetRequest_ = false;
};

} // namespace blueshift
