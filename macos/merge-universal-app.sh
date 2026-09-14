#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 3 ]]; then
  echo "Usage: macos/merge-universal-app.sh arm64/EiskaltDC++.app x86_64/EiskaltDC++.app out/EiskaltDC++.app" >&2
  exit 2
fi

arm_app="$1"
x64_app="$2"
out_app="$3"
rm -rf "$out_app"
cp -R "$arm_app" "$out_app"

while IFS= read -r -d '' arm_file; do
  rel="${arm_file#$arm_app/}"
  x64_file="$x64_app/$rel"
  out_file="$out_app/$rel"
  if [[ -f "$x64_file" ]] && file "$arm_file" | grep -q 'Mach-O'; then
    lipo -create "$arm_file" "$x64_file" -output "$out_file"
  fi
done < <(find "$arm_app" -type f -print0)

codesign --force --deep --sign - "$out_app"
file "$out_app/Contents/MacOS/EiskaltDC++"
