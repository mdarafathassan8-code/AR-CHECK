#!/usr/bin/env bash
set -euo pipefail

# Apply the Nova Android extension integration to a Chromium checkout.
# Usage: ./apply_nova_extension_patch.sh /path/to/chromium/src

CHROMIUM_SRC="${1:-}"
if [[ -z "${CHROMIUM_SRC}" || ! -d "${CHROMIUM_SRC}/chrome/browser/extensions" ]]; then
  echo "Usage: $0 /path/to/chromium/src" >&2
  exit 2
fi

ROOT="$(cd "$(dirname "$0")" && pwd)"
TARGET="${CHROMIUM_SRC}/chrome/browser/extensions"

cp "${ROOT}/nova_extension_manager_android.h" "${TARGET}/nova_extension_manager_android.h"
cp "${ROOT}/nova_extension_manager_android.cc" "${TARGET}/nova_extension_manager_android.cc"

python3 - "${CHROMIUM_SRC}/chrome/browser/extensions/BUILD.gn" <<'PY'
from pathlib import Path
import sys

p = Path(sys.argv[1])
s = p.read_text()
marker = '  "crx_installer.h",\n'
add = ('  "nova_extension_manager_android.cc",\n'
       '  "nova_extension_manager_android.h",\n')
if '"nova_extension_manager_android.cc"' not in s:
    if marker not in s:
        raise SystemExit('Could not find crx_installer.h in chrome/browser/extensions/BUILD.gn')
    s = s.replace(marker, marker + add, 1)

# The manager uses Chromium's safe zip::Unzip implementation.
zip_dep = '    "//third_party/zlib/google:zip",\n'
if '"//third_party/zlib/google:zip"' not in s:
    # Insert into the first deps list in this target.
    deps_marker = '  deps = [\n'
    if deps_marker not in s:
        raise SystemExit('Could not find deps list in chrome/browser/extensions/BUILD.gn')
    s = s.replace(deps_marker, deps_marker + zip_dep, 1)

p.write_text(s)
PY

echo "Nova extension manager sources installed."
echo "Next: enable the Chromium extension runtime for your Android target, regenerate GN, then build chrome_public_apk."
