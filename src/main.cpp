#include <Arduino.h>

#include "app/application.h"

static blueshift::Application g_app;

void setup() {
    g_app.begin();
}

void loop() {
#if defined(ARDUINO)
    g_app.loopOnce(millis());
    delay(10);
#else
    g_app.loopOnce(0);
#endif
}
