# PlatformIO pre-script: select sdkconfig.defaults per IDF spike env.
# PlatformIO 6.9.0 / espidf.py only auto-loads project-root sdkconfig.defaults.

Import("env")
from pathlib import Path
import shutil

root = Path(env["PROJECT_DIR"])
pioenv = env["PIOENV"]

mapping = {
    "t-lion-idf-spike": "sdkconfig.defaults.dual",
    "t-lion-idf-spike-classic": "sdkconfig.defaults.classic",
    "t-lion-idf-spike-ble": "sdkconfig.defaults.ble",
}

name = mapping.get(pioenv)
if name:
    src = root / "spike" / "idf" / name
    dst = root / "sdkconfig.defaults"
    if src.is_file():
        shutil.copyfile(src, dst)
        print(f"[BlueShift] sdkconfig.defaults <- spike/idf/{name}")
