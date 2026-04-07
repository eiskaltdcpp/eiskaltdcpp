<#
.SYNOPSIS
    Local build script for EiskaltDC++ on Windows.

.DESCRIPTION
    Builds eiskaltdcpp-qt (Qt6/MSVC), eiskaltdcpp-gtk (GTK3/MSYS2),
    eiskaltdcpp-daemon (JSONRPC) and eiskaltdcpp-cli (JSONRPC).

    Installs or verifies all required dependencies before building.
    When run without -SkipDeps, the script will auto-install most tools
    via winget/vcpkg/pacman. Qt6 is the one dependency that usually
    requires manual installation beforehand.

    PREREQUISITES
    =============

    The following must be installed or available BEFORE running this script.
    Items marked [auto] are installed automatically when -SkipDeps is NOT used.

    For ALL targets:
      - Windows 10/11 64-bit
      - PowerShell 5.1+ (built-in on Windows 10+)
          Run:  Set-ExecutionPolicy Bypass -Scope Process -Force
          before invoking this script if execution policy blocks it.
      - CMake >= 3.10             [auto via winget: Kitware.CMake]
      - Ninja build system        [auto via winget: Ninja-build.Ninja]
      - Git                       [auto via winget: Git.Git]

    For Qt6 / daemon / CLI targets (MSVC build):
      - Visual Studio 2022 (Community or Build Tools) with "Desktop
        development with C++" workload. The script auto-detects
        vcvarsall.bat and imports the x64 MSVC environment, so you do
        NOT need to run from a VS Developer prompt (though that works too).
      - vcpkg  (https://github.com/microsoft/vcpkg)
          [auto-cloned to C:\vcpkg if not found]
          The following vcpkg packages are installed automatically:
            openssl, bzip2, zlib, miniupnpc, pcre2, lua,
            libiconv, libidn2, gettext[tools]
      - Qt 6 (>= 6.2) for MSVC 2022 64-bit, with the Qt Multimedia module.
          Install via one of:
            1. Qt Online Installer  https://www.qt.io/download-qt-installer
               Select: Qt 6.x > MSVC 2022 64-bit, plus "Qt Multimedia"
            2. aqtinstall (pip):
               pip install aqtinstall
               aqt install-qt windows desktop 6.8.3 win64_msvc2022_64 -m qtmultimedia
          Then either:
            - Set env var Qt6_DIR to <Qt root>/6.x.x/msvc2022_64/lib/cmake/Qt6
            - Or install Qt to C:\Qt (auto-detected)
          The script searches C:\Qt and ~/Qt for Qt6Config.cmake.

    For GTK3 target (MSYS2/UCRT64 build):
      - MSYS2  (https://www.msys2.org/)
          [auto via winget: MSYS2.MSYS2, installs to C:\msys64]
          The following UCRT64 packages are installed automatically:
            gcc, cmake, ninja, pkgconf, gtk3, openssl, bzip2, zlib,
            miniupnpc, pcre2, lua, libidn2, gettext-tools,
            gettext-runtime, libiconv, libnotify

    For NSIS installer packaging (optional):
      - NSIS (Nullsoft Scriptable Install System)
          [NOT auto-installed; install manually if you want an installer]
          Install via:  winget install NSIS.NSIS
          If not found, the build still succeeds — files are staged in
          windows\installer\ and you can run makensis manually later.

    WHAT THE SCRIPT DOES
    ====================

    1. Checks/installs dependencies (skipped with -SkipDeps)
    2. Sets up MSVC environment and PATH for vcpkg/Qt tools
    3. Builds Qt6 GUI + daemon + CLI via CMake/Ninja/MSVC
       - Deploys Qt6 + vcpkg DLLs to build and install directories
    4. Builds GTK3 GUI via MSYS2 UCRT64 (cmake/ninja/gcc)
       - Bundles UCRT64 DLLs, GTK runtime data, pixbuf loaders
    5. Stages Qt build output and creates NSIS installer (if makensis found)
    6. Prints summary with paths to all built executables

    OUTPUT DIRECTORIES
    ==================

      dist-qt\            Qt6 + daemon + CLI install (with all DLLs)
      dist-gtk\           GTK3 install (with all DLLs)
      build-qt\           CMake/Ninja build tree (MSVC)
      build-gtk\          CMake/Ninja build tree (MSYS2)
      windows\installer\  NSIS staging directory (auto-created, gitignored)
      windows\EiskaltDC++-<ver>-x86_64-installer.exe  (if NSIS available)

.PARAMETER Targets
    Comma-separated list of targets to build: qt, gtk, daemon, cli, all.
    Default: all

.PARAMETER BuildType
    CMake build type: Debug, Release, RelWithDebInfo.
    Default: Release

.PARAMETER InstallPrefix
    Installation prefix (absolute or relative to repo root).
    Default: .\dist-qt for MSVC targets, .\dist-gtk for GTK target.

.PARAMETER SkipDeps
    Skip dependency installation (assume everything is already present).
    The script still sets up MSVC environment and PATH even with -SkipDeps.

.PARAMETER Clean
    Remove build directories before configuring.

.PARAMETER Jobs
    Number of parallel build jobs. Default: number of logical processors.

.EXAMPLE
    .\build-local.ps1
    # Full build of all targets with dependency installation.

.EXAMPLE
    .\build-local.ps1 -Targets qt,daemon -BuildType Debug
    # Debug build of Qt GUI and daemon only.

.EXAMPLE
    .\build-local.ps1 -Targets gtk -Clean
    # Clean GTK3 build.

.EXAMPLE
    .\build-local.ps1 -Targets all -SkipDeps
    # Build everything, skip dependency checks (faster for repeat builds).
#>

[CmdletBinding()]
param(
    [ValidateSet('qt','gtk','daemon','cli','all')]
    [string[]]$Targets = @('all'),

    [ValidateSet('Debug','Release','RelWithDebInfo')]
    [string]$BuildType = 'Release',

    [string]$InstallPrefix,

    [switch]$SkipDeps,

    [switch]$Clean,

    [int]$Jobs = 0
)

Set-StrictMode -Version Latest
# Use 'Continue' because PS 5.1 treats native-command stderr as errors
# when 'Stop' is set, which kills cmake/vcpkg/ninja calls that write warnings.
# The script checks $LASTEXITCODE explicitly after each native command.
$ErrorActionPreference = 'Continue'

# ─── Helpers ──────────────────────────────────────────────────────────

function Write-Step  { param([string]$msg) Write-Host "`n===> $msg" -ForegroundColor Cyan }
function Write-Ok    { param([string]$msg) Write-Host "  OK: $msg" -ForegroundColor Green }
function Write-Warn  { param([string]$msg) Write-Host "  WARN: $msg" -ForegroundColor Yellow }
function Write-Err   { param([string]$msg) Write-Host "  ERROR: $msg" -ForegroundColor Red }
function Bail        { param([string]$msg) Write-Err $msg; exit 1 }

function Test-Command { param([string]$cmd) $null -ne (Get-Command $cmd -ErrorAction SilentlyContinue) }

# Run a native command without PS5.1 treating stderr as a terminating error.
# Usage: Invoke-Native cmd arg1 arg2 ...
function Invoke-Native {
    $prevEAP = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        & $args[0] @($args | Select-Object -Skip 1)
    } finally {
        $ErrorActionPreference = $prevEAP
    }
}

# Deploy Qt6 + vcpkg DLLs into a target directory that contains .exe files.
# This makes the executables runnable in-place (build dir or install dir).
function Deploy-WindowsDlls {
    param(
        [string]$TargetDir,    # Directory containing .exe files
        [string]$Label,        # Label for log messages (e.g. "build-dir", "install-dir")
        [string]$VcpkgRoot,    # vcpkg root path
        [string]$BuildType,    # Debug or Release
        [switch]$DeployQt      # Whether to run windeployqt
    )

    if ($DeployQt -and (Test-Command windeployqt)) {
        Write-Step "Deploying Qt6 DLLs to $Label"
        $deployFlag = if ($BuildType -eq 'Debug') { '--debug' } else { '--release' }
        $exes = Get-ChildItem $TargetDir -Filter '*.exe' -ErrorAction SilentlyContinue |
                Where-Object { $_.Name -notlike 'cmake*' -and $_.Name -notlike 'CMake*' }
        if ($exes) {
            $firstExe = $exes | Select-Object -First 1
            & windeployqt $deployFlag --no-translations --no-compiler-runtime $firstExe.FullName
            # Create qt.conf so Qt finds plugins in the same directory
            @"
[Paths]
Plugins = .
"@ | Set-Content (Join-Path $firstExe.DirectoryName 'qt.conf') -Encoding UTF8
            Write-Ok "Qt DLLs deployed to $Label"
        } else {
            Write-Warn "No .exe found in $TargetDir – skipping windeployqt"
        }
    }

    if ($VcpkgRoot) {
        Write-Step "Copying vcpkg DLL dependencies to $Label"
        $vcpkgBin = "$VcpkgRoot\installed\x64-windows\bin"
        if ($BuildType -eq 'Debug') {
            $vcpkgDebugBin = "$VcpkgRoot\installed\x64-windows\debug\bin"
            $searchPaths = @($vcpkgDebugBin, $vcpkgBin)
        } else {
            $searchPaths = @($vcpkgBin)
        }

        if (Test-Command dumpbin) {
            $visited = [System.Collections.Generic.HashSet[string]]::new(
                         [System.StringComparer]::OrdinalIgnoreCase)
            $toCopy  = [ordered]@{}
            $queue   = [System.Collections.Queue]::new()

            Get-ChildItem $TargetDir -Recurse -Include *.exe,*.dll -ErrorAction SilentlyContinue |
                ForEach-Object { $queue.Enqueue($_.FullName) }

            while ($queue.Count -gt 0) {
                $file = $queue.Dequeue()
                $fileName = [IO.Path]::GetFileName($file)
                if (-not $visited.Add($fileName)) { continue }

                $deps = & dumpbin /nologo /dependents $file 2>&1 |
                        Select-String '^\s+(\S+\.dll)\s*$' |
                        ForEach-Object { $_.Matches.Groups[1].Value }

                foreach ($dep in $deps) {
                    if ($visited.Contains($dep)) { continue }
                    if ($dep -like 'api-ms-*' -or $dep -like 'ext-ms-*') { continue }

                    $inTarget = Get-ChildItem "$TargetDir\$dep" -ErrorAction SilentlyContinue
                    if ($inTarget) {
                        $queue.Enqueue($inTarget.FullName)
                        continue
                    }

                    $found = $false
                    foreach ($sp in $searchPaths) {
                        $candidate = Join-Path $sp $dep
                        if (Test-Path $candidate) {
                            $toCopy[$dep] = $candidate
                            $queue.Enqueue($candidate)
                            $found = $true
                            break
                        }
                    }
                    if ($found) { continue }

                    if ($dep -match '^(msvcp\d|vcruntime\d|concrt\d|ucrtbase|vcomp\d)') {
                        $sys32 = Join-Path $env:SystemRoot "System32\$dep"
                        if (Test-Path $sys32) { $toCopy[$dep] = $sys32 }
                    }
                }
            }

            foreach ($entry in $toCopy.GetEnumerator()) {
                Copy-Item $entry.Value $TargetDir
                Write-Host "    $($entry.Key)  <-  $($entry.Value)" -ForegroundColor DarkGray
            }
            Write-Ok "Copied $($toCopy.Count) DLL(s) to $Label"
        } else {
            Write-Warn "dumpbin not available – copying all vcpkg runtime DLLs"
            foreach ($sp in $searchPaths) {
                if (Test-Path $sp) {
                    Copy-Item (Join-Path $sp '*.dll') $TargetDir -ErrorAction SilentlyContinue
                }
            }
        }
    }
}

# ─── Resolve paths ───────────────────────────────────────────────────

$RepoRoot = $PSScriptRoot   # Script lives at repo root
Push-Location $RepoRoot

if ($Jobs -le 0) { $Jobs = [Environment]::ProcessorCount }

# Expand target list
if ($Targets -contains 'all') {
    $Targets = @('qt','gtk','daemon','cli')
}

# Figure out which build "flavours" we need
$NeedMSVC  = ($Targets -contains 'qt') -or ($Targets -contains 'daemon') -or ($Targets -contains 'cli')
$NeedMSYS2 = ($Targets -contains 'gtk')

# Install prefixes
if ($InstallPrefix) {
    $resolved = Resolve-Path -LiteralPath $InstallPrefix -ErrorAction SilentlyContinue
    $QtInstallPrefix  = if ($resolved) { $resolved.Path } else { Join-Path $RepoRoot $InstallPrefix }
    $GtkInstallPrefix = $QtInstallPrefix
} else {
    $QtInstallPrefix  = Join-Path $RepoRoot 'dist-qt'
    $GtkInstallPrefix = Join-Path $RepoRoot 'dist-gtk'
}
# (When -InstallPrefix is given, both prefixes are already shared above)

$BuildDirQt  = Join-Path $RepoRoot 'build-qt'
$BuildDirGtk = Join-Path $RepoRoot 'build-gtk'

# ─── Banner ──────────────────────────────────────────────────────────

Write-Host ""
Write-Host "=============================================" -ForegroundColor Magenta
Write-Host "  EiskaltDC++ Local Build Script" -ForegroundColor Magenta
Write-Host "=============================================" -ForegroundColor Magenta
Write-Host "  Targets    : $($Targets -join ', ')"
Write-Host "  Build type : $BuildType"
Write-Host "  Jobs       : $Jobs"
Write-Host "  Repo root  : $RepoRoot"
if ($NeedMSVC)  { Write-Host "  Qt prefix  : $QtInstallPrefix" }
if ($NeedMSYS2) { Write-Host "  GTK prefix : $GtkInstallPrefix" }
Write-Host ""

# Declare variables that are set inside -SkipDeps block but used later
$qt6Dir = $null

# =====================================================================
# 1. DEPENDENCY CHECKS & INSTALLATION
# =====================================================================

if (-not $SkipDeps) {

    # ── 1a. Common tools ─────────────────────────────────────────────

    Write-Step "Checking common tools"

    # CMake
    if (-not (Test-Command cmake)) {
        Write-Warn "cmake not found – attempting install via winget"
        winget install --id Kitware.CMake --silent --accept-source-agreements --accept-package-agreements
        $env:Path = [System.Environment]::GetEnvironmentVariable('Path','Machine') + ';' + [System.Environment]::GetEnvironmentVariable('Path','User')
        if (-not (Test-Command cmake)) { Bail "cmake still not found after install. Please install CMake and add it to PATH." }
    }
    Write-Ok "cmake $(cmake --version | Select-Object -First 1)"

    # Ninja
    if (-not (Test-Command ninja)) {
        Write-Warn "ninja not found – attempting install via winget"
        winget install --id Ninja-build.Ninja --silent --accept-source-agreements --accept-package-agreements
        $env:Path = [System.Environment]::GetEnvironmentVariable('Path','Machine') + ';' + [System.Environment]::GetEnvironmentVariable('Path','User')
        if (-not (Test-Command ninja)) { Bail "ninja still not found after install. Please install Ninja and add it to PATH." }
    }
    Write-Ok "ninja $(ninja --version)"

    # Git (needed for version detection)
    if (-not (Test-Command git)) {
        Write-Warn "git not found – attempting install via winget"
        winget install --id Git.Git --silent --accept-source-agreements --accept-package-agreements
        $env:Path = [System.Environment]::GetEnvironmentVariable('Path','Machine') + ';' + [System.Environment]::GetEnvironmentVariable('Path','User')
    }
    if (Test-Command git) { Write-Ok "git $(git --version)" } else { Write-Warn "git not found – version detection may use fallback" }

    # ── 1b. MSVC / vcpkg dependencies (Qt6, daemon, cli) ────────────

    if ($NeedMSVC) {
        Write-Step "Checking MSVC toolchain"

        # Check for cl.exe – user should run from a VS Developer prompt or have
        # Visual Studio installed.  We try to find and invoke vcvarsall if needed.
        if (-not (Test-Command cl)) {
            Write-Warn "cl.exe not found in PATH – looking for Visual Studio..."
            $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
            if (Test-Path $vswhere) {
                $vsPath = & $vswhere -latest -property installationPath 2>$null
                if ($vsPath) {
                    $vcvarsall = Join-Path $vsPath 'VC\Auxiliary\Build\vcvarsall.bat'
                    if (Test-Path $vcvarsall) {
                        Write-Host "  Found vcvarsall.bat at $vcvarsall"
                        Write-Host "  Importing MSVC x64 environment..."
                        # Import environment from vcvarsall
                        $pinfo = New-Object System.Diagnostics.ProcessStartInfo
                        $pinfo.FileName = "cmd.exe"
                        $pinfo.Arguments = ('/c "{0}" x64 >nul 2>&1 & set' -f $vcvarsall)
                        $pinfo.RedirectStandardOutput = $true
                        $pinfo.UseShellExecute = $false
                        $pinfo.CreateNoWindow = $true
                        $p = [System.Diagnostics.Process]::Start($pinfo)
                        $output = $p.StandardOutput.ReadToEnd()
                        $p.WaitForExit()
                        foreach ($line in $output -split "`n") {
                            if ($line -match '^([^=]+)=(.*)$') {
                                [Environment]::SetEnvironmentVariable($Matches[1].Trim(), $Matches[2].Trim(), 'Process')
                            }
                        }
                        Write-Ok "MSVC environment imported"
                    }
                }
            }
            if (-not (Test-Command cl)) {
                Bail "MSVC compiler (cl.exe) not found. Please run this script from a Visual Studio Developer PowerShell, or install Visual Studio Build Tools."
            }
        }
        Write-Ok "cl.exe found"

        # vcpkg
        Write-Step "Checking vcpkg"
        $vcpkgRoot = if ($env:VCPKG_INSTALLATION_ROOT) { $env:VCPKG_INSTALLATION_ROOT.Trim() } else { $null }
        if (-not $vcpkgRoot -and $env:VCPKG_ROOT) { $vcpkgRoot = $env:VCPKG_ROOT.Trim() }
        if (-not $vcpkgRoot -and (Test-Path "C:\vcpkg")) { $vcpkgRoot = "C:\vcpkg" }
        if (-not $vcpkgRoot) {
            # Try to find vcpkg in PATH
            $vcpkgCmd = Get-Command vcpkg -ErrorAction SilentlyContinue
            if ($vcpkgCmd) { $vcpkgRoot = Split-Path $vcpkgCmd.Source }
        }

        $vcpkgExe = if ($vcpkgRoot) { Join-Path $vcpkgRoot 'vcpkg.exe' } else { $null }
        if (-not $vcpkgRoot -or -not ($vcpkgExe -and (Test-Path $vcpkgExe))) {
            Write-Warn "vcpkg not found – cloning into C:\vcpkg"
            git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
            Push-Location C:\vcpkg
            & .\bootstrap-vcpkg.bat -disableMetrics
            Pop-Location
            $vcpkgRoot = "C:\vcpkg"
        }
        $env:VCPKG_INSTALLATION_ROOT = $vcpkgRoot
        $env:Path = "$vcpkgRoot;$env:Path"
        Write-Ok "vcpkg at $vcpkgRoot"

        # Install vcpkg packages
        Write-Step "Installing vcpkg dependencies (x64-windows)"
        $vcpkgPackages = @(
            'openssl',
            'bzip2',
            'zlib',
            'miniupnpc',
            'pcre2',
            'lua',
            'libiconv',
            'libidn2',
            'gettext[tools]'
        )
        & vcpkg install --triplet x64-windows @vcpkgPackages
        if ($LASTEXITCODE -ne 0) { Bail "vcpkg install failed" }
        Write-Ok "vcpkg packages installed"

        # Add vcpkg tool directories to PATH so that cmake can find msgfmt etc.
        $vcpkgBase = "$vcpkgRoot\installed\x64-windows"
        $toolsBase = "$vcpkgBase\tools"
        if (Test-Path $toolsBase) {
            Get-ChildItem -Path $toolsBase -Directory -Recurse |
                Where-Object { Get-ChildItem $_.FullName -Filter "*.exe" -ErrorAction SilentlyContinue } |
                ForEach-Object { $env:Path = "$($_.FullName);$env:Path" }
        }
        $env:Path = "$vcpkgBase\bin;$env:Path"

        # Qt6
        Write-Step "Checking Qt6"
        $qt6Dir = $null

        # Check if Qt6 is already findable
        $testQt = cmake --find-package -DNAME=Qt6 -DCOMPILER_ID=MSVC -DLANGUAGE=CXX -DMODE=EXIST 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Ok "Qt6 already available via CMake"
        } else {
            # Look in common install locations
            $qtSearchPaths = @(
                "$env:QT_ROOT_DIR",
                "$env:Qt6_DIR",
                "C:\Qt",
                "$env:USERPROFILE\Qt"
            ) | Where-Object { $_ -and (Test-Path $_ -ErrorAction SilentlyContinue) }

            foreach ($qp in $qtSearchPaths) {
                $candidates = Get-ChildItem -Path $qp -Recurse -Filter "Qt6Config.cmake" -ErrorAction SilentlyContinue | Select-Object -First 1
                if ($candidates) {
                    $qt6Dir = $candidates.DirectoryName
                    break
                }
            }

            if (-not $qt6Dir) {
                Write-Warn "Qt6 not found automatically."
                Write-Host ""
                Write-Host "  Qt6 is required for the Qt interface. Please install Qt6 (>= 6.2) using"
                Write-Host "  one of these methods:"
                Write-Host ""
                Write-Host "    1. Qt Online Installer : https://www.qt.io/download-qt-installer"
                Write-Host "       (select Qt 6.x, MSVC 2022 64-bit, plus Qt Multimedia module)"
                Write-Host ""
                Write-Host "    2. aqtinstall (pip)    : pip install aqtinstall"
                Write-Host "       aqt install-qt windows desktop 6.8.3 win64_msvc2022_64 -m qtmultimedia"
                Write-Host ""
                Write-Host "  Then set the environment variable Qt6_DIR or CMAKE_PREFIX_PATH to point"
                Write-Host "  to the lib/cmake/Qt6 directory, or re-run with -SkipDeps if already set."
                Write-Host ""

                # Try aqtinstall as automatic fallback
                if (Test-Command pip) {
                    Write-Host "  Attempting automatic install via aqtinstall..." -ForegroundColor Yellow
                    $qtInstallDir = Join-Path $RepoRoot 'qt6-install'
                    pip install aqtinstall 2>$null
                    if (Test-Command aqt) {
                        & aqt install-qt windows desktop 6.8.3 win64_msvc2022_64 -m qtmultimedia --outputdir $qtInstallDir
                        if ($LASTEXITCODE -eq 0) {
                            $candidates = Get-ChildItem -Path $qtInstallDir -Recurse -Filter "Qt6Config.cmake" -ErrorAction SilentlyContinue | Select-Object -First 1
                            if ($candidates) {
                                $qt6Dir = $candidates.DirectoryName
                                Write-Ok "Qt6 installed to $qtInstallDir"
                            }
                        }
                    }
                }

                if (-not $qt6Dir) {
                    Bail "Qt6 not found. Please install it and try again."
                }
            }
            Write-Ok "Qt6 found at $qt6Dir"
        }
    }

    # ── 1c. MSYS2 dependencies (GTK3) ───────────────────────────────

    if ($NeedMSYS2) {
        Write-Step "Checking MSYS2 (for GTK3 build)"

        $msys2Root = $null
        foreach ($candidate in @("$env:MSYS2_ROOT", "C:\msys64", "C:\msys2", "$env:USERPROFILE\msys64")) {
            if ($candidate -and (Test-Path "$candidate\usr\bin\bash.exe")) {
                $msys2Root = $candidate
                break
            }
        }

        if (-not $msys2Root) {
            Write-Warn "MSYS2 not found – attempting install via winget"
            winget install --id MSYS2.MSYS2 --silent --accept-source-agreements --accept-package-agreements
            if (Test-Path "C:\msys64\usr\bin\bash.exe") {
                $msys2Root = "C:\msys64"
            } else {
                Bail "MSYS2 not found. Please install MSYS2 (https://www.msys2.org/) and ensure it's at C:\msys64."
            }
        }
        Write-Ok "MSYS2 at $msys2Root"

        # Install MSYS2/UCRT64 packages
        Write-Step "Installing MSYS2 UCRT64 packages"
        $msys2Bash = "$msys2Root\usr\bin\bash.exe"
        $msys2Packages = @(
            'mingw-w64-ucrt-x86_64-gcc',
            'mingw-w64-ucrt-x86_64-cmake',
            'mingw-w64-ucrt-x86_64-ninja',
            'mingw-w64-ucrt-x86_64-pkgconf',
            'mingw-w64-ucrt-x86_64-gtk3',
            'mingw-w64-ucrt-x86_64-openssl',
            'mingw-w64-ucrt-x86_64-bzip2',
            'mingw-w64-ucrt-x86_64-zlib',
            'mingw-w64-ucrt-x86_64-miniupnpc',
            'mingw-w64-ucrt-x86_64-pcre2',
            'mingw-w64-ucrt-x86_64-lua',
            'mingw-w64-ucrt-x86_64-libidn2',
            'mingw-w64-ucrt-x86_64-gettext-tools',
            'mingw-w64-ucrt-x86_64-gettext-runtime',
            'mingw-w64-ucrt-x86_64-libiconv',
            'mingw-w64-ucrt-x86_64-libnotify'
        )
        $pkgList = $msys2Packages -join ' '
        # Sync package databases (without -u to avoid core-system upgrades that kill bash).
        # Then install packages.
        & $msys2Bash --login -c "pacman -Sy --noconfirm 2>/dev/null; pacman -S --noconfirm --needed $pkgList"
        if ($LASTEXITCODE -ne 0) { Bail "MSYS2 package installation failed" }
        Write-Ok "MSYS2 UCRT64 packages installed"
    }

} else {
    Write-Step "Skipping dependency installation (-SkipDeps)"
}

# =====================================================================
# 1b. ENVIRONMENT SETUP (always runs, even with -SkipDeps)
# =====================================================================

if ($NeedMSVC) {
    # ── Import MSVC environment if cl.exe is not on PATH ─────────────
    if (-not (Test-Command cl)) {
        Write-Step "Importing MSVC environment"
        $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
        if (Test-Path $vswhere) {
            $vsPath = & $vswhere -latest -property installationPath 2>$null
            if ($vsPath) {
                $vcvarsall = Join-Path $vsPath 'VC\Auxiliary\Build\vcvarsall.bat'
                if (Test-Path $vcvarsall) {
                    Write-Host "  Found vcvarsall.bat at $vcvarsall"
                    $pinfo = New-Object System.Diagnostics.ProcessStartInfo
                    $pinfo.FileName = "cmd.exe"
                    $pinfo.Arguments = ('/c "{0}" x64 >nul 2>&1 & set' -f $vcvarsall)
                    $pinfo.RedirectStandardOutput = $true
                    $pinfo.UseShellExecute = $false
                    $pinfo.CreateNoWindow = $true
                    $p = [System.Diagnostics.Process]::Start($pinfo)
                    $output = $p.StandardOutput.ReadToEnd()
                    $p.WaitForExit()
                    foreach ($line in $output -split "`n") {
                        if ($line -match '^([^=]+)=(.*)$') {
                            [Environment]::SetEnvironmentVariable($Matches[1].Trim(), $Matches[2].Trim(), 'Process')
                        }
                    }
                    Write-Ok "MSVC environment imported"
                }
            }
        }
        if (-not (Test-Command cl)) {
            Bail "MSVC compiler (cl.exe) not found. Please run from a VS Developer prompt or install Visual Studio."
        }
    }

    # ── Detect vcpkg root ────────────────────────────────────────────
    $vcpkgRoot = if ($env:VCPKG_INSTALLATION_ROOT) { $env:VCPKG_INSTALLATION_ROOT.Trim() } else { $null }
    if (-not $vcpkgRoot -and $env:VCPKG_ROOT) { $vcpkgRoot = $env:VCPKG_ROOT.Trim() }
    if (-not $vcpkgRoot -and (Test-Path "C:\vcpkg")) { $vcpkgRoot = "C:\vcpkg" }
    if (-not $vcpkgRoot) {
        $vcpkgCmd = Get-Command vcpkg -ErrorAction SilentlyContinue
        if ($vcpkgCmd) { $vcpkgRoot = Split-Path $vcpkgCmd.Source }
    }
    if ($vcpkgRoot) {
        $env:VCPKG_INSTALLATION_ROOT = $vcpkgRoot
        $env:Path = "$vcpkgRoot;$env:Path"
        # Add vcpkg tool directories to PATH
        $vcpkgBase = "$vcpkgRoot\installed\x64-windows"
        $toolsBase = "$vcpkgBase\tools"
        if (Test-Path $toolsBase) {
            Get-ChildItem -Path $toolsBase -Directory -Recurse |
                Where-Object { Get-ChildItem $_.FullName -Filter "*.exe" -ErrorAction SilentlyContinue } |
                ForEach-Object { $env:Path = "$($_.FullName);$env:Path" }
        }
        $env:Path = "$vcpkgBase\bin;$env:Path"
    }

    # ── Detect Qt6 ──────────────────────────────────────────────────
    if (-not $qt6Dir) {
        $qtSearchPaths = @(
            "$env:QT_ROOT_DIR",
            "$env:Qt6_DIR",
            "C:\Qt",
            "$env:USERPROFILE\Qt"
        ) | Where-Object { $_ -and (Test-Path $_ -ErrorAction SilentlyContinue) }

        foreach ($qp in $qtSearchPaths) {
            $candidates = Get-ChildItem -Path $qp -Recurse -Filter "Qt6Config.cmake" -ErrorAction SilentlyContinue | Select-Object -First 1
            if ($candidates) {
                $qt6Dir = $candidates.DirectoryName
                break
            }
        }
    }

    # Add Qt bin dir to PATH so windeployqt is available
    if ($qt6Dir) {
        $qtBinDir = [IO.Path]::GetFullPath((Join-Path $qt6Dir '..\..\..\bin'))
        if (Test-Path $qtBinDir) {
            $env:Path = "$qtBinDir;$env:Path"
            Write-Ok "Added Qt bin dir to PATH: $qtBinDir"
        }
    }
}

# =====================================================================
# 2. BUILD: Qt6 + Daemon + CLI  (MSVC / Ninja)
# =====================================================================

if ($NeedMSVC) {

    Write-Step "Configuring MSVC build (Qt6 + Daemon + CLI)"

    if ($Clean -and (Test-Path $BuildDirQt)) {
        Write-Host "  Removing $BuildDirQt"
        Remove-Item -Recurse -Force $BuildDirQt
    }

    # Pre-create a directory junction for icons so CMake's create_symlink
    # does not fail on Windows (symlinks need Developer Mode; junctions do not).
    $iconsSrc = Join-Path $RepoRoot 'eiskaltdcpp-qt\icons'
    $iconsDst = Join-Path $BuildDirQt 'eiskaltdcpp-qt\icons'
    if ((Test-Path $iconsSrc) -and -not (Test-Path $iconsDst)) {
        $parent = Split-Path $iconsDst -Parent
        if (-not (Test-Path $parent)) { New-Item -ItemType Directory -Path $parent -Force | Out-Null }
        cmd /c mklink /J "`"$iconsDst`"" "`"$iconsSrc`"" >$null 2>&1
    }

    $cmakeArgs = @(
        '-B', $BuildDirQt,
        '-G', 'Ninja',
        "-DCMAKE_BUILD_TYPE=$BuildType",
        "-DCMAKE_INSTALL_PREFIX=$QtInstallPrefix",
        '-DCMAKE_POLICY_VERSION_MINIMUM=3.5'
    )

    # vcpkg toolchain
    $vcpkgRoot = $env:VCPKG_INSTALLATION_ROOT
    if ($vcpkgRoot) {
        $cmakeArgs += "-DCMAKE_TOOLCHAIN_FILE=$vcpkgRoot\scripts\buildsystems\vcpkg.cmake"
        $vcpkgBase = "$vcpkgRoot\installed\x64-windows"
        $cmakeArgs += "-DGETTEXT_SEARCH_PATH=$vcpkgBase"
    }

    # Qt6 hint
    if ($qt6Dir) {
        $cmakeArgs += "-DQt6_DIR=$qt6Dir"
    }

    # Qt6 interface ON
    $wantQt = ($Targets -contains 'qt')
    $wantDaemon = ($Targets -contains 'daemon')
    $wantCli = ($Targets -contains 'cli')

    $cmakeArgs += @(
        "-DUSE_QT6=$(if ($wantQt) { 'ON' } else { 'OFF' })",
        '-DUSE_GTK3=OFF',
        '-DUSE_JS=ON',
        '-DUSE_QT_QML=OFF',
        '-DUSE_MINIUPNP=ON',
        '-DWITH_DHT=ON',
        '-DUSE_IDN2=ON',
        '-DLUA_SCRIPT=ON',
        '-DPERL_REGEX=ON',
        '-DUSE_ASPELL=OFF',           # aspell not easily available on Windows/vcpkg
        '-DDBUS_NOTIFY=OFF',          # no D-Bus on Windows
        '-DWITH_EMOTICONS=ON',
        '-DWITH_SOUNDS=ON',
        '-DWITH_DEV_FILES=OFF',
        '-DOPENSSL_MSVC=ON',
        "-DNO_UI_DAEMON=$(if ($wantDaemon) { 'ON' } else { 'OFF' })",
        "-DJSONRPC_DAEMON=$(if ($wantDaemon -or $wantCli) { 'ON' } else { 'OFF' })",
        "-DUSE_CLI_JSONRPC=$(if ($wantCli) { 'ON' } else { 'OFF' })"
    )

    Write-Host "  cmake $($cmakeArgs -join ' ')" -ForegroundColor DarkGray
    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) { Bail "CMake configure failed (MSVC build)" }
    Write-Ok "Configuration succeeded"

    # Build
    Write-Step "Building MSVC targets (Qt6 + Daemon + CLI)"
    & cmake --build $BuildDirQt --parallel $Jobs
    if ($LASTEXITCODE -ne 0) { Bail "Build failed (MSVC)" }
    Write-Ok "Build succeeded"

    # Deploy DLLs to build directory so devs can run/debug from there
    $buildDirExeDirs = @()
    if ($wantQt) {
        $qtBuildExeDir = Join-Path $BuildDirQt 'eiskaltdcpp-qt'
        if (Test-Path $qtBuildExeDir) { $buildDirExeDirs += $qtBuildExeDir }
    }
    $daemonBuildExeDir = Join-Path $BuildDirQt 'eiskaltdcpp-daemon'
    if (Test-Path $daemonBuildExeDir) { $buildDirExeDirs += $daemonBuildExeDir }

    foreach ($exeDir in $buildDirExeDirs) {
        $dirLabel = "build-dir/$([IO.Path]::GetFileName($exeDir))"
        $isQtDir  = $exeDir -like '*eiskaltdcpp-qt*'
        Deploy-WindowsDlls -TargetDir $exeDir -Label $dirLabel `
            -VcpkgRoot $vcpkgRoot -BuildType $BuildType -DeployQt:($wantQt -and $isQtDir)
    }

    # Install
    Write-Step "Installing to $QtInstallPrefix"
    & cmake --install $BuildDirQt --config $BuildType
    if ($LASTEXITCODE -ne 0) { Bail "Install failed (MSVC)" }
    Write-Ok "Install succeeded"

    # Deploy DLLs to install directory
    Deploy-WindowsDlls -TargetDir $QtInstallPrefix -Label 'install-dir' `
        -VcpkgRoot $vcpkgRoot -BuildType $BuildType -DeployQt:$wantQt
}

# =====================================================================
# 3. BUILD: GTK3  (MSYS2 / UCRT64)
# =====================================================================

if ($NeedMSYS2) {

    Write-Step "Building GTK3 interface via MSYS2 UCRT64"

    # Find MSYS2
    $msys2Root = $null
    foreach ($candidate in @("$env:MSYS2_ROOT", "C:\msys64", "C:\msys2", "$env:USERPROFILE\msys64")) {
        if ($candidate -and (Test-Path "$candidate\usr\bin\bash.exe")) {
            $msys2Root = $candidate
            break
        }
    }
    if (-not $msys2Root) { Bail "MSYS2 not found" }

    if ($Clean -and (Test-Path $BuildDirGtk)) {
        Remove-Item -Recurse -Force $BuildDirGtk
    }

    # Convert Windows paths to MSYS2 paths
    $driveRe = "^(`[A-Za-z`]):"   # backtick-escaped brackets for PS 5.1 parser
    $repoMsys = ($RepoRoot -replace '\\','/') -replace $driveRe,'/$1'
    $repoMsys = $repoMsys.ToLower()
    $buildDirMsys = ($BuildDirGtk -replace '\\','/') -replace $driveRe,'/$1'
    $buildDirMsys = $buildDirMsys.ToLower()
    $installMsys = ($GtkInstallPrefix -replace '\\','/') -replace $driveRe,'/$1'
    $installMsys = $installMsys.ToLower()

    $msys2Bash = "$msys2Root\usr\bin\bash.exe"

    # Build script to run inside MSYS2
    # Build as an array of lines to avoid PS expanding $ in bash variables.
    $dollar = '$'
    $msys2Script = @(
        'export MSYSTEM=UCRT64',
        '# source profile with -e disabled (read -r PRINTER can fail)',
        'set +e',
        'source /etc/profile',
        'set -e',
        '',
        "cd `"$repoMsys`"",
        '',
        'echo "==> Configuring GTK3 build..."',
        "cmake -B `"$buildDirMsys`" -G Ninja \",
        "    -DCMAKE_BUILD_TYPE=$BuildType \",
        "    -DCMAKE_INSTALL_PREFIX=`"$installMsys`" \",
        '    -DUSE_QT6=OFF \',
        '    -DUSE_GTK3=ON \',
        '    -DUSE_MINIUPNP=ON \',
        '    -DWITH_DHT=ON \',
        '    -DUSE_IDN2=ON \',
        '    -DLUA_SCRIPT=ON \',
        '    -DPERL_REGEX=ON \',
        '    -DUSE_ASPELL=OFF \',
        '    -DUSE_LIBNOTIFY=ON \',
        '    -DUSE_LIBCANBERRA=OFF \',
        '    -DWITH_EMOTICONS=ON \',
        '    -DWITH_SOUNDS=ON \',
        '    -DWITH_DEV_FILES=OFF \',
        '    -DCMAKE_POLICY_VERSION_MINIMUM=3.5',
        '',
        'echo "==> Building GTK3..."',
        "cmake --build `"$buildDirMsys`" --parallel $Jobs",
        '',
        'echo "==> Installing GTK3..."',
        "cmake --install `"$buildDirMsys`" --config $BuildType",
        '',
        'echo "==> Bundling DLLs..."',
        "if command -v ldd >/dev/null 2>&1 && [ -f `"$installMsys/eiskaltdcpp-gtk.exe`" ]; then",
        '',
        '    # GTK3 runtime data (copy BEFORE ldd pass so loader DLLs are scanned too)',
        "    mkdir -p `"$installMsys/lib`"",
        "    cp -r /ucrt64/lib/gdk-pixbuf-2.0 `"$installMsys/lib/`" 2>/dev/null || true",
        "    mkdir -p `"$installMsys/share/glib-2.0`"",
        "    cp -r /ucrt64/share/glib-2.0/schemas `"$installMsys/share/glib-2.0/`" 2>/dev/null || true",
        "    /ucrt64/bin/glib-compile-schemas `"$installMsys/share/glib-2.0/schemas/`" 2>/dev/null || true",
        "    mkdir -p `"$installMsys/share/icons`"",
        "    cp -r /ucrt64/share/icons/Adwaita `"$installMsys/share/icons/`" 2>/dev/null || true",
        "    cp -r /ucrt64/share/icons/hicolor `"$installMsys/share/icons/`" 2>/dev/null || true",
        '',
        '    # Collect DLL deps from exe AND all bundled plugins/loaders',
        "    collect_dlls() {",
        "        ldd `"${dollar}1`" 2>/dev/null | grep -i '/ucrt64/' | awk '{print ${dollar}3}'",
        "    }",
        '',
        '    {',
        "        collect_dlls `"$installMsys/eiskaltdcpp-gtk.exe`"",
        "        for loader in `"$installMsys`"/lib/gdk-pixbuf-2.0/2.10.0/loaders/*.dll; do",
        "            [ -f `"${dollar}loader`" ] && collect_dlls `"${dollar}loader`"",
        '        done',
        "    } | sort -u | while read -r dll; do",
        "        cp -n `"${dollar}dll`" `"$installMsys/`" 2>/dev/null || true",
        '    done',
        '',
        '    # Regenerate loaders.cache with relative paths for the install dir',
        "    GDK_PIXBUF_MODULEDIR=`"$installMsys/lib/gdk-pixbuf-2.0/2.10.0/loaders`" /ucrt64/bin/gdk-pixbuf-query-loaders > `"$installMsys/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache`" 2>/dev/null || true",
        "    WINPREFIX=${dollar}(cygpath -m `"$installMsys`")",
        ('    sed -i "s|${WINPREFIX}/||g" ' + "`"$installMsys/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache`" 2>/dev/null || true"),
        '',
        '    echo "==> GTK3 build complete!"',
        'else',
        '    echo "==> GTK3 build complete (DLL bundling skipped - exe not found)"',
        'fi'
    ) -join "`n"

    $scriptFile = Join-Path $env:TEMP 'eiskaltdcpp-gtk-build.sh'
    # Write without BOM - bash chokes on the UTF-8 BOM that PS 5.1's Set-Content adds
    [System.IO.File]::WriteAllText($scriptFile, $msys2Script, [System.Text.UTF8Encoding]::new($false))
    $scriptFileMsys = ($scriptFile -replace '\\','/') -replace $driveRe,'/$1'
    $scriptFileMsys = $scriptFileMsys.ToLower()

    & $msys2Bash --login -c "bash '$scriptFileMsys'"
    if ($LASTEXITCODE -ne 0) { Bail "GTK3 build failed" }

    Remove-Item $scriptFile -ErrorAction SilentlyContinue
    Write-Ok "GTK3 build and install succeeded"
}

# =====================================================================
# 4. NSIS INSTALLER PACKAGING (Qt build only)
# =====================================================================

if ($NeedMSVC) {
    $nsisScript = Join-Path $RepoRoot 'windows\EiskaltDC++.nsi'
    $stagingDir = Join-Path $RepoRoot 'windows\installer'

    # Check that the Qt install directory has the expected exe
    $mainExe = Join-Path $QtInstallPrefix 'EiskaltDC++.exe'
    if (-not (Test-Path $mainExe)) {
        Write-Warn "Skipping NSIS installer: EiskaltDC++.exe not found in $QtInstallPrefix"
    } else {
        Write-Step "Staging files for NSIS installer..."

        # Clean and create staging directory
        if (Test-Path $stagingDir) { Remove-Item $stagingDir -Recurse -Force }
        New-Item -ItemType Directory -Path $stagingDir -Force | Out-Null

        # Copy executables
        Copy-Item (Join-Path $QtInstallPrefix 'EiskaltDC++.exe') $stagingDir
        $daemonSrc = Join-Path $QtInstallPrefix 'eiskaltdcpp-daemon.exe'
        if (Test-Path $daemonSrc) { Copy-Item $daemonSrc $stagingDir }

        # Copy dcppboot.xml from windows directory
        $bootXml = Join-Path $RepoRoot 'windows\dcppboot.xml'
        if (Test-Path $bootXml) { Copy-Item $bootXml $stagingDir }

        # Copy qt.conf
        $qtConf = Join-Path $QtInstallPrefix 'qt.conf'
        if (Test-Path $qtConf) { Copy-Item $qtConf $stagingDir }

        # Copy all DLLs
        Get-ChildItem (Join-Path $QtInstallPrefix '*.dll') | ForEach-Object {
            Copy-Item $_.FullName $stagingDir
        }

        # Copy docs directory
        $docsDir = Join-Path $QtInstallPrefix 'docs'
        if (Test-Path $docsDir) {
            Copy-Item $docsDir (Join-Path $stagingDir 'docs') -Recurse
        }

        # Copy resources directory
        $resDir = Join-Path $QtInstallPrefix 'resources'
        if (Test-Path $resDir) {
            Copy-Item $resDir (Join-Path $stagingDir 'resources') -Recurse
        }

        # Copy Qt6 plugin directories
        $qtPluginDirs = @('generic','iconengines','imageformats','multimedia',
                          'networkinformation','platforms','qml','qmltooling',
                          'sqldrivers','styles','tls')
        foreach ($dir in $qtPluginDirs) {
            $src = Join-Path $QtInstallPrefix $dir
            if (Test-Path $src) {
                Copy-Item $src (Join-Path $stagingDir $dir) -Recurse
            }
        }

        # Copy installer graphics from data/icons
        $iconsDir = Join-Path $RepoRoot 'data\icons'
        $icoFile  = Join-Path $iconsDir 'eiskaltdcpp.ico'
        $bmpFile  = Join-Path $iconsDir 'icon_164x314.bmp'
        if (Test-Path $icoFile) { Copy-Item $icoFile $stagingDir }
        if (Test-Path $bmpFile) { Copy-Item $bmpFile $stagingDir }

        # Copy LICENSE file
        $licFile = Join-Path $RepoRoot 'LICENSE'
        if (Test-Path $licFile) { Copy-Item $licFile $stagingDir }

        # Count staged files
        $stagedCount = (Get-ChildItem $stagingDir -Recurse -File).Count
        Write-Ok "Staged $stagedCount files into $stagingDir"

        # ── Locate makensis ─────────────────────────────────────────
        $makensis = $null
        $nsisSearchPaths = @(
            'C:\Program Files (x86)\NSIS\makensis.exe',
            'C:\Program Files\NSIS\makensis.exe',
            'C:\NSIS\makensis.exe'
        )
        foreach ($p in $nsisSearchPaths) {
            if (Test-Path $p) { $makensis = $p; break }
        }
        if (-not $makensis) {
            # Try PATH
            $found = Get-Command makensis -ErrorAction SilentlyContinue
            if ($found) { $makensis = $found.Source }
        }

        if (-not $makensis) {
            Write-Warn "makensis.exe not found. Install NSIS to build the installer."
            Write-Warn "  winget install NSIS.NSIS"
            Write-Warn "Files are staged in $stagingDir — you can run makensis manually:"
            Write-Warn '  makensis /Dshared=64 /Dversion=2.5.0 windows\EiskaltDC++.nsi'
        } else {
            Write-Step "Building NSIS installer with $makensis ..."

            # Extract version from CMakeLists.txt
            $versionLine = Select-String -Path (Join-Path $RepoRoot 'CMakeLists.txt') `
                -Pattern 'set\s*\(APP_VERSION\s+"([^"]+)"\)' | Select-Object -First 1
            $appVersion = '2.5.0'
            if ($versionLine) {
                $appVersion = $versionLine.Matches[0].Groups[1].Value
            }

            Push-Location (Join-Path $RepoRoot 'windows')
            & $makensis /V3 /Dshared=64 /Dversion=$appVersion 'EiskaltDC++.nsi'
            $nsisExitCode = $LASTEXITCODE
            Pop-Location

            if ($nsisExitCode -ne 0) {
                Write-Warn "NSIS installer build failed (exit code $nsisExitCode)"
            } else {
                $installerExe = Get-ChildItem (Join-Path $RepoRoot 'windows\EiskaltDC++*installer.exe') -ErrorAction SilentlyContinue | Select-Object -First 1
                if ($installerExe) {
                    Write-Ok "Installer created: $($installerExe.FullName)"
                } else {
                    Write-Ok "NSIS installer build completed."
                }
            }
        }
    }
}

# =====================================================================
# 5. SUMMARY
# =====================================================================

Write-Host ""
Write-Host "=============================================" -ForegroundColor Green
Write-Host "  Build Complete!" -ForegroundColor Green
Write-Host "=============================================" -ForegroundColor Green
Write-Host ""

if ($NeedMSVC) {
    Write-Host "  MSVC build output:" -ForegroundColor White
    Write-Host "    Build dir  : $BuildDirQt"
    Write-Host "    Install dir: $QtInstallPrefix"
    $qtExe = Get-ChildItem "$QtInstallPrefix\EiskaltDC++.exe" -ErrorAction SilentlyContinue
    $daemonExe = Get-ChildItem "$QtInstallPrefix\eiskaltdcpp-daemon.exe" -ErrorAction SilentlyContinue
    if ($qtExe)     { Write-Host "    Qt GUI     : $($qtExe.FullName)" -ForegroundColor Green }
    if ($daemonExe) { Write-Host "    Daemon     : $($daemonExe.FullName)" -ForegroundColor Green }
    if ($Targets -contains 'cli') {
        $cliScript = Get-ChildItem "$QtInstallPrefix" -Recurse -Filter "cli-jsonrpc*" -ErrorAction SilentlyContinue | Select-Object -First 1
        $cliDir = Join-Path (Join-Path $QtInstallPrefix 'resources') 'cli'
        if ($cliScript) { Write-Host "    CLI        : $($cliScript.FullName)" -ForegroundColor Green }
        else { Write-Host "    CLI        : Perl scripts installed in $cliDir" -ForegroundColor Green }
    }
    # Show installer if it exists
    $installerExe = Get-ChildItem (Join-Path $RepoRoot 'windows\EiskaltDC++*installer.exe') -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($installerExe) {
        Write-Host "    Installer  : $($installerExe.FullName)" -ForegroundColor Green
    }
    Write-Host ""
}

if ($NeedMSYS2) {
    Write-Host "  GTK3 build output:" -ForegroundColor White
    Write-Host "    Build dir  : $BuildDirGtk"
    Write-Host "    Install dir: $GtkInstallPrefix"
    $gtkExe = Get-ChildItem "$GtkInstallPrefix\eiskaltdcpp-gtk.exe" -ErrorAction SilentlyContinue
    if ($gtkExe) { Write-Host "    GTK GUI    : $($gtkExe.FullName)" -ForegroundColor Green }
    Write-Host ""
}

Write-Host "  To run the Qt interface:"
Write-Host "    $QtInstallPrefix\EiskaltDC++.exe"
Write-Host ""
if ($NeedMSYS2) {
    Write-Host "  To run the GTK interface:"
    Write-Host "    $GtkInstallPrefix\eiskaltdcpp-gtk.exe"
    Write-Host ""
}

Pop-Location
