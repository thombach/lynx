# Injects the git version (e.g. "v1.2.0-3-g1a2b3c4-dirty") as LYNX_VERSION
import subprocess

Import("env")

try:
    version = subprocess.check_output(
        ["git", "describe", "--tags", "--always", "--dirty"], text=True, stderr=subprocess.DEVNULL
    ).strip()
except Exception:
    version = "unknown"

env.Append(CPPDEFINES=[("LYNX_VERSION", env.StringifyMacro(version))])
