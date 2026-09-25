#pragma once

#include <cstdint>

namespace blueshift {

enum class NavAction : uint8_t {
    None = 0,
    Up,
    Down,
    Left,
    Right,
    Confirm,
    Back,
    LongPress
};

struct NavEvent {
    NavAction action = NavAction::None;
    uint32_t timestampMs = 0;
};

// Debounced 5-way + long-press. Host-testable; GPIO sampling is injected.
class NavigationInput {
public:
    static constexpr uint32_t kDefaultDebounceMs = 30;
    static constexpr uint32_t kDefaultLongPressMs = 800;
    static constexpr uint32_t kDefaultRepeatMs = 250;

    void setTiming(uint32_t debounceMs, uint32_t longPressMs, uint32_t repeatMs) {
        debounceMs_ = debounceMs;
        longPressMs_ = longPressMs;
        repeatMs_ = repeatMs;
    }

    // Raw active-low style: true = pressed.
    void setRaw(bool up, bool down, bool left, bool right, bool confirm, uint32_t nowMs);

    bool poll(NavEvent &out);

private:
    struct Edge {
        bool raw = false;
        bool stable = false;
        uint32_t lastChangeMs = 0;
        uint32_t pressedAtMs = 0;
        bool longFired = false;
        uint32_t lastRepeatMs = 0;
    };

    void updateEdge(Edge &e, bool raw, uint32_t nowMs, NavAction shortAction, bool allowRepeat,
                    NavEvent &pending, bool &havePending);

    Edge up_{};
    Edge down_{};
    Edge left_{};
    Edge right_{};
    Edge confirm_{};

    uint32_t debounceMs_ = kDefaultDebounceMs;
    uint32_t longPressMs_ = kDefaultLongPressMs;
    uint32_t repeatMs_ = kDefaultRepeatMs;

    NavEvent queued_{};
    bool hasQueued_ = false;
};

} // namespace blueshift
