!include MUI2.nsh

!define PRODUCT_NAME                 "EiskaltDC++ r1410"
!define PRODUCT_PUBLISHER            "EiskaltDC++"
!define PRODUCT_DISPLAY_VERSION      "r1410"
!define PRODUCT_WEB_SITE             "http://code.google.com/p/eiskaltdc/"
!define PRODUCT_UNINST_KEY           "Software\Microsoft\Windows\CurrentVersion\Uninstall\EiskaltDC++"
!define PRODUCT_UNINST_ROOT_KEY      "HKLM"
!define PRODUCT_INSTALL_DIR          "$PROGRAMFILES\EiskaltDC++"
!define MUI_ICON                     "installer\eiskaltdcpp.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "installer\icon_128x128.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP_NOSTRETCH
!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_UNFINISHPAGE_NOAUTOCLOSE

SetCompressor /SOLID lzma

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "installer\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
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
!insertmacro MUI_RESERVEFILE_LANGDLL
Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

Name "${PRODUCT_NAME}"
OutFile "EiskaltDC++-r1410_x86.exe"
InstallDir "${PRODUCT_INSTALL_DIR}"
ShowInstDetails show
ShowUnInstDetails show
RequestExecutionLevel admin

Section "EiskaltDC++"
  SetOutPath $INSTDIR
  File "installer\EiskaltDC++ Qt.exe"
  File "installer\dcppboot.xml"
  File "installer\iconv.dll"
  File "installer\intl.dll"
  File "installer\libeay32.dll"
  File "installer\ssleay32.dll"
  File "installer\libgcc_s_dw2-1.dll"
  File "installer\mgwbz2-1.dll"
  File "installer\mgwz.dll"
  File "installer\mingwm10.dll"
  File "installer\QtCore4.dll"
  File "installer\QtGui4.dll"
  File "installer\QtNetwork4.dll"
  File "installer\QtXml4.dll"
  File "installer\QtScript4.dll"
  File "installer\aspell-15.dll" 
  File /r "installer\resources"

  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "DisplayName"     "${PRODUCT_NAME}"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "DisplayIcon"     "$INSTDIR\EiskaltDC++.exe"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "DisplayVersion"  "${PRODUCT_DISPLAY_VERSION}"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "URLInfoAbout"    "${PRODUCT_WEB_SITE}"
  WriteRegStr   ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "Publisher"       "${PRODUCT_PUBLISHER}"
SectionEnd

Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\EiskaltDC++"
  CreateShortCut  "$SMPROGRAMS\EiskaltDC++\EiskaltDC++.lnk" "$INSTDIR\EiskaltDC++ Qt.exe"
  CreateShortCut  "$SMPROGRAMS\EiskaltDC++\Uninstall.lnk"   "$INSTDIR\uninstall.exe"
SectionEnd

Section "Uninstall"
  RMDir /r "$SMPROGRAMS\EiskaltDC++"
  RMDir /r "$INSTDIR"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY}
SectionEnd

