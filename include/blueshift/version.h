#pragma once

// Single source of truth for the BlueShift project version (pre-release).
// Future firmware will also expose build type and platform/framework metadata
// at boot / on the OLED About screen — keep this header the version anchor.

#define BLUESHIFT_VERSION_MAJOR 0
#define BLUESHIFT_VERSION_MINOR 1
#define BLUESHIFT_VERSION_PATCH 0
#define BLUESHIFT_VERSION_SUFFIX "dev"

#define BLUESHIFT_VERSION_STRING "0.1.0-dev"

#ifndef BLUESHIFT_BUILD_TYPE_DEBUG
#define BLUESHIFT_BUILD_TYPE_DEBUG 0
#endif
