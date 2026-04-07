; ---------------------------------------------------------------------------
; EiskaltDC++ NSIS Installer Script
; ---------------------------------------------------------------------------
; Usage:
;   makensis /Dversion=2.5.0 /Dshared=64 EiskaltDC++.nsi
;
; Expects all files staged in  windows\installer\  relative to this script.
; The build script (build-local.ps1) creates that directory automatically.
; ---------------------------------------------------------------------------

!include MUI2.nsh

; --- Architecture detection ------------------------------------------------
!ifdef static
    !if ${static} == 32
        !define arch_x86
    !endif
!endif

!ifdef shared
    !if ${shared} == 32
        !define arch_x86
    !endif
!endif

; --- Version ---------------------------------------------------------------
!ifdef version
    !define PRODUCT_DISPLAY_VERSION  "${version}"
!else
    !define PRODUCT_DISPLAY_VERSION  "X.Y.Z"
!endif

; --- Product metadata ------------------------------------------------------
!define PRODUCT_PUBLISHER            "EiskaltDC++"
!define PRODUCT_WEB_SITE             "https://github.com/eiskaltdcpp/eiskaltdcpp"
!ifdef arch_x86
    !define PRODUCT_INSTALL_DIR      "$PROGRAMFILES\EiskaltDC++"
    !define PRODUCT_NAME             "EiskaltDC++ ${PRODUCT_DISPLAY_VERSION} (32-bit)"
    !define PRODUCT_UNINST_KEY       "Software\Microsoft\Windows\CurrentVersion\Uninstall\EiskaltDC++"
!else
    !define PRODUCT_INSTALL_DIR      "$PROGRAMFILES64\EiskaltDC++"
    !define PRODUCT_NAME             "EiskaltDC++ ${PRODUCT_DISPLAY_VERSION} (64-bit)"
    !define PRODUCT_UNINST_KEY       "Software\Microsoft\Windows\CurrentVersion\Uninstall\EiskaltDC++64"
!endif
!define PRODUCT_UNINST_ROOT_KEY      "HKLM"

; --- MUI settings ----------------------------------------------------------
!define MUI_ICON                     "installer\eiskaltdcpp.ico"
!define MUI_UNICON                   "installer\eiskaltdcpp.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "installer\icon_164x314.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP_NOSTRETCH
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "installer\icon_164x314.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP_NOSTRETCH
!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_FINISHPAGE_TITLE_3LINES
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_UNFINISHPAGE_NOAUTOCLOSE

SetCompressor /SOLID lzma

; --- Pages -----------------------------------------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "installer\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\EiskaltDC++.exe"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; --- Languages -------------------------------------------------------------
!define MUI_LANGDLL_ALLLANGUAGES
!insertmacro MUI_LANGUAGE "English" ;first language is the default language
!insertmacro MUI_LANGUAGE "Basque"
!insertmacro MUI_LANGUAGE "Belarusian"
!insertmacro MUI_LANGUAGE "Bulgarian"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Greek"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "PortugueseBR"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Serbian"
!insertmacro MUI_LANGUAGE "SerbianLatin"
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "Swedish"
!insertmacro MUI_LANGUAGE "Turkish"
!insertmacro MUI_LANGUAGE "Ukrainian"
!insertmacro MUI_RESERVEFILE_LANGDLL

Function .onInit
    !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

; --- Installer properties --------------------------------------------------
Name "${PRODUCT_NAME}"
!ifdef arch_x86
    OutFile "EiskaltDC++-${PRODUCT_DISPLAY_VERSION}-x86-installer.exe"
!else
    OutFile "EiskaltDC++-${PRODUCT_DISPLAY_VERSION}-x86_64-installer.exe"
!endif
InstallDir "${PRODUCT_INSTALL_DIR}"
ShowInstDetails show
ShowUnInstDetails show
RequestExecutionLevel admin

; ===== Main install section ================================================
Section "EiskaltDC++" SEC_MAIN
    SectionIn RO ; required — user cannot deselect

    ; NSIS is a 32-bit application. On 64-bit Windows, registry writes from a
    ; 32-bit process are redirected to WOW6432Node by default. SetRegView 64
    ; forces writes to the native 64-bit registry hive so that Add/Remove
    ; Programs (a 64-bit app) can see our uninstall entry.
!ifndef arch_x86
    SetRegView 64
!endif

    SetOutPath $INSTDIR

    ; --- Core executables & config -----------------------------------------
    File "installer\EiskaltDC++.exe"
    File "installer\eiskaltdcpp-daemon.exe"
    File "installer\dcppboot.xml"

    ; --- Documentation & data ----------------------------------------------
    File /r "installer\docs"
    File /r "installer\resources"

    ; --- Shared (Qt6) runtime ----------------------------------------------
!ifdef shared
    File "installer\qt.conf"
    File "installer\*.dll"

    ; Qt6 plugin directories
    File /r "installer\generic"
    File /r "installer\iconengines"
    File /r "installer\imageformats"
    File /r "installer\multimedia"
    File /r "installer\networkinformation"
    File /r "installer\platforms"
    File /r "installer\sqldrivers"
    File /r "installer\styles"
    File /r "installer\tls"

    ; Optional Qt6 directories (only included if present in staging)
    File /nonfatal /r "installer\qml"
    File /nonfatal /r "installer\qmltooling"
!endif

    ; --- Uninstaller -------------------------------------------------------
    WriteUninstaller "$INSTDIR\uninstall.exe"

    ; --- Add/Remove Programs registry --------------------------------------
    WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "DisplayName"     "${PRODUCT_NAME}"
    WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "DisplayIcon"     "$INSTDIR\EiskaltDC++.exe"
    WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "DisplayVersion"  "${PRODUCT_DISPLAY_VERSION}"
    WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "URLInfoAbout"    "${PRODUCT_WEB_SITE}"
    WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "Publisher"       "${PRODUCT_PUBLISHER}"
    WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "InstallLocation" "$INSTDIR"
    WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "NoModify"        1
    WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "NoRepair"        1

    ; Estimated size (in KB) — NSIS sets this automatically from installed files

    ; --- Start Menu shortcuts -----------------------------------------------
    SetShellVarContext all
    !ifdef arch_x86
        CreateDirectory "$SMPROGRAMS\EiskaltDC++"
        CreateShortCut  "$SMPROGRAMS\EiskaltDC++\EiskaltDC++.lnk" "$INSTDIR\EiskaltDC++.exe"
        CreateShortCut  "$SMPROGRAMS\EiskaltDC++\Uninstall.lnk"   "$INSTDIR\uninstall.exe"
    !else
        CreateDirectory "$SMPROGRAMS\EiskaltDC++ (x64)"
        CreateShortCut  "$SMPROGRAMS\EiskaltDC++ (x64)\EiskaltDC++.lnk" "$INSTDIR\EiskaltDC++.exe"
        CreateShortCut  "$SMPROGRAMS\EiskaltDC++ (x64)\Uninstall.lnk"   "$INSTDIR\uninstall.exe"
    !endif

    ; --- Desktop shortcut ---------------------------------------------------
    CreateShortCut "$DESKTOP\EiskaltDC++.lnk" "$INSTDIR\EiskaltDC++.exe"
SectionEnd

; ===== Uninstall section ===================================================
Section "Uninstall"
!ifndef arch_x86
    SetRegView 64
!endif
    SetShellVarContext all

    ; Remove desktop shortcut
    Delete "$DESKTOP\EiskaltDC++.lnk"

    ; Remove Start Menu shortcuts
    !ifdef arch_x86
        RMDir /r "$SMPROGRAMS\EiskaltDC++"
    !else
        RMDir /r "$SMPROGRAMS\EiskaltDC++ (x64)"
    !endif

    ; Remove install directory
    RMDir /r "$INSTDIR"

    ; Remove registry keys
    DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY}
SectionEnd

