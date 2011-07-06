!include MUI2.nsh

!define PRODUCT_DISPLAY_VERSION      "2.2.3"
!define PRODUCT_NAME                 "EiskaltDC++ ${PRODUCT_DISPLAY_VERSION}"
!define PRODUCT_PUBLISHER            "EiskaltDC++"
!define PRODUCT_WEB_SITE             "http://code.google.com/p/eiskaltdc/"
!define PRODUCT_UNINST_KEY           "Software\Microsoft\Windows\CurrentVersion\Uninstall\EiskaltDC++"
!define PRODUCT_UNINST_ROOT_KEY      "HKLM"
!define PRODUCT_INSTALL_DIR          "$PROGRAMFILES\EiskaltDC++"
!define MUI_ICON                     "installer\eiskaltdcpp.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "installer\icon_164x314.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP_NOSTRETCH
!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_FINISHPAGE_TITLE_3LINES
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_UNFINISHPAGE_NOAUTOCLOSE

SetCompressor /SOLID lzma

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "installer\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\eiskaltdcpp-qt.exe"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!define MUI_LANGDLL_ALLLANGUAGES
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "Ukrainian"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "SerbianLatin"
!insertmacro MUI_LANGUAGE "Belarusian"
!insertmacro MUI_LANGUAGE "Bulgarian"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_RESERVEFILE_LANGDLL
Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

Name "${PRODUCT_NAME}"
OutFile "EiskaltDC++-${PRODUCT_DISPLAY_VERSION}-x86.exe"
InstallDir "${PRODUCT_INSTALL_DIR}"
ShowInstDetails show
ShowUnInstDetails show
RequestExecutionLevel admin

Section "EiskaltDC++"
  SetOutPath $INSTDIR
  File "installer\eiskaltdcpp-qt.exe"
  File "installer\eiskaltdcpp-daemon.exe"
  File "installer\dcppboot.xml"
  File "installer\qt.conf"
  File "installer\QtCore4.dll"
  File "installer\QtGui4.dll"
  File "installer\QtNetwork4.dll"
  File "installer\QtXml4.dll"
  File "installer\QtScript4.dll"
  File "installer\QtDeclarative4.dll"
  File "installer\QtSql4.dll"
  File "installer\QtXmlPatterns4.dll"
  File "installer\libgcc_s_dw2-1.dll"
  File "installer\libeay32.dll"
  File "installer\ssleay32.dll"
  File "installer\mingwm10.dll"
  File "installer\libintl-8.dll"
  File "installer\libaspell-15.dll"
  File "installer\lua51.dll"
  File "installer\libidn-11.dll"

  File "installer\iconv.dll"
  ;File "installer\libiconv-2.dll"

  File "installer\mgwz.dll"
  ;File "installer\zlib1.dll"

  File "installer\mgwbz2-1.dll"

  ;File "installer\libgcc_s_sjlj-1.dll"

  ;File "installer\libstdc++-6.dll"
  ;File "installer\libpcrecpp-0.dll"
  ;File "installer\libpcre-0.dll"

  File /r "installer\aspell"
  File /r "installer\plugins"
  File /r "installer\resources"
  File /r "installer\script"

  WriteUninstaller "$INSTDIR\uninstall.exe"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "DisplayName"     "${PRODUCT_NAME}"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "DisplayIcon"     "$INSTDIR\eiskaltdcpp-qt.exe"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "DisplayVersion"  "${PRODUCT_DISPLAY_VERSION}"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "URLInfoAbout"    "${PRODUCT_WEB_SITE}"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "Publisher"       "${PRODUCT_PUBLISHER}"
SectionEnd

Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\EiskaltDC++"
  CreateShortCut  "$SMPROGRAMS\EiskaltDC++\EiskaltDC++.lnk" "$INSTDIR\eiskaltdcpp-qt.exe"
  CreateShortCut  "$SMPROGRAMS\EiskaltDC++\Uninstall.lnk"   "$INSTDIR\uninstall.exe"
SectionEnd

Section "Uninstall"
  RMDir /r "$SMPROGRAMS\EiskaltDC++"
  RMDir /r "$INSTDIR"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY}
SectionEnd

