# ESP-IDF main component for BlueShift feasibility spikes only.
# Arduino builds do not use this file; idf_spike/ is excluded from Arduino
# build_src_filter in platformio.ini.

set(SPIKE_MODE "dual")
if(DEFINED BLUESHIFT_SPIKE_MODE)
    set(SPIKE_MODE "${BLUESHIFT_SPIKE_MODE}")
endif()

if(SPIKE_MODE STREQUAL "classic")
    set(SPIKE_SRCS "idf_spike/classic_hid_host_spike.c")
    set(SPIKE_REQUIRES nvs_flash bt)
elseif(SPIKE_MODE STREQUAL "ble")
    set(SPIKE_SRCS "idf_spike/ble_hid_device_spike.c")
    set(SPIKE_REQUIRES nvs_flash bt esp_hid)
else()
    set(SPIKE_SRCS "idf_spike/dual_mode_spike.c")
    set(SPIKE_REQUIRES nvs_flash bt esp_hid)
endif()

idf_component_register(
    SRCS ${SPIKE_SRCS}
    INCLUDE_DIRS "idf_spike"
    REQUIRES ${SPIKE_REQUIRES}
)
