#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 ]]; then
  echo "Usage: macos/sign-and-notarize.sh /path/to/EiskaltDC++.app 'Developer ID Application: Name (TEAMID)' [output.dmg]" >&2
  exit 2
fi

app="$1"
identity="$2"
dmg="${3:-EiskaltDC++-macos.dmg}"

codesign --force --deep --timestamp --options runtime --sign "$identity" "$app"
codesign --verify --deep --strict --verbose=2 "$app"

rm -f "$dmg"
hdiutil create -volname "EiskaltDC++" -srcfolder "$app" -ov -format UDZO "$dmg"
codesign --force --timestamp --sign "$identity" "$dmg"

if [[ -n "${APPLE_ID:-}" && -n "${APPLE_TEAM_ID:-}" && -n "${APPLE_APP_SPECIFIC_PASSWORD:-}" ]]; then
  xcrun notarytool submit "$dmg" \
    --apple-id "$APPLE_ID" \
    --team-id "$APPLE_TEAM_ID" \
    --password "$APPLE_APP_SPECIFIC_PASSWORD" \
    --wait
  xcrun stapler staple "$dmg"
  spctl -a -vv -t open --context context:primary-signature "$dmg"
else
  echo "Skipping notarization; set APPLE_ID, APPLE_TEAM_ID, and APPLE_APP_SPECIFIC_PASSWORD."
fi
