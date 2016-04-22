!include MUI2.nsh

; See http://nsis.sourceforge.net/Check_if_a_file_exists_at_compile_time for documentation
!macro !defineifexist _VAR_NAME _FILE_NAME
	!tempfile _TEMPFILE
	!ifdef NSIS_WIN32_MAKENSIS
		; Windows - cmd.exe
		!system 'if exist "${_FILE_NAME}" echo !define ${_VAR_NAME} > "${_TEMPFILE}"'
	!else
		; Posix - sh
		!system 'if [ -e "${_FILE_NAME}" ]; then echo "!define ${_VAR_NAME}" > "${_TEMPFILE}"; fi'
	!endif
	!include '${_TEMPFILE}'
	!delfile '${_TEMPFILE}'
	!undef _TEMPFILE
!macroend
!define !defineifexist "!insertmacro !defineifexist"

!if ${static} == 32
  !define arch_x86
!else
  !if ${static} != 64
    !define shared
    ${!defineifexist} arch_x86 "installer\libgcc_s_sjlj-1.dll"
  !endif
!endif

!define PRODUCT_DISPLAY_VERSION      "2.4.0"
!define PRODUCT_PUBLISHER            "EiskaltDC++"
!define PRODUCT_WEB_SITE             "https://github.com/eiskaltdcpp/eiskaltdcpp"
!ifdef arch_x86
  !define PRODUCT_INSTALL_DIR        "$PROGRAMFILES\EiskaltDC++"
  !define PRODUCT_NAME               "EiskaltDC++ ${PRODUCT_DISPLAY_VERSION}"
  !define PRODUCT_UNINST_KEY         "Software\Microsoft\Windows\CurrentVersion\Uninstall\EiskaltDC++"
!else
  !define PRODUCT_INSTALL_DIR        "$PROGRAMFILES64\EiskaltDC++"
  !define PRODUCT_NAME               "EiskaltDC++ ${PRODUCT_DISPLAY_VERSION} (64)"
  !define PRODUCT_UNINST_KEY         "Software\Microsoft\Windows\CurrentVersion\Uninstall\EiskaltDC++64"
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
;!insertmacro MUI_LANGUAGE "Vietnamese"
!insertmacro MUI_RESERVEFILE_LANGDLL
Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

Name "${PRODUCT_NAME}"
!ifdef arch_x86
  !ifdef shared
    OutFile "EiskaltDC++-${PRODUCT_DISPLAY_VERSION}-x86.exe"
  !else
    OutFile "EiskaltDC++-${PRODUCT_DISPLAY_VERSION}-x86-static.exe"
  !endif
!else
  !ifdef shared
    OutFile "EiskaltDC++-${PRODUCT_DISPLAY_VERSION}-x86_64.exe"
  !else
    OutFile "EiskaltDC++-${PRODUCT_DISPLAY_VERSION}-x86_64-static.exe"
  !endif
!endif
InstallDir "${PRODUCT_INSTALL_DIR}"
ShowInstDetails show
ShowUnInstDetails show
RequestExecutionLevel admin

;Function RunEiskaltDC++
;  ShellExecAsUser::ShellExecAsUser "" "$INSTDIR/eiskaltdcpp-qt.exe" ""
;FunctionEnd

Section "EiskaltDC++"
  SetOutPath $INSTDIR
  File "installer\eiskaltdcpp-qt.exe"
  File "installer\eiskaltdcpp-daemon.exe"
  File "installer\eiskaltdcpp-cli-jsonrpc"
  File "installer\cli-jsonrpc-config.pl"
  File "installer\dcppboot.xml"

!ifdef shared
  ;File "installer\qt.conf"
  File "installer\Qt5Concurrent.dll"
  File "installer\Qt5Core.dll"
  ;File "installer\Qt5Declarative.dll"
  File "installer\Qt5Gui.dll"
  File "installer\Qt5Multimedia.dll"
  File "installer\Qt5Network.dll"
  File "installer\Qt5Script.dll"
  File "installer\Qt5Sql.dll"
  File "installer\Qt5Widgets.dll"
  File "installer\Qt5Xml.dll"
  ;File "installer\Qt5XmlPatterns.dll"
  File "installer\libaspell-15.dll"
  File "installer\libboost_system-mt.dll"
  File "installer\libbz2.dll"
  File "installer\libeay32.dll"
  File "installer\libfreetype-6.dll"
  ;File "installer\libgcc_s_dw2-1.dll"
  !ifdef arch_x86
    File "installer\libgcc_s_sjlj-1.dll"
  !else
    File "installer\libgcc_s_seh-1.dll"
  !endif
  File "installer\libglib-2.0-0.dll"
  File "installer\libharfbuzz-0.dll"
  File "installer\libiconv-2.dll"
  File "installer\libidn-11.dll"
  File "installer\libintl-8.dll"
  File "installer\libjsoncpp.dll"
  File "installer\liblua5.3.dll"
  File "installer\libminiupnpc.dll"
  File "installer\libpcre16-0.dll"
  File "installer\libpcre-1.dll"
  File "installer\libpcrecpp-0.dll"
  File "installer\libpng16-16.dll"
  File "installer\libsqlite3-0.dll"
  File "installer\libstdc++-6.dll"
  ;File "installer\libwinpthread-1.dll"
  File "installer\ssleay32.dll"
  File "installer\zlib1.dll"

  SetOutPath "$INSTDIR\audio"
  File "/oname=qtaudio_windows.dll" "installer\audio\qtaudio_windows.dll"
  SetOutPath "$INSTDIR\platforms"
  File "/oname=qwindows.dll" "installer\platforms\qwindows.dll"
  SetOutPath "$INSTDIR\sqldrivers"
  File "/oname=qsqlite.dll" "installer\sqldrivers\qsqlite.dll"
!endif

  SetOutPath $INSTDIR
  File /r "installer\aspell"
  ;File /r "installer\plugins"
  File /r "installer\resources"

  WriteUninstaller "$INSTDIR\uninstall.exe"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "DisplayName"     "${PRODUCT_NAME}"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "DisplayIcon"     "$INSTDIR\eiskaltdcpp-qt.exe"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "DisplayVersion"  "${PRODUCT_DISPLAY_VERSION}"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "URLInfoAbout"    "${PRODUCT_WEB_SITE}"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "Publisher"       "${PRODUCT_PUBLISHER}"
SectionEnd

Section "Start Menu Shortcuts"
  SetShellVarContext all
  !ifdef arch_x86
    CreateDirectory "$SMPROGRAMS\EiskaltDC++"
    CreateShortCut  "$SMPROGRAMS\EiskaltDC++\EiskaltDC++.lnk" "$INSTDIR\eiskaltdcpp-qt.exe"
    CreateShortCut  "$SMPROGRAMS\EiskaltDC++\Uninstall.lnk"   "$INSTDIR\uninstall.exe"
  !else
    CreateDirectory "$SMPROGRAMS\EiskaltDC++ (64)"
    CreateShortCut  "$SMPROGRAMS\EiskaltDC++ (64)\EiskaltDC++ (64).lnk" "$INSTDIR\eiskaltdcpp-qt.exe"
    CreateShortCut  "$SMPROGRAMS\EiskaltDC++ (64)\Uninstall.lnk"   "$INSTDIR\uninstall.exe"
  !endif
SectionEnd

Section "Uninstall"
  SetShellVarContext all
  !ifdef arch_x86
    RMDir /r "$SMPROGRAMS\EiskaltDC++"
  !else
    RMDir /r "$SMPROGRAMS\EiskaltDC++ (64)"
  !endif
  RMDir /r "$INSTDIR"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY}
SectionEnd

