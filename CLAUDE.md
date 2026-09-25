# ENGINEERING STANDARDS


## 45. Code Style

Use a consistent C/C++ style throughout the project.

General rules:

- C++17 or newer only when supported by the selected ESP32 toolchain.
- Prefer modern, explicit C++ over Arduino-style global state.
- 4 spaces indentation.
- No tabs for indentation.
- Opening braces on the same line.
- One statement per line.
- Keep functions small and focused.
- Avoid deeply nested control flow.
- Prefer early returns where they improve readability.
- Do not use clever/obscure constructs merely to reduce line count.

Naming:

    Types / classes:      PascalCase
    functions / methods:  camelCase
    local variables:      camelCase
    constants:            kPascalCase
    enum classes:         PascalCase
    enum values:          PascalCase

Example:

    enum class BridgeState {
        Idle,
        PairingClassic,
        ClassicConnected,
        WaitingForBleHost,
        Bridging,
        Error
    };

Do not mix naming conventions between modules.


## 46. Automatic Formatting

Use clang-format.

Add and maintain:

    .clang-format

Formatting must be deterministic.

Agents should format modified C/C++ files before completing a task.

Do not reformat unrelated files during a feature/debugging task.

A formatting-only change must not be mixed with a functional change unless
necessary.


## 47. Static Analysis

Where practical use:

- compiler warnings
- clang-tidy
- PlatformIO check

Enable useful warnings without turning third-party library warnings into
project failures.

Project-owned code should compile without avoidable warnings.

Do not suppress warnings globally merely to hide a local problem.


## 48. Source Layout

Prefer a modular source layout such as:

    src/
        main.cpp

    include/

    components/
        hardware/
        display/
        input/
        bluetooth/
        bridge/
        diagnostics/
        storage/
        ui/
        i18n/

Exact structure may evolve, but responsibilities must remain separated.

Do not accumulate the entire firmware in main.cpp.


## 49. Dependency Boundaries

Keep third-party code isolated.

Do not scatter direct Bluepad32 / BTstack / ESP-IDF calls throughout the
application.

Use project-owned interfaces such as:

    ClassicHidHost
    BleHidDevice
    Display
    BatteryMonitor
    InputDevice
    DiagnosticLogger

This allows libraries to be replaced without rewriting the entire project.


## 50. No Hidden Global State

Avoid mutable global variables where practical.

Prefer explicit ownership and dependency injection.

Hardware singletons may be acceptable when required by a framework, but
their use must remain localized.

Do not use global state as an easy shortcut between Bluetooth input and BLE
output.


# MEMORY / REAL-TIME


## 51. Memory Management

BlueShift runs on constrained embedded hardware.

Avoid:

- uncontrolled heap allocation
- repeated String allocation in hot paths
- dynamic allocation per HID report
- memory leaks during reconnect cycles

Prefer:

- fixed-size buffers
- bounded queues
- RAII where available
- explicit ownership

Bluetooth connect/disconnect cycles must not continuously consume memory.


## 52. Real-Time Behavior

Input forwarding is latency-sensitive.

Do not perform:

- filesystem operations
- expensive logging
- OLED redraws
- JSON generation

inside HID timing-critical callbacks.

Transport callbacks should capture events quickly and hand them to the
appropriate processing layer.


## 53. Queues and Backpressure

All event queues must be bounded.

Define behavior for overflow.

Never allow an unavailable BLE host to create an indefinitely growing queue
of Classic HID events.

For state-based devices such as gamepads, prefer transmitting the newest
relevant state rather than replaying obsolete movement events.


# HID BEHAVIOR


## 54. Raw vs Normalized HID

Always distinguish:

    Raw HID Report
          |
      Device Parser
          |
    Normalized State
          |
      Output Mapper
          |
    BLE HID Report

Keep raw diagnostic information available where practical.

Never mutate raw reports to make a specific device appear standard.


## 55. Device Profiles

Device-specific behavior belongs in profiles/adapters.

Examples:

    Sn30ProProfile
    Nimbus69070Profile
    GenericKeyboardProfile

Do not fill generic bridge code with:

    if (device == "SN30 Pro") ...

Prefer registration/profile mechanisms.


## 56. HID Descriptor Handling

Where possible, inspect HID descriptors rather than relying solely on device
names.

Device names are not reliable identifiers.

Identification may use combinations of:

- VID/PID where available
- HID descriptor
- Bluetooth address metadata where appropriate
- manufacturer information
- known report structure

Do not make permanent compatibility decisions based solely on advertised
names.


# I18N / I18X


## 57. Internationalization

BlueShift user-facing text must be internationalizable from the beginning.

Do not hard-code user-visible strings throughout firmware.

This includes:

- OLED messages
- diagnostics shown to the user
- pairing instructions
- errors
- menus
- battery messages
- device states


## 58. i18x

Use the existing project-family i18x concepts where practical.

BlueShift should remain compatible with the user's broader i18x approach.

Do not reduce internationalization to simple string translation only.

The architecture should allow locale-specific:

- text
- number formatting
- percentages
- durations
- dates/times where applicable
- units
- pluralization where needed

Do not invent a second incompatible localization architecture when existing
i18x infrastructure can be reused.


## 59. Firmware Localization Constraints

Embedded resources must remain memory-conscious.

Do not load every translation dynamically into RAM.

Prefer compact immutable resources in flash.

At minimum plan for:

    en-US
    de-DE

Additional languages may be generated/added later.

English should be the technical fallback locale.


## 60. OLED Localization

The OLED is small.

Translations must not assume English string length.

UI components must support:

- clipping where unavoidable
- scrolling text where useful
- abbreviated labels
- fallback strings

Do not design layouts around one fixed English phrase.


# LOGGING / DIAGNOSTICS


## 61. Structured Logging

Use consistent structured log categories.

Examples:

    [BOOT]
    [POWER]
    [OLED]
    [BT-CLASSIC]
    [BLE]
    [HID]
    [BRIDGE]
    [BATTERY]
    [DIAG]
    [ERROR]

Failures:

    [BLE][FAIL] ...
    [BT-CLASSIC][FAIL] ...

Avoid arbitrary printf output without subsystem context.


## 62. Log Levels

Support at least conceptually:

    ERROR
    WARN
    INFO
    DEBUG
    TRACE

Production builds should not continuously emit high-volume HID traces.

Debug builds may enable them explicitly.


## 63. Sensitive Data

Do not unnecessarily log:

- complete Bluetooth keys
- credentials
- private bonding material
- other secrets

Bluetooth addresses may be useful during diagnostics but should not be
published blindly in example logs.


# CONFIGURATION


## 64. Configuration

Keep configuration separate from implementation.

Possible configuration includes:

- device name
- OLED timeout
- pairing behavior
- preferred profile
- debug level
- language
- power-saving settings

Use a versioned configuration schema.


## 65. Persistent Configuration

Use supported ESP32 persistent storage mechanisms.

Configuration must survive firmware updates where practical.

Always validate stored configuration.

Corrupt or incompatible configuration must fall back safely to defaults.


## 66. Factory Reset

Plan a user-accessible factory reset.

It should be able to clear:

- Classic bonds
- BLE bonds
- device profiles
- user settings

It must NOT accidentally trigger during ordinary use.

Display clear confirmation on OLED.


# TESTING


## 67. Testing Strategy

Separate tests into:

    host/unit tests
    firmware tests
    physical hardware tests

Pure logic should be testable without physical hardware where practical.

Examples:

- HID normalization
- report mapping
- configuration parsing
- localization
- diagnostic JSON generation


## 68. Regression Tests

Every hardware milestone should verify existing functionality.

Eventually maintain a regression checklist including:

    OLED
    controls
    battery
    Classic BT
    BLE
    HID input
    HID output
    bridge
    reconnect
    sleep/wake


## 69. Reconnect Stress Test

Bluetooth reliability is a core product requirement.

Later milestones must test repeated:

    connect
    disconnect
    reconnect

Do not consider a device supported merely because it connected once.


## 70. Long-Running Test

Before declaring the bridge stable, perform extended operation tests.

Watch for:

- memory leaks
- connection loss
- queue buildup
- OLED freezes
- watchdog resets
- battery problems

A bridge expected to run for hours must be tested for hours.


# BUILD / RELEASE


## 71. Build Types

Provide at least:

    debug
    release

Debug:

- diagnostics enabled
- assertions where useful
- additional logging

Release:

- reduced logging
- optimized
- no unnecessary diagnostics overhead


## 72. Versioning

Use Semantic Versioning where practical:

    MAJOR.MINOR.PATCH

Pre-1.0 development may use:

    0.x.y

Firmware should expose its version through:

- serial boot log
- OLED About/diagnostic screen
- build metadata where useful


## 73. Reproducible Builds

Pin important dependency versions.

Do not silently track arbitrary latest versions of critical Bluetooth
libraries.

Document:

- PlatformIO platform
- ESP-IDF/framework
- Bluepad32 if used
- BTstack if used

A known-good release should remain buildable later.


## 74. Release Checklist

Before a release verify:

- clean build
- hardware regression
- Bluetooth input
- BLE output
- reconnect
- battery
- OLED
- documentation
- licenses
- compatibility database


# SECURITY / ROBUSTNESS


## 75. Pairing Security

Do not accept arbitrary pairing forever.

Pairing mode must be explicit and time-limited where practical.

Once paired, BlueShift should normally reconnect only to trusted devices.


## 76. Untrusted HID Input

Treat HID reports as untrusted external input.

Validate:

- report lengths
- indexes
- descriptor lengths
- values

Malformed Bluetooth packets must not cause buffer overflows or crashes.


## 77. Failure Recovery

BlueShift should recover gracefully from:

- Classic input disappearing
- BLE host disappearing
- malformed HID report
- failed pairing
- low battery
- temporary Bluetooth stack errors

A failure on one side must not require reflashing the device.


# LICENSES


## 78. Third-Party Licenses

Maintain:

    THIRD_PARTY_LICENSES.md

or equivalent.

For every incorporated dependency record:

- project
- version
- license
- source URL
- modifications if any

Do not copy code merely because it is visible on GitHub.


## 79. Bluepad32 / BTstack Licensing

Before adopting Bluepad32/BTstack, verify the exact licenses applicable to
this open-source project and document them.

Do not assume the licensing of one component automatically covers another.


# GIT / PROJECT HYGIENE


## 80. Git Rules

Keep commits focused.

Prefer:

    one logical change = one commit

Do not mix:

- formatting
- hardware debugging
- documentation rewrite
- unrelated refactoring

in one commit.


## 81. CLAUDE.md

CLAUDE.md is part of the project architecture.

It MUST be committed and backed up.

Agents may update factual project status after verified milestones.

Architectural rules should not be silently rewritten.


## 82. Secrets

Never commit:

- Wi-Fi credentials
- Bluetooth secrets
- tokens
- private NAS paths
- API keys

Provide example configuration files where needed.


# µGULP / BACKUP


## 83. Gulp

BlueShift should use the established µGulp-compatible Gulp workflow.

At minimum plan for:

    help
    docs
    backup:git
    backup:nas
    backup:all


## 84. Git Backup

Git backup must include:

- source
- CLAUDE.md
- documentation sources
- compatibility data
- PlatformIO configuration
- Gulp configuration

Do not automatically commit generated/build artifacts.


## 85. NAS Backup

Support up to THREE configurable NAS destinations.

No NAS destination may be hard-coded into the repository.

One unavailable NAS must not prevent backup to another configured target.


# DOCUMENTATION


## 86. Documentation Truthfulness

Documentation must distinguish clearly:

    VERIFIED
    EXPERIMENTAL
    PLANNED
    INCOMPATIBLE

Never advertise planned functionality as implemented.


## 87. Compatibility Database as Source

Do not manually maintain the same compatibility information in several
documents.

Prefer one structured source, for example:

    docs/compatibility/devices.json

Generate README tables from that source where practical.


## 88. Manufacturer Links

Compatibility records should prefer manufacturer/documentation links for
technical information.

Retail links such as Amazon/eBay may be included separately as convenience
links but must not be treated as authoritative technical evidence.


# FINAL ENGINEERING PRINCIPLE


## 89. Preserve Information

Do not throw away information prematurely.

Keep:

    raw HID
        ->
    normalized HID
        ->
    mapped output

as distinct layers.

This allows future compatibility fixes without changing transport code.


## 90. Optimize After Measurement

Do not optimize based on assumptions.

Measure:

- latency
- memory
- CPU
- battery consumption
- packet rate

before introducing complexity.


## 91. Simplicity First

The first successful bridge should be boring:

    one Classic device
    one BLE host
    one HID class
    stable reconnect

Reliability is more important than feature count.