# 寮哄埗浣跨敤UTF-8缂栫爜
# 娉ㄦ剰锛氳繖涓枃浠跺繀椤讳互UTF-8缂栫爜淇濆瓨锛屼笖涓嶅甫BOM
Unicode true

; ========================================
; 妗岄潰灏忕粍浠剁郴缁?- 澧炲己瀹夎鑴氭湰
; ========================================

!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "WinVer.nsh"

; ========================================
; 搴旂敤绋嬪簭淇℃伅
; ========================================
!define APP_NAME "妗岄潰灏忕粍浠剁郴缁?
!define APP_NAME_EN "DesktopWidgetSystem"
!define APP_VERSION "1.1.0"
!define APP_PUBLISHER "Widget Studio"
!define APP_EXE "DesktopWidgetSystem.exe"
!define APP_UNINSTALLER "Uninstall.exe"
!define APP_REGKEY "Software\${APP_NAME_EN}"
!define APP_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME_EN}"

; ========================================
; 瀹夎绋嬪簭璁剧疆
; ========================================
Name "${APP_NAME}"
OutFile "DesktopWidgetSystem_Setup.exe"
InstallDir "$PROGRAMFILES\${APP_NAME}"
InstallDirRegKey HKLM "${APP_REGKEY}" "InstallLocation"
RequestExecutionLevel admin
BrandingText "${APP_NAME} ${APP_VERSION} 瀹夎绋嬪簭"

; 妫€鏌indows鐗堟湰 (闇€瑕乄indows 10鎴栨洿楂樼増鏈?
${VersionCompare} ${WINVER_10} $R0
${If} $R0 == 1 ; 褰撳墠鐗堟湰浣庝簬Windows 10
    MessageBox MB_OK|MB_ICONSTOP "姝ょ▼搴忛渶瑕?Windows 10 鎴栨洿楂樼増鏈紒$\n褰撳墠绯荤粺鐗堟湰涓嶅彈鏀寔銆?
    Quit
${EndIf}

; ========================================
; 鐗堟湰淇℃伅
; ========================================
VIProductVersion "1.1.0.0"
VIAddVersionKey "ProductName" "${APP_NAME}"
VIAddVersionKey "CompanyName" "${APP_PUBLISHER}"
VIAddVersionKey "FileVersion" "${APP_VERSION}"
VIAddVersionKey "ProductVersion" "${APP_VERSION}"
VIAddVersionKey "FileDescription" "${APP_NAME} 瀹夎绋嬪簭"
VIAddVersionKey "LegalCopyright" "漏 2025 ${APP_PUBLISHER}. All rights reserved."

; ========================================
; 鐣岄潰璁剧疆
; ========================================
!define MUI_ABORTWARNING
!define MUI_ICON "icon.ico"
!define MUI_UNICON "icon.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "header.bmp" ; 闇€瑕?50x57鐨勫浘鐗?
!define MUI_WELCOMEFINISHPAGE_BITMAP "welcome.bmp" ; 闇€瑕?64x314鐨勫浘鐗?

; 鑷畾涔夐〉闈㈡枃鏈?
!define MUI_WELCOMEPAGE_TITLE "娆㈣繋瀹夎 ${APP_NAME}"
!define MUI_WELCOMEPAGE_TEXT "杩欏皢鍦ㄦ偍鐨勮绠楁満涓婂畨瑁?${APP_NAME}銆?\r$\n$\r$\n${APP_NAME} 鏄竴涓姛鑳藉己澶х殑妗岄潰灏忕粍浠剁郴缁燂紝鎻愪緵鏃堕挓銆佸ぉ姘斻€佺瑪璁扮瓑澶氱瀹炵敤宸ュ叿銆?\r$\n$\r$\n鐐瑰嚮"涓嬩竴姝?缁х画瀹夎銆?

!define MUI_LICENSEPAGE_TEXT_TOP "璇蜂粩缁嗛槄璇讳互涓嬭鍙崗璁細"
!define MUI_LICENSEPAGE_TEXT_BOTTOM "濡傛灉鎮ㄦ帴鍙楀崗璁腑鐨勬潯娆撅紝璇烽€夋嫨"鎴戞帴鍙?缁х画瀹夎銆?

!define MUI_DIRECTORYPAGE_TEXT_TOP "瀹夎绋嬪簭灏嗘妸 ${APP_NAME} 瀹夎鍒颁互涓嬫枃浠跺す涓€?\r$\n$\r$\n瑕佸畨瑁呭埌鍏朵粬鏂囦欢澶癸紝璇风偣鍑?娴忚"骞堕€夋嫨鍏朵粬鏂囦欢澶广€傜偣鍑?涓嬩竴姝?缁х画銆?

!define MUI_FINISHPAGE_TITLE "瀹夎瀹屾垚"
!define MUI_FINISHPAGE_TEXT "${APP_NAME} 宸叉垚鍔熷畨瑁呭埌鎮ㄧ殑璁＄畻鏈恒€?\r$\n$\r$\n鐐瑰嚮"瀹屾垚"鍏抽棴姝ゅ畨瑁呭悜瀵笺€?
!define MUI_FINISHPAGE_RUN "$INSTDIR\${APP_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "杩愯 ${APP_NAME}"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\鏇存柊鏃ュ織.md"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "鏌ョ湅鏇存柊鏃ュ織"
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED

; ========================================
; 瀹夎椤甸潰
; ========================================
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; ========================================
; 鍗歌浇椤甸潰
; ========================================
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; ========================================
; 璇█璁剧疆
; ========================================
!insertmacro MUI_LANGUAGE "SimpChinese"

; ========================================
; 瀹夎缁勪欢
; ========================================
InstType "瀹屾暣瀹夎"
InstType "鏈€灏忓畨瑁?

Section "!涓荤▼搴? SecMain
    SectionIn RO
    
    ; 妫€鏌eploy鐩綍
    IfFileExists "deploy\${APP_EXE}" +3 0
        MessageBox MB_OK|MB_ICONSTOP "閿欒锛氭湭鎵惧埌绋嬪簭鏂囦欢锛?\n璇风‘淇濆凡姝ｇ‘鏋勫缓椤圭洰銆?
        Abort
    
    ; 璁剧疆杈撳嚭璺緞
    SetOutPath "$INSTDIR"
    
    ; 鍋滄鍙兘姝ｅ湪杩愯鐨勭▼搴?
    DetailPrint "妫€鏌ュ苟鍋滄姝ｅ湪杩愯鐨勭▼搴?.."
    ${nsProcess::FindProcess} "${APP_EXE}" $R0
    ${If} $R0 = 0
        MessageBox MB_YESNO|MB_ICONQUESTION "妫€娴嬪埌 ${APP_NAME} 姝ｅ湪杩愯銆?\n闇€瑕佸厛鍏抽棴绋嬪簭鎵嶈兘缁х画瀹夎銆?\n$\n鏄惁鑷姩鍏抽棴绋嬪簭锛? IDNO AbortInstall
        ${nsProcess::KillProcess} "${APP_EXE}" $R0
        Sleep 2000
    ${EndIf}
    Goto ContinueInstall
    
    AbortInstall:
        Abort "瀹夎宸插彇娑堛€?
    
    ContinueInstall:
    
    ; 澶嶅埗鎵€鏈夋枃浠?
    DetailPrint "澶嶅埗绋嬪簭鏂囦欢..."
    File /r "deploy\*"
    
    ; 澶嶅埗鏇存柊鏃ュ織
    DetailPrint "澶嶅埗鏇存柊鏃ュ織..."
    File "鏇存柊鏃ュ織.md"
    
    ; 鍒涘缓蹇呰鐩綍
    CreateDirectory "$INSTDIR\logs"
    CreateDirectory "$INSTDIR\config"
    CreateDirectory "$INSTDIR\themes"
    
    ; 鍐欏叆娉ㄥ唽琛?
    WriteRegStr HKLM "${APP_REGKEY}" "InstallLocation" $INSTDIR
    WriteRegStr HKLM "${APP_REGKEY}" "Version" "${APP_VERSION}"
    WriteRegStr HKLM "${APP_REGKEY}" "Publisher" "${APP_PUBLISHER}"
    
    ; 鍒涘缓鍗歌浇绋嬪簭
    WriteUninstaller "$INSTDIR\${APP_UNINSTALLER}"
    
    ; 鍐欏叆鍗歌浇淇℃伅
    WriteRegStr HKLM "${APP_UNINST_KEY}" "DisplayName" "${APP_NAME}"
    WriteRegStr HKLM "${APP_UNINST_KEY}" "UninstallString" "$INSTDIR\${APP_UNINSTALLER}"
    WriteRegStr HKLM "${APP_UNINST_KEY}" "DisplayIcon" "$INSTDIR\${APP_EXE}"
    WriteRegStr HKLM "${APP_UNINST_KEY}" "Publisher" "${APP_PUBLISHER}"
    WriteRegStr HKLM "${APP_UNINST_KEY}" "DisplayVersion" "${APP_VERSION}"
    WriteRegStr HKLM "${APP_UNINST_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "${APP_UNINST_KEY}" "URLInfoAbout" "https://github.com/yourproject"
    WriteRegDWORD HKLM "${APP_UNINST_KEY}" "NoModify" 1
    WriteRegDWORD HKLM "${APP_UNINST_KEY}" "NoRepair" 1
    
    ; 璁＄畻瀹夎澶у皬
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKLM "${APP_UNINST_KEY}" "EstimatedSize" "$0"
    
SectionEnd

Section "妗岄潰蹇嵎鏂瑰紡" SecDesktop
    SectionIn 1
    CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0
SectionEnd

Section "寮€濮嬭彍鍗曞揩鎹锋柟寮? SecStartMenu
    SectionIn 1 2
    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\鍗歌浇 ${APP_NAME}.lnk" "$INSTDIR\${APP_UNINSTALLER}"
SectionEnd

Section "寮€鏈鸿嚜鍚姩" SecAutoStart
    SectionIn 1
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "${APP_NAME_EN}" "$INSTDIR\${APP_EXE}"
SectionEnd

Section "鏂囦欢鍏宠仈" SecFileAssoc
    SectionIn 1
    ; 鍏宠仈.widget鏂囦欢绫诲瀷锛堝鏋滈渶瑕侊級
    WriteRegStr HKCR ".widget" "" "${APP_NAME_EN}.widget"
    WriteRegStr HKCR "${APP_NAME_EN}.widget" "" "${APP_NAME} 灏忕粍浠舵枃浠?
    WriteRegStr HKCR "${APP_NAME_EN}.widget\DefaultIcon" "" "$INSTDIR\${APP_EXE},0"
    WriteRegStr HKCR "${APP_NAME_EN}.widget\shell\open\command" "" "$\"$INSTDIR\${APP_EXE}$\" $\"%1$\""
SectionEnd

; ========================================
; 缁勪欢鎻忚堪
; ========================================
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "瀹夎 ${APP_NAME} 鏍稿績绋嬪簭鏂囦欢锛堝繀闇€锛?
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "鍦ㄦ闈㈠垱寤哄揩鎹锋柟寮?
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} "鍦ㄥ紑濮嬭彍鍗曞垱寤虹▼搴忕粍"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecAutoStart} "璁剧疆绋嬪簭寮€鏈鸿嚜鍔ㄥ惎鍔?
    !insertmacro MUI_DESCRIPTION_TEXT ${SecFileAssoc} "鍏宠仈 .widget 鏂囦欢绫诲瀷"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; ========================================
; 瀹夎鍥炶皟鍑芥暟
; ========================================
Function .onInit
    ; 妫€鏌ユ槸鍚﹀凡瀹夎
    ReadRegStr $R0 HKLM "${APP_UNINST_KEY}" "DisplayName"
    StrCmp $R0 "" done
    
    MessageBox MB_YESNO|MB_ICONQUESTION "妫€娴嬪埌宸插畨瑁?${APP_NAME}銆?\n$\n鏄惁瑕佸崌绾у埌鏂扮増鏈紵$\n(閫夋嫨"鍚?灏嗗彇娑堝畨瑁?" IDYES upgrade IDNO 0
    Abort
    
    upgrade:
    ; 鎵ц闈欓粯鍗歌浇
    ReadRegStr $R1 HKLM "${APP_UNINST_KEY}" "UninstallString"
    ExecWait '$R1 /S'
    
    done:
FunctionEnd

Function .onInstSuccess
    MessageBox MB_YESNO|MB_ICONINFORMATION "${APP_NAME} 瀹夎瀹屾垚锛?\n$\n鏄惁绔嬪嵆杩愯绋嬪簭锛? IDNO end
    Exec "$INSTDIR\${APP_EXE}"
    end:
FunctionEnd

; ========================================
; 鍗歌浇閮ㄥ垎
; ========================================
Section "Uninstall"
    ; 鍋滄绋嬪簭
    DetailPrint "鍋滄姝ｅ湪杩愯鐨勭▼搴?.."
    ${nsProcess::FindProcess} "${APP_EXE}" $R0
    ${If} $R0 = 0
        ${nsProcess::KillProcess} "${APP_EXE}" $R0
        Sleep 2000
    ${EndIf}
    
    ; 鍒犻櫎娉ㄥ唽琛ㄩ」
    DeleteRegKey HKLM "${APP_UNINST_KEY}"
    DeleteRegKey HKLM "${APP_REGKEY}"
    DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "${APP_NAME_EN}"
    
    ; 鍒犻櫎鏂囦欢鍏宠仈
    DeleteRegKey HKCR ".widget"
    DeleteRegKey HKCR "${APP_NAME_EN}.widget"
    
    ; 鍒犻櫎鏂囦欢鍜岀洰褰?
    RMDir /r "$INSTDIR"
    
    ; 鍒犻櫎蹇嵎鏂瑰紡
    Delete "$DESKTOP\${APP_NAME}.lnk"
    RMDir /r "$SMPROGRAMS\${APP_NAME}"
    
    ; 娓呯悊绌虹殑绋嬪簭鐩綍
    RMDir "$PROGRAMFILES\${APP_NAME}"
    
SectionEnd

Function un.onInit
    MessageBox MB_YESNO|MB_ICONQUESTION "纭畾瑕佸畬鍏ㄧЩ闄?${APP_NAME} 鍚楋紵" IDYES +2
    Abort
FunctionEnd

Function un.onUninstSuccess
    MessageBox MB_OK|MB_ICONINFORMATION "${APP_NAME} 宸叉垚鍔熶粠鎮ㄧ殑璁＄畻鏈轰腑绉婚櫎銆?
FunctionEnd 