# Modern macOS modernization

This branch modernizes EiskaltDC++ for current macOS releases while preserving the cross-platform Qt 5 application.

## Phase 1: launch reliability

Implemented:

- Native macOS single-instance handling now uses `QLocalServer`/`QLocalSocket` as the liveness check instead of trusting stale Qt shared-memory files.
- Stale local-server state is removed automatically with `QLocalServer::removeServer()`.
- A second launch forwards URLs/arguments to the running instance immediately.
- If the existing instance cannot be contacted, the new process continues startup instead of silently exiting.
- Recovery startup flags:
  - `--safe-mode` skips hash load, shared-file refresh, and autoconnect.
  - `--skip-hash-load` skips HashIndex/HashData load and implies skipped share refresh.
  - `--skip-share-refresh` skips the initial shared-files cache/refresh step.
  - `--nonblocking-share-refresh` avoids blocking startup on a cold share scan.
  - `--startup-timing` prints phase timings.

Example:

```sh
/Applications/EiskaltDC++.app/Contents/MacOS/EiskaltDC++ --safe-mode --startup-timing
```

## Phase 2: build, packaging, CI

Implemented:

- `CMakePresets.json` for reproducible Homebrew-based macOS builds.
- `macos/build-app.sh` for local arm64/x86_64 builds.
- `macos/sign-and-notarize.sh` for Developer ID signing/notarization.
- `macos/merge-universal-app.sh` for merging separately built thin apps into a universal app.
- GitHub Actions workflow at `.github/workflows/macos.yml`.

Local Apple Silicon build:

```sh
brew install cmake ninja ccache qt@5 gettext openssl@3 aspell jsoncpp libidn2 lua miniupnpc pcre2
export HOMEBREW_PREFIX="$(brew --prefix)"
macos/build-app.sh --arch arm64 --no-package
open dist/macos-arm64-release/EiskaltDC++.app
```

Universal build note: Homebrew Qt/dependencies are usually thin per prefix/runner. A true universal app requires building arm64 and x86_64 apps separately, merging every Mach-O binary/framework/plugin with `lipo`, and re-signing.

## Phase 3: macOS UX polish

The app now has a better foundation for macOS UX:

- URL/argument forwarding to the existing instance is immediate and no longer waits on shared-memory polling.
- Safe-mode and timing flags provide user-facing recovery behavior for hung launches.
- The packaged app is ad-hoc signed locally by `build-app.sh`; use `sign-and-notarize.sh` for public distribution.

Recommended follow-up UI work:

- Audit menus for standard macOS conventions (`Preferences…` on Cmd+, and app menu placement).
- Audit dark-mode colors/icons and Retina assets.
- Add native notification permission handling if Qt notifications are insufficient.

## Phase 4: resilience

Implemented:

- Safe mode for broken configs and giant hash/share states.
- Nonblocking share-refresh option.
- Startup timing diagnostics for hash database, shared files, queue, users, main-window, icons, notifications, and show phases.

Operational guidance:

- Use `--startup-timing` to identify the slow startup phase.
- Use `--safe-mode` to launch without touching the large hash/share state.
- Use `--skip-share-refresh` when a configured `/Volumes/...` share is offline or very slow.

Recommended follow-up engineering:

- Move hash-index parsing to an asynchronous loaded-state model.
- Add a GUI hash database repair/rebuild command.
- Add an exportable diagnostic bundle with redacted paths/passwords.

## Phase 5: security

Implemented on macOS:

- Favorite hub passwords are migrated from plaintext `Favorites.xml` into the macOS Keychain.
- New saves store hub passwords in Keychain and write `PasswordStore="Keychain"` instead of plaintext `Password`.
- Non-macOS behavior remains unchanged for compatibility.

Keychain item shape:

- Class: generic password
- Service: `org.eiskaltdcpp.hub-password`
- Account: `<hub-server>|<nick>`

If Keychain is unavailable or saving fails, the application falls back conservatively rather than losing the password.

## Release checklist

1. Build each target architecture.
2. Run the app with a clean profile and with an existing profile.
3. Verify single-instance forwarding and stale-lock recovery.
4. Verify favorite hub password migration with a test hub entry.
5. Sign with Developer ID and hardened runtime.
6. Notarize and staple the DMG.
7. Upload checksums and artifacts to GitHub Releases.
