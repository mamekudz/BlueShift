# PlatformIO pre-script: assemble sdkconfig.defaults for BlueShift IDF envs.
# PlatformIO espidf.py only auto-loads project-root sdkconfig.defaults.

Import("env")
from pathlib import Path

root = Path(env["PROJECT_DIR"])
pioenv = env["PIOENV"]
dst = root / "sdkconfig.defaults"
parts_dir = root / "sdkconfig.d"

spike_map = {
    "t-lion-idf-spike": root / "spike" / "idf" / "sdkconfig.defaults.dual",
    "t-lion-idf-spike-classic": root / "spike" / "idf" / "sdkconfig.defaults.classic",
    "t-lion-idf-spike-ble": root / "spike" / "idf" / "sdkconfig.defaults.ble",
}

prod_map = {
    "t-lion-idf-debug": ["defaults.common", "defaults.t-lion", "defaults.debug"],
    "t-lion-idf-release": ["defaults.common", "defaults.t-lion", "defaults.release"],
}

chunks = []
if pioenv in spike_map:
    src = spike_map[pioenv]
    if src.is_file():
        chunks.append(src.read_text(encoding="utf-8"))
        print(f"[BlueShift] sdkconfig.defaults <- {src.relative_to(root)}")
elif pioenv in prod_map:
    for name in prod_map[pioenv]:
        src = parts_dir / name
        if not src.is_file():
            raise FileNotFoundError(src)
        chunks.append(f"# ---- {name} ----\n")
        chunks.append(src.read_text(encoding="utf-8"))
        if not chunks[-1].endswith("\n"):
            chunks.append("\n")
    print(f"[BlueShift] sdkconfig.defaults <- sdkconfig.d for {pioenv}")
else:
    print(f"[BlueShift] sdkconfig.defaults unchanged for {pioenv}")

if chunks:
    dst.write_text("\n".join(chunks).rstrip() + "\n", encoding="utf-8")
