!include MUI2.nsh

!if ${static} == 32
    !define arch_x86
!endif

!if ${shared} == 32
    !define arch_x86
!endif

!ifdef version
    !define PRODUCT_DISPLAY_VERSION  "${version}"
!else
    !define PRODUCT_DISPLAY_VERSION  "X.Y.Z"
!endif

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
!define MUI_ICON                     "installer\eiskaltdcpp.ico"
;!define MUI_UNICON                   "installer\eiskaltdcpp.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "installer\icon_164x314.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP_NOSTRETCH
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "installer\icon_164x314.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP_NOSTRETCH
!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_FINISHPAGE_TITLE_3LINES
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_UNFINISHPAGE_NOAUTOCLOSE

SetCompressor /SOLID lzma

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "installer\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
;!define MUI_FINISHPAGE_RUN
;!define MUI_FINISHPAGE_RUN_FUNCTION "RunEiskaltDC++"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

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
;!insertmacro MUI_LANGUAGE "Chinese (China)"
;!insertmacro MUI_LANGUAGE "Vietnamese"
!insertmacro MUI_RESERVEFILE_LANGDLL
Function .onInit
    !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

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

;Function RunEiskaltDC++
;  ShellExecAsUser::ShellExecAsUser "" "$INSTDIR/EiskaltDC++.exe" ""
;FunctionEnd

Section "EiskaltDC++"
    SetOutPath $INSTDIR
    File "installer\EiskaltDC++.exe"
    File "installer\eiskaltdcpp-daemon.exe"
    File "installer\dcppboot.xml"

    File /r "installer\docs"
    File /r "installer\resources"
    File /r "installer\web-ui"

!ifdef shared
    File "installer\qt.conf"
    File "installer\*.dll"

    File /r "installer\plugins"
!endif

    WriteUninstaller "$INSTDIR\uninstall.exe"
    WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "DisplayName"     "${PRODUCT_NAME}"
    WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "DisplayIcon"     "$INSTDIR\EiskaltDC++.exe"
    WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "DisplayVersion"  "${PRODUCT_DISPLAY_VERSION}"
    WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "URLInfoAbout"    "${PRODUCT_WEB_SITE}"
    WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "Publisher"       "${PRODUCT_PUBLISHER}"
SectionEnd

Section "Start Menu Shortcuts"
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
SectionEnd

Section "Uninstall"
    SetShellVarContext all
    !ifdef arch_x86
        RMDir /r "$SMPROGRAMS\EiskaltDC++"
    !else
        RMDir /r "$SMPROGRAMS\EiskaltDC++ (x64)"
    !endif
    RMDir /r "$INSTDIR"
    DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY}
SectionEnd

