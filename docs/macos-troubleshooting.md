# macOS troubleshooting

## App silently exits

Run with timing and safe mode:

```sh
EiskaltDC++.app/Contents/MacOS/EiskaltDC++ --safe-mode --startup-timing
```

The modernized single-instance code should recover from stale Qt IPC files automatically. If a second launch cannot contact the first instance, it continues startup and prints a warning.

## Slow startup

Use:

```sh
EiskaltDC++.app/Contents/MacOS/EiskaltDC++ --startup-timing
```

Common slow phases:

- `Hash database`: large `HashData.dat`/`HashIndex.xml`.
- `Shared Files`: large shares, missing external volumes, or cold share cache.

Recovery flags:

- `--safe-mode`
- `--skip-hash-load`
- `--skip-share-refresh`
- `--nonblocking-share-refresh`

## Gatekeeper says the app is rejected

Local builds are ad-hoc signed. Public distribution requires Developer ID signing and notarization:

```sh
macos/sign-and-notarize.sh dist/macos-arm64-release/EiskaltDC++.app 'Developer ID Application: Name (TEAMID)'
```

Set `APPLE_ID`, `APPLE_TEAM_ID`, and `APPLE_APP_SPECIFIC_PASSWORD` to notarize.
