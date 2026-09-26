#include "ui/ui_controller.h"

namespace blueshift {

namespace {

constexpr int kMenuCount = 8;

UiScreen menuScreenAt(int index) {
    switch (index) {
    case 0:
        return UiScreen::Status;
    case 1:
        return UiScreen::PairInput;
    case 2:
        return UiScreen::PairHost;
    case 3:
        return UiScreen::Devices;
    case 4:
        return UiScreen::Profiles;
    case 5:
        return UiScreen::Diagnostics;
    case 6:
        return UiScreen::Settings;
    case 7:
        return UiScreen::Audio;
    default:
        return UiScreen::Status;
    }
}

} // namespace

void UiController::moveSelection(int delta) {
    selection_ += delta;
    while (selection_ < 0) {
        selection_ += kMenuCount;
    }
    selection_ %= kMenuCount;
}

void UiController::openSelected() {
    if (screen_ == UiScreen::Settings && selection_ == 0) {
        previous_ = screen_;
        screen_ = UiScreen::ConfirmFactoryReset;
        factoryResetArmed_ = true;
        return;
    }
    if (screen_ == UiScreen::ConfirmFactoryReset) {
        factoryResetRequest_ = true;
        screen_ = UiScreen::Settings;
        return;
    }
    previous_ = screen_;
    screen_ = menuScreenAt(selection_);
}

void UiController::goBack() {
    if (screen_ == UiScreen::ConfirmFactoryReset) {
        factoryResetArmed_ = false;
        screen_ = UiScreen::Settings;
        return;
    }
    if (screen_ == UiScreen::Status) {
        return;
    }
    screen_ = UiScreen::Status;
}

void UiController::onNav(NavAction action) {
    switch (action) {
    case NavAction::Up:
        moveSelection(-1);
        break;
    case NavAction::Down:
        moveSelection(1);
        break;
    case NavAction::Left:
    case NavAction::Back:
        goBack();
        break;
    case NavAction::Right:
    case NavAction::Confirm:
        openSelected();
        break;
    case NavAction::LongPress:
        // Pairing shortcut — never factory reset.
        previous_ = screen_;
        screen_ = UiScreen::PairInput;
        break;
    case NavAction::None:
        break;
    }
}

} // namespace blueshift
