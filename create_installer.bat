@echo off
echo Creating installer for Desktop Widget System...
echo.

REM Set variables
set "NSIS_DIR=C:\Program Files (x86)\NSIS"
set "APP_NAME=DesktopWidgetSystem"

REM Check if NSIS is installed
if not exist "%NSIS_DIR%\makensis.exe" (
    echo ERROR: NSIS not found at: %NSIS_DIR%
    echo Please install NSIS from: https://nsis.sourceforge.io/
    echo Or modify NSIS_DIR variable in this script
    pause
    exit /b 1
)

REM Check if deploy directory exists
if not exist "deploy\%APP_NAME%.exe" (
    echo ERROR: Deploy directory not found or incomplete
    echo Please run deploy_qtcreator.bat first to create the deploy folder
    pause
    exit /b 1
)

REM Check if update log exists
if not exist "更新日志.md" (
    echo ERROR: Update log file not found: 更新日志.md
    echo Please make sure the update log file exists
    pause
    exit /b 1
)

REM Convert NSIS scripts to UTF-8 without BOM
echo Converting NSIS scripts to UTF-8...
powershell -ExecutionPolicy Bypass -File convert_encoding.ps1
if %errorlevel% neq 0 (
    echo ERROR: Failed to convert file encoding
    pause
    exit /b 1
)

REM Create LICENSE.txt if it doesn't exist
if not exist "LICENSE.txt" (
    echo Creating LICENSE.txt...
    echo MIT License > LICENSE.txt
    echo. >> LICENSE.txt
    echo Copyright ^(c^) 2024 Widget Studio >> LICENSE.txt
    echo. >> LICENSE.txt
    echo Permission is hereby granted, free of charge, to any person obtaining a copy >> LICENSE.txt
    echo of this software and associated documentation files ^(the "Software"^), to deal >> LICENSE.txt
    echo in the Software without restriction, including without limitation the rights >> LICENSE.txt
    echo to use, copy, modify, merge, publish, distribute, sublicense, and/or sell >> LICENSE.txt
    echo copies of the Software, and to permit persons to whom the Software is >> LICENSE.txt
    echo furnished to do so, subject to the following conditions: >> LICENSE.txt
    echo. >> LICENSE.txt
    echo The above copyright notice and this permission notice shall be included in all >> LICENSE.txt
    echo copies or substantial portions of the Software. >> LICENSE.txt
    echo. >> LICENSE.txt
    echo THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR >> LICENSE.txt
    echo IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, >> LICENSE.txt
    echo FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE >> LICENSE.txt
    echo AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER >> LICENSE.txt
    echo LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, >> LICENSE.txt
    echo OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE >> LICENSE.txt
    echo SOFTWARE. >> LICENSE.txt
)

REM Copy update log to deploy directory
echo Copying update log...
copy "更新日志.md" "deploy\" /Y

REM Create installer
echo Building installer...
"%NSIS_DIR%\makensis.exe" installer_enhanced.nsi

if %errorlevel% neq 0 (
    echo ERROR: Installer creation failed
    pause
    exit /b 1
) else (
    echo.
    echo ========================================
    echo Installer created successfully!
    echo ========================================
    echo.
    echo Installer file: DesktopWidgetSystem_Setup.exe
    echo.
)

pause 