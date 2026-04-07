# EiskaltDC++ Windows Build Environment Setup
# Run this script in PowerShell as Administrator on the win11 VM
#
# Usage:
#   Set-ExecutionPolicy Bypass -Scope Process -Force
#   .\setup-win11-buildenv.ps1
#
# This installs everything needed for both Qt6/MSVC and GTK3/MSYS2 builds.

$ErrorActionPreference = "Stop"

function Write-Step($msg) {
    Write-Host "`n=== $msg ===" -ForegroundColor Cyan
}

# ── 1. Install winget if missing (Windows 11 should have it) ──
Write-Step "Checking winget"
if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
    Write-Host "winget not found, installing..."
    Add-AppxPackage -RegisterByFamilyName -MainPackage Microsoft.DesktopAppInstaller_8wekyb3d8bbwe
    Start-Sleep -Seconds 5
}
winget --version

# ── 2. Install Git ──
Write-Step "Installing Git"
winget install --id Git.Git -e --accept-source-agreements --accept-package-agreements --silent
$env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")

# ── 3. Install Visual Studio 2022 Community (full IDE with C++ workload) ──
Write-Step "Installing Visual Studio 2022 Community with C++ desktop workload"
winget install --id Microsoft.VisualStudio.2022.Community -e --accept-source-agreements --accept-package-agreements --silent `
    --override "--wait --passive --add Microsoft.VisualStudio.Workload.NativeDesktop --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.Windows11SDK.22621 --add Microsoft.Component.MSBuild --add Microsoft.VisualStudio.Component.VC.CMake.Project --add Microsoft.VisualStudio.Component.VC.DiagnosticTools --add Microsoft.VisualStudio.Component.Debugger.JustInTime --add Microsoft.VisualStudio.Component.IntelliCode"

# ── 4. Install CMake and Ninja ──
Write-Step "Installing CMake and Ninja"
winget install --id Kitware.CMake -e --accept-source-agreements --accept-package-agreements --silent
winget install --id Ninja-build.Ninja -e --accept-source-agreements --accept-package-agreements --silent

# ── 5. Install Python (needed for some build tools) ──
Write-Step "Installing Python 3.12"
winget install --id Python.Python.3.12 -e --accept-source-agreements --accept-package-agreements --silent

# Refresh PATH
$env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")

# ── 6. Install vcpkg ──
Write-Step "Installing vcpkg"
if (-not (Test-Path "C:\vcpkg")) {
    git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
    & C:\vcpkg\bootstrap-vcpkg.bat -disableMetrics
} else {
    Write-Host "vcpkg already exists at C:\vcpkg"
}

# Set vcpkg environment variables permanently
[System.Environment]::SetEnvironmentVariable("VCPKG_INSTALLATION_ROOT", "C:\vcpkg", "Machine")
[System.Environment]::SetEnvironmentVariable("VCPKG_DEFAULT_TRIPLET", "x64-windows", "Machine")
$env:VCPKG_INSTALLATION_ROOT = "C:\vcpkg"
$env:VCPKG_DEFAULT_TRIPLET = "x64-windows"
$env:Path += ";C:\vcpkg"

# ── 7. Install vcpkg dependencies for Qt6/MSVC build ──
Write-Step "Installing vcpkg dependencies (this may take 20-30 minutes)"
& C:\vcpkg\vcpkg.exe install --triplet x64-windows `
    openssl bzip2 zlib miniupnpc pcre2 lua `
    libiconv libidn2 "gettext[tools]"

# ── 8. Install Qt6 via aqtinstall (command-line Qt installer) ──
Write-Step "Installing Qt 6.8.3 via aqtinstall"
& python -m pip install aqtinstall
$qtDir = "C:\Qt"
if (-not (Test-Path "$qtDir\6.8.3")) {
    & python -m aqt install-qt windows desktop 6.8.3 win64_msvc2022_64 `
        --outputdir $qtDir `
        --modules qtmultimedia
} else {
    Write-Host "Qt 6.8.3 already installed"
}

# Set Qt environment
$qt6Dir = "$qtDir\6.8.3\msvc2022_64"
[System.Environment]::SetEnvironmentVariable("Qt6_DIR", $qt6Dir, "Machine")
[System.Environment]::SetEnvironmentVariable("CMAKE_PREFIX_PATH", $qt6Dir, "Machine")
$env:Qt6_DIR = $qt6Dir
$env:CMAKE_PREFIX_PATH = $qt6Dir

# ── 9. Install MSYS2 for GTK3 build ──
Write-Step "Installing MSYS2"
winget install --id MSYS2.MSYS2 -e --accept-source-agreements --accept-package-agreements --silent

# Wait for MSYS2 to finish setup
Start-Sleep -Seconds 10

# Install GTK3 build dependencies in MSYS2 UCRT64
$msys2 = "C:\msys64\usr\bin\bash.exe"
if (Test-Path $msys2) {
    Write-Step "Installing MSYS2/UCRT64 packages for GTK3 build"
    & $msys2 -lc "pacman --noconfirm -Syu"
    & $msys2 -lc "pacman --noconfirm -S mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-pkg-config"
    & $msys2 -lc "pacman --noconfirm -S mingw-w64-ucrt-x86_64-gtk3 mingw-w64-ucrt-x86_64-openssl mingw-w64-ucrt-x86_64-bzip2 mingw-w64-ucrt-x86_64-zlib mingw-w64-ucrt-x86_64-miniupnpc mingw-w64-ucrt-x86_64-pcre2 mingw-w64-ucrt-x86_64-lua mingw-w64-ucrt-x86_64-libidn2 mingw-w64-ucrt-x86_64-gettext-tools mingw-w64-ucrt-x86_64-gettext-runtime mingw-w64-ucrt-x86_64-libiconv mingw-w64-ucrt-x86_64-libnotify"
} else {
    Write-Host "MSYS2 not found at expected path. Install GTK3 deps manually." -ForegroundColor Yellow
}

# ── 10. Clone the repository ──
Write-Step "Cloning eiskaltdcpp repository"
$repoDir = "C:\src\eiskaltdcpp"
if (-not (Test-Path $repoDir)) {
    git clone https://github.com/eiskaltdcpp/eiskaltdcpp.git $repoDir
} else {
    Write-Host "Repository already exists at $repoDir"
}

# ── 11. Create build helper scripts ──
Write-Step "Creating build helper scripts"

# Qt6/MSVC build script
$qt6BuildScript = @'
@echo off
REM EiskaltDC++ Qt6/MSVC Build Script
REM Run from "x64 Native Tools Command Prompt for VS 2022"

set VCPKG_ROOT=C:\vcpkg
set Qt6_DIR=C:\Qt\6.8.3\msvc2022_64
set CMAKE_PREFIX_PATH=%Qt6_DIR%
set PATH=%Qt6_DIR%\bin;%VCPKG_ROOT%\installed\x64-windows\bin;%VCPKG_ROOT%\installed\x64-windows\tools\gettext\bin;%PATH%

cd /d C:\src\eiskaltdcpp

cmake -B build-qt6 -G Ninja ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
    -DGETTEXT_SEARCH_PATH=%VCPKG_ROOT%\installed\x64-windows ^
    -DUSE_QT6=ON ^
    -DUSE_GTK3=OFF ^
    -DUSE_JS=ON ^
    -DUSE_QT_QML=OFF ^
    -DUSE_MINIUPNP=ON ^
    -DWITH_DHT=ON ^
    -DUSE_IDN2=ON ^
    -DLUA_SCRIPT=ON ^
    -DPERL_REGEX=ON ^
    -DUSE_ASPELL=OFF ^
    -DDBUS_NOTIFY=OFF ^
    -DWITH_EMOTICONS=ON ^
    -DWITH_SOUNDS=ON ^
    -DBUILD_TESTS=ON ^
    -DOPENSSL_MSVC=ON

cmake --build build-qt6
echo.
echo Build complete! Binary at: C:\src\eiskaltdcpp\build-qt6\
'@

Set-Content -Path "C:\src\build-qt6.bat" -Value $qt6BuildScript

# GTK3/MSYS2 build script
$gtk3BuildScript = @'
#!/bin/bash
# EiskaltDC++ GTK3/MSYS2 Build Script
# Run from MSYS2 UCRT64 shell

cd /c/src/eiskaltdcpp

cmake -B build-gtk3 -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DUSE_QT6=OFF \
    -DUSE_GTK3=ON \
    -DUSE_MINIUPNP=ON \
    -DWITH_DHT=ON \
    -DUSE_IDN2=ON \
    -DLUA_SCRIPT=ON \
    -DPERL_REGEX=ON \
    -DUSE_ASPELL=OFF \
    -DUSE_LIBNOTIFY=ON \
    -DWITH_EMOTICONS=ON \
    -DWITH_SOUNDS=ON \
    -DBUILD_TESTS=ON

cmake --build build-gtk3
echo ""
echo "Build complete! Binary at: /c/src/eiskaltdcpp/build-gtk3/"
'@

# Write with Unix line endings for MSYS2
[System.IO.File]::WriteAllText("C:\src\build-gtk3.sh", $gtk3BuildScript.Replace("`r`n", "`n"))

# ── Done ──
Write-Step "Setup Complete!"
Write-Host @"

Build environment is ready. Summary:
  - Visual Studio 2022 Community (full IDE with C++ desktop workload)
  - CMake, Ninja, Git, Python 3.12
  - vcpkg at C:\vcpkg (with all deps installed)
  - Qt 6.8.3 at C:\Qt\6.8.3\msvc2022_64
  - MSYS2 at C:\msys64 (with GTK3 and all deps)
  - Source at C:\src\eiskaltdcpp

To build Qt6 (MSVC):
  1. Open "x64 Native Tools Command Prompt for VS 2022"
     (or open VS 2022 > Tools > Command Line > Developer Command Prompt)
  2. Run: C:\src\build-qt6.bat
  3. To debug: Open C:\src\eiskaltdcpp in VS 2022 (File > Open > CMake)

To build GTK3 (MSYS2):
  1. Open "MSYS2 UCRT64" from Start Menu
  2. Run: bash /c/src/build-gtk3.sh

"@ -ForegroundColor Green
