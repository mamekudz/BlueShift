# Used only by native ESP-IDF PlatformIO environments (framework = espidf).
# Arduino environments ignore this file.
#
# BLUESHIFT_SPIKE_MODE — minimal Bluetooth link proofs (exclude product modules)
# BLUESHIFT_BUILD=production — future production firmware (t-lion-idf-*)

cmake_minimum_required(VERSION 3.16.0)

# Product modules live under components/ without per-folder IDF CMakeLists.
# They are compiled into the main component for production builds only.
# Keep directories without CMakeLists so ESP-IDF does not auto-add empty components.

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

if(DEFINED BLUESHIFT_SPIKE_MODE)
    project(blueshift_idf_spike)
else()
    project(blueshift)
endif()
