@echo off
echo ========================================
echo Desktop Widget System - Build Release
echo ========================================
echo.

REM Configuration variables (modify according to your environment)
set "QT_DIR=C:\Qt\6.9.0\mingw_64"
set "CMAKE_DIR=C:\Program Files\CMake\bin"
set "NSIS_DIR=C:\Program Files (x86)\NSIS"
set "APP_NAME=DesktopWidgetSystem"

REM Check required tools
echo [1/6] Checking build environment...
if not exist "%QT_DIR%\bin\qmake.exe" (
    echo ERROR: Qt not found: %QT_DIR%
    echo Please modify QT_DIR variable in the script
    pause & exit /b 1
)

if not exist "%CMAKE_DIR%\cmake.exe" (
    echo ERROR: CMake not found: %CMAKE_DIR%
    echo Please modify CMAKE_DIR variable or ensure CMake is in PATH
    pause & exit /b 1
)

echo Environment check completed

REM Set environment variables
set "PATH=%QT_DIR%\bin;%CMAKE_DIR%;%PATH%"

REM Clean and create build directory
echo.
echo [2/6] Preparing build directory...
if exist "build" rmdir /s /q "build"
mkdir "build"
cd "build"

REM CMake configuration
echo.
echo [3/6] Configuring CMake project...
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="%QT_DIR%"
if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed
    pause & exit /b 1
)

REM Build project
echo.
echo [4/6] Building project...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo ERROR: Build failed
    pause & exit /b 1
)

echo Build completed successfully

REM Return to project root
cd ..

REM Deploy Qt dependencies
echo.
echo [5/6] Deploying Qt dependencies...
if exist "deploy" rmdir /s /q "deploy"
mkdir "deploy"

copy "build\%APP_NAME%.exe" "deploy\"
if %errorlevel% neq 0 (
    echo ERROR: Failed to copy executable
    pause & exit /b 1
)

cd "deploy"
windeployqt.exe "%APP_NAME%.exe" --compiler-runtime --no-translations --no-system-d3d-compiler --no-opengl-sw
if %errorlevel% neq 0 (
    echo ERROR: Qt dependency deployment failed
    pause & exit /b 1
)

cd ..

REM Copy resource files
echo Copying resource files...
if exist "themes" xcopy "themes" "deploy\themes" /e /i /y
if exist "config" xcopy "config" "deploy\config" /e /i /y
if exist "LICENSE" copy "LICENSE" "deploy\"
if exist "README.md" copy "README.md" "deploy\"

REM Create necessary directories
mkdir "deploy\logs" 2>nul

echo Deployment completed successfully

REM Create installer (if NSIS available)
echo.
echo [6/6] Creating installer...
if exist "%NSIS_DIR%\makensis.exe" (
    if exist "LICENSE.txt" (
        "%NSIS_DIR%\makensis.exe" installer.nsi
        if %errorlevel% neq 0 (
            echo ERROR: Installer creation failed
        ) else (
            echo Installer created successfully: DesktopWidgetSystem_Setup.exe
        )
    ) else (
        echo WARNING: LICENSE.txt not found, skipping installer creation
        echo Please create LICENSE.txt and manually run: "%NSIS_DIR%\makensis.exe" installer.nsi
    )
) else (
    echo WARNING: NSIS not found, skipping installer creation
    echo Download NSIS from: https://nsis.sourceforge.io/
)

REM Output results
echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo Deploy files location: deploy\
echo Portable version: Copy deploy folder to target machine
echo.
if exist "DesktopWidgetSystem_Setup.exe" (
    echo Installer: DesktopWidgetSystem_Setup.exe
)
echo.
echo Testing recommendations:
echo 1. Test on current machine: deploy\%APP_NAME%.exe
echo 2. Test compatibility on VM or other machines
echo 3. Verify all features work correctly
echo.
pause 