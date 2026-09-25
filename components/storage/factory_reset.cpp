#include "storage/factory_reset.h"

namespace blueshift {

FactoryResetResult FactoryReset::run(const FactoryResetHooks &hooks) {
    FactoryResetResult r;
    if (hooks.config != nullptr) {
        r.settingsCleared = hooks.config->clear();
        AppConfigV1 fresh = makeDefaultConfigV1();
        hooks.config->save(fresh);
    }
    if (hooks.classic != nullptr) {
        r.classicForgetOk = hooks.classic->forgetDevice();
    } else {
        r.classicForgetOk = true;
    }
    if (hooks.ble != nullptr) {
        r.bleForgetOk = hooks.ble->forgetHost();
    } else {
        r.bleForgetOk = true;
    }
    r.ok = r.settingsCleared && r.classicForgetOk && r.bleForgetOk;
    return r;
}

} // namespace blueshift
