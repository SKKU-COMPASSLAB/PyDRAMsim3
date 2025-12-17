import os
import sys
import configparser
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

def create_new_dramsim_config_file(
    src_config_path: str, 
    new_config_path: str,
    
    # channel_size: int,
    # n_channel: int,
    system_params: dict[str, int] = None,
    dram_structure_params: dict[str, int] = None,
):
    if not os.path.isfile(src_config_path):
        src_config_path = PYDRAMSIM_MSYS_CONFIG_PATH(src_config_path)
    if not os.path.isfile(src_config_path):
        raise FileNotFoundError(f"DRAMSim3 config file '{src_config_path}' not found.")

    os.makedirs(os.path.dirname(new_config_path), exist_ok=True)

    src_config = configparser.ConfigParser()
    src_config.read(src_config_path)
    
    # src_config["system"]["channel_size"] = str(channel_size)
    # src_config["system"]["channels"] = str(n_channel)
    
    if system_params is not None:
        for key, value in system_params.items():
            src_config["system"][key] = str(value)
    
    if dram_structure_params is not None:
        for key, value in dram_structure_params.items():
            src_config["dram_structure"][key] = str(value)

    with open(new_config_path, "w") as new_file:
        src_config.write(new_file)
        
        
def get_bandwidth_from_dramsim_config(config_path: str) -> float:
    if not os.path.isfile(config_path):
        config_path = PYDRAMSIM_MSYS_CONFIG_PATH(config_path)
    if not os.path.isfile(config_path):
        raise FileNotFoundError(f"DRAMSim3 config file '{config_path}' not found.")

    src_config = configparser.ConfigParser()
    src_config.read(config_path)
    
    is_ddr = False
    protocol = src_config["system"].get("protocol", "DDR3").upper()
    
    if "DDR" in protocol or "HBM" in protocol:
        is_ddr = True
    
    tck = float(src_config["timing"]["tck"])  # in ns
    data_rate = (2 if is_ddr else 1) * (1 / tck * 1e9)  # in Transfers/sec
    channels = int(src_config["system"]["channels"])
    bus_width = int(src_config["system"]["bus_width"])  # in bits
    
    return data_rate * channels * (bus_width / 8)  # in Byte/sec