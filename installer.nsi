; uWidget NSIS Installer Script

;--------------------------------
; General
!define APP_NAME "uWidget"
!define COMP_NAME "uWidget Desktop System"
!define VERSION "1.1.0"
!define COMPANY "uWidget"
!define WEBSITE "https://github.com/your-repo/uWidget" ; Change to your project's website if you have one
!define EXE_NAME "uWidget.exe"

; MUI 2.0 interface
!include "MUI2.nsh"

;--------------------------------
; Installer Attributes

Name "${APP_NAME}"
BrandingText " " ; Remove NSIS branding
OutFile "uWidget-v${VERSION}-Installer.exe"
InstallDir "$PROGRAMFILES64\${APP_NAME}"
InstallDirRegKey HKCU "Software\${APP_NAME}" ""
RequestExecutionLevel admin ; Request admin rights

;--------------------------------
; Interface Configuration

!define MUI_ICON "icons\window.ico"
!define MUI_UNICON "icons\window.ico"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "icons\header_logo.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "icons\welcome_logo.bmp"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt" ; You should create a LICENSE.txt file
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; Languages

!insertmacro MUI_LANGUAGE "SimpChinese"

;--------------------------------
; Installer Sections

Section "MainSection" SEC_MAIN
  SetOutPath $INSTDIR
  
  ; Add all files from the deploy folder
  File /r "deploy\*.*"
  
  ; Store installation folder
  WriteRegStr HKCU "Software\${APP_NAME}" "" $INSTDIR
  
  ; Add uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  ; Create shortcuts
  CreateDirectory "$SMPROGRAMS\${APP_NAME}"
  CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${EXE_NAME}"
  CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${EXE_NAME}"

  ; Add to StartUp
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "${APP_NAME}" "$INSTDIR\${EXE_NAME}"

SectionEnd

;--------------------------------
; Uninstaller Section

Section "Uninstall"
  ; Remove from registry
  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "${APP_NAME}"
  DeleteRegKey HKCU "Software\${APP_NAME}"

  ; Remove files and directories
  RMDir /r "$INSTDIR"
  RMDir /r "$SMPROGRAMS\${APP_NAME}"
  Delete "$DESKTOP\${APP_NAME}.lnk"
SectionEnd 