# ESP-IDF main component — spike OR production (selected by cmake_extra_args).
# Arduino builds ignore this file; idf_spike/ and idf_app/ are excluded from Arduino
# build_src_filter.

set(ROOT "${CMAKE_SOURCE_DIR}")

if(DEFINED BLUESHIFT_SPIKE_MODE)
    set(SPIKE_MODE "${BLUESHIFT_SPIKE_MODE}")
    if(SPIKE_MODE STREQUAL "classic")
        set(SPIKE_SRCS "idf_spike/classic_hid_host_spike.c")
        set(SPIKE_REQUIRES nvs_flash bt)
    elseif(SPIKE_MODE STREQUAL "ble")
        set(SPIKE_SRCS "idf_spike/ble_hid_device_spike.c")
        set(SPIKE_REQUIRES nvs_flash bt esp_hid)
    elseif(SPIKE_MODE STREQUAL "a2dp")
        set(SPIKE_SRCS "idf_spike/a2dp_source_spike.c")
        set(SPIKE_REQUIRES nvs_flash bt)
    elseif(SPIKE_MODE STREQUAL "triple")
        set(SPIKE_SRCS "idf_spike/triple_role_spike.c")
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
else()
    # Production native ESP-IDF application.
    idf_component_register(
        SRCS "idf_app/app_main.cpp"
        INCLUDE_DIRS
            "idf_app"
            "${ROOT}/include"
            "${ROOT}/components"
            "${ROOT}/components/i18n"
        REQUIRES
            nvs_flash
            bt
            esp_hid
            driver
            esp_driver_gpio
            esp_driver_i2c
            esp_adc
            esp_timer
            esp_app_format
            spi_flash
            esp_psram
            freertos
            log
    )

    set(BLUESHIFT_PRODUCT_SRCS
        "${ROOT}/components/app/application.cpp"
        "${ROOT}/components/battery/adc_battery.cpp"
        "${ROOT}/components/battery/battery_monitor.cpp"
        "${ROOT}/components/bluetooth/ble_hid_peripheral.cpp"
        "${ROOT}/components/bluetooth/bluetooth_platform.cpp"
        "${ROOT}/components/bluetooth/classic_hid_host.cpp"
        "${ROOT}/components/bluetooth/esp_idf_ble_hid_peripheral.cpp"
        "${ROOT}/components/bluetooth/esp_idf_classic_hid_host.cpp"
        "${ROOT}/components/bridge/bridge_core.cpp"
        "${ROOT}/components/display/ssd1306_display.cpp"
        "${ROOT}/components/display/idf_ssd1306_display.cpp"
        "${ROOT}/components/hid/ble_hid_reports.cpp"
        "${ROOT}/components/hid/device_parsers.cpp"
        "${ROOT}/components/i18n/i18n.cpp"
        "${ROOT}/components/input/gpio_navigation.cpp"
        "${ROOT}/components/input/navigation_input.cpp"
        "${ROOT}/components/power/power_policy.cpp"
        "${ROOT}/components/storage/config_store.cpp"
        "${ROOT}/components/storage/device_store.cpp"
        "${ROOT}/components/storage/factory_reset.cpp"
        "${ROOT}/components/ui/ui_controller.cpp"
        "${ROOT}/components/ui/ui_renderer.cpp"
        "${ROOT}/components/diagnostics/self_test.cpp"
        "${ROOT}/components/diagnostics/chip_info.cpp"
        "${ROOT}/components/diagnostics/hw_revision.cpp"
        "${ROOT}/components/bluetooth/bluetooth_error.cpp"
        "${ROOT}/components/protocol/extension_codec.cpp"
        "${ROOT}/components/protocol/extension_parser.cpp"
        "${ROOT}/components/audio/edge_to_pcm.cpp"
        "${ROOT}/components/audio/esp_idf_a2dp_audio_output.cpp"
    )

    target_sources(${COMPONENT_LIB} PRIVATE ${BLUESHIFT_PRODUCT_SRCS})
    target_compile_definitions(${COMPONENT_LIB} PUBLIC
        BLUESHIFT_BOARD_T_LION=1
        BLUESHIFT_NATIVE_ESP_IDF=1
        BLUESHIFT_ENABLE_ESP_IDF_BT=1
        # Audio path is optional at runtime; stack A2DP remains gated by sdkconfig.
        BLUESHIFT_AUDIO_PIPELINE=1
    )
endif()
