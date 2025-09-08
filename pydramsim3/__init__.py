import os
import sys
import ctypes
from pathlib import Path

_pkg_dir = Path(__file__).resolve().parent
_libdir = _pkg_dir / "lib"

if sys.platform == "win32":
    os.add_dll_directory(str(_libdir))   # Windows
else:
    dep = next(_libdir.glob("libdramsim3.*"), None)
    if dep:
        ctypes.CDLL(str(dep), mode=getattr(ctypes, "RTLD_GLOBAL", os.RTLD_GLOBAL))

from .pydramsim3 import *

PYDRAMSIM_ROOT = os.path.abspath(os.path.dirname(__file__))
PYDRAMSIM_CONFIG_DIR = os.path.join(PYDRAMSIM_ROOT, "configs")
PYDRAMSIM_DEFAULT_OUT_DIR = os.path.join(PYDRAMSIM_ROOT, ".tmp")

os.makedirs(PYDRAMSIM_DEFAULT_OUT_DIR, exist_ok=True)

def PYDRAMSIM_MSYS_CONFIG_PATH(config_name: str) -> str:
    if not config_name.endswith(".ini"):
        config_name += ".ini"
    return os.path.join(PYDRAMSIM_CONFIG_DIR, config_name)
