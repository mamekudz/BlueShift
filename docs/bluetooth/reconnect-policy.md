# Bridge disconnect / reconnect policy

Language-neutral machine policy. UI strings come from i18x.  
**Status:** DOCUMENTED / IMPLEMENTED_UNVERIFIED — not PHYSICALLY VERIFIED.

## Boot sequence

1. Initialize NVS / BluetoothPlatform (BTDM + Bluedroid)
2. Start Classic HID Host + BLE HID Device adapters
3. Attempt **known Classic input** reconnect (from `DeviceStore`, bounded)
4. Expose BLE output (advertise / reconnect host as configured)
5. Update UI / self-test
6. Fall back to pairing UI only when no known device or reconnect exhausted

## Bounded retry / backoff

`ReconnectPolicy` defaults:

| Parameter | Default |
| --- | --- |
| maxAttempts | 5 |
| initialDelayMs | 1000 |
| maxDelayMs | 30000 |
| pairingTimeoutMs | from config (60 s) |

No endless rapid reconnect loops.

## Input (Classic) disappears

| Action | Decision |
| --- | --- |
| BLE output stays available | Yes — keep advertising/connected host link |
| Release keys/buttons | **Yes** — emit cleared Keyboard/Mouse/Gamepad snapshot |
| Bridge state | `INPUT_LOST` |
| Replay old gamepad motion after reconnect | **No** |

## Output (BLE host) disappears

| Action | Decision |
| --- | --- |
| Classic input may remain connected | Yes |
| Discard obsolete queued reports | **Yes** — bounded queue + DropOldest / KeepNewest for axes |
| Bridge state | `OUTPUT_LOST` |
| On host return | Resume from **newest** gamepad/mouse state; keyboard starts empty then live transitions |

## Queues

| Stream | Capacity | Overflow |
| --- | --- | --- |
| Gamepad snapshots | small (e.g. 4) | KeepNewestOnly / DropOldest |
| Keyboard events | bounded | DropOldest with release-on-disconnect cleanup |
| Mouse motion | small | KeepNewestOnly |

Never allow disconnected output to grow queues without bound.

Bluetooth callbacks must not render OLED, write config, or allocate large buffers — translate to bounded project queues/state.

## Pairing

INPUT pairing (Classic) and OUTPUT pairing (BLE) are separate UI screens and separate FSM events. Timeouts return UI to a stable Idle/connected state.

## Task model (native ESP-IDF)

Minimal ownership:

| Owner | Role |
| --- | --- |
| Bluedroid / esp_hid | Stack callbacks (capture only) |
| `bs_app` FreeRTOS task | BridgeCore, UI, battery, reconnect ticks (~10 ms) |

Do **not** create one FreeRTOS task per component.
