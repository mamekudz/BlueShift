# Bridge disconnect / reconnect policy (Milestone 2)

Language-neutral machine policy. UI strings come from i18x.

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

## Pairing

INPUT pairing (Classic) and OUTPUT pairing (BLE) are separate UI screens and separate FSM events. Do not conflate.
