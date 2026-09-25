#include "input/navigation_input.h"

namespace blueshift {

void NavigationInput::updateEdge(Edge &e, bool raw, uint32_t nowMs, NavAction shortAction,
                                 bool allowRepeat, NavEvent &pending, bool &havePending) {
    if (raw != e.raw) {
        e.raw = raw;
        e.lastChangeMs = nowMs;
    }
    if ((nowMs - e.lastChangeMs) < debounceMs_) {
        return;
    }
    if (raw == e.stable) {
        if (allowRepeat && e.stable && !e.longFired && shortAction != NavAction::None) {
            if (e.lastRepeatMs == 0) {
                e.lastRepeatMs = e.pressedAtMs;
            }
            if ((nowMs - e.lastRepeatMs) >= repeatMs_) {
                e.lastRepeatMs = nowMs;
                if (!havePending) {
                    pending.action = shortAction;
                    pending.timestampMs = nowMs;
                    havePending = true;
                }
            }
        }
        if (e.stable && !e.longFired && shortAction == NavAction::Confirm) {
            if ((nowMs - e.pressedAtMs) >= longPressMs_) {
                e.longFired = true;
                if (!havePending) {
                    pending.action = NavAction::LongPress;
                    pending.timestampMs = nowMs;
                    havePending = true;
                }
            }
        }
        return;
    }

    e.stable = raw;
    if (raw) {
        e.pressedAtMs = nowMs;
        e.longFired = false;
        e.lastRepeatMs = 0;
        if (!havePending && shortAction != NavAction::None) {
            pending.action = shortAction;
            pending.timestampMs = nowMs;
            havePending = true;
        }
    } else {
        e.longFired = false;
        e.lastRepeatMs = 0;
    }
}

void NavigationInput::setRaw(bool up, bool down, bool left, bool right, bool confirm,
                             uint32_t nowMs) {
    NavEvent pending{};
    bool have = false;
    updateEdge(up_, up, nowMs, NavAction::Up, true, pending, have);
    updateEdge(down_, down, nowMs, NavAction::Down, true, pending, have);
    updateEdge(left_, left, nowMs, NavAction::Back, false, pending, have);
    updateEdge(right_, right, nowMs, NavAction::Confirm, false, pending, have);
    updateEdge(confirm_, confirm, nowMs, NavAction::Confirm, false, pending, have);
    if (have) {
        queued_ = pending;
        hasQueued_ = true;
    }
}

bool NavigationInput::poll(NavEvent &out) {
    if (!hasQueued_) {
        return false;
    }
    out = queued_;
    hasQueued_ = false;
    return true;
}

} // namespace blueshift
