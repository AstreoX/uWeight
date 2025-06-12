@echo off
echo Deploying Desktop Widget System from Qt Creator build...
echo.

REM Set variables for Qt Creator build
set "APP_NAME=DesktopWidgetSystem"
set "BUILD_DIR=build\Desktop_Qt_6_9_0_MinGW_64_bit-Release\bin"
set "DEPLOY_DIR=deploy"
set "QT_DIR=C:\Qt\6.9.0\mingw_64"

REM Check Qt installation path
if not exist "%QT_DIR%\bin\windeployqt.exe" (
    echo ERROR: Qt installation not found: %QT_DIR%
    echo Please modify QT_DIR variable in script to your Qt installation path
    pause
    exit /b 1
)

REM Check if build file exists
if not exist "%BUILD_DIR%\%APP_NAME%.exe" (
    echo ERROR: Built application not found: %BUILD_DIR%\%APP_NAME%.exe
    echo Please build Release version in Qt Creator first
    echo Expected path: %CD%\%BUILD_DIR%\%APP_NAME%.exe
    pause
    exit /b 1
)

REM Create deploy directory
if exist "%DEPLOY_DIR%" rmdir /s /q "%DEPLOY_DIR%"
mkdir "%DEPLOY_DIR%"

REM Copy executable file
echo Copying executable file...
copy "%BUILD_DIR%\%APP_NAME%.exe" "%DEPLOY_DIR%\"
if %errorlevel% neq 0 (
    echo ERROR: Failed to copy executable
    pause
    exit /b 1
)

REM Set Qt environment and run windeployqt
echo Deploying Qt dependencies...
set "PATH=%QT_DIR%\bin;%PATH%"
cd "%DEPLOY_DIR%"
windeployqt.exe "%APP_NAME%.exe" --compiler-runtime --no-translations --no-system-d3d-compiler --no-opengl-sw
if %errorlevel% neq 0 (
    echo ERROR: Qt dependency deployment failed
    pause
    exit /b 1
)

cd ..

REM Copy other necessary files
echo Copying config and resource files...
if exist "themes" xcopy "themes" "%DEPLOY_DIR%\themes" /e /i /y
if exist "config" xcopy "config" "%DEPLOY_DIR%\config" /e /i /y
if exist "resources" xcopy "resources" "%DEPLOY_DIR%\resources" /e /i /y
if exist "src\resources" xcopy "src\resources" "%DEPLOY_DIR%\resources" /e /i /y
if exist "LICENSE" copy "LICENSE" "%DEPLOY_DIR%\"
if exist "README.md" copy "README.md" "%DEPLOY_DIR%\"

REM Create necessary directories
mkdir "%DEPLOY_DIR%\logs" 2>nul

echo.
echo ========================================
echo Deployment completed successfully!
echo ========================================
echo.
echo Deploy files location: %DEPLOY_DIR%\
echo Portable version ready for distribution
echo.
echo To test: %DEPLOY_DIR%\%APP_NAME%.exe
echo.
pause 