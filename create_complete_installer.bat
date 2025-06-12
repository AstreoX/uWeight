@echo off
chcp 65001 >nul
echo ========================================
echo Desktop Widget System - 完整安装程序制作
echo ========================================
echo.

REM 配置变量 (根据您的环境修改)
set "QT_DIR=C:\Qt\6.9.0\mingw_64"
set "CMAKE_DIR=C:\Qt\Tools\CMake_64\bin"
set "MINGW_DIR=C:\Qt\Tools\mingw1310_64\bin"
set "NSIS_DIR=C:\Program Files (x86)\NSIS"
set "APP_NAME=DesktopWidgetSystem"
set "APP_VERSION=1.0.0"

echo [INFO] 开始检查构建环境...

REM 检查必要工具
if not exist "%QT_DIR%\bin\qmake.exe" (
    echo [ERROR] Qt未找到: %QT_DIR%
    echo 请修改脚本中的QT_DIR变量
    pause & exit /b 1
)

if not exist "%CMAKE_DIR%\cmake.exe" (
    echo [ERROR] CMake未找到: %CMAKE_DIR%
    echo 请修改脚本中的CMAKE_DIR变量
    pause & exit /b 1
)

if not exist "%MINGW_DIR%\g++.exe" (
    echo [ERROR] MinGW未找到: %MINGW_DIR%
    echo 请修改脚本中的MINGW_DIR变量
    pause & exit /b 1
)

echo [OK] 构建环境检查完成

REM 设置环境变量
set "PATH=%QT_DIR%\bin;%CMAKE_DIR%;%MINGW_DIR%;%PATH%"

REM 第1步：清理并构建Release版本
echo.
echo [1/6] 清理和构建项目...
if exist "build\Desktop_Qt_6_9_0_MinGW_64_bit-Release" rmdir /s /q "build\Desktop_Qt_6_9_0_MinGW_64_bit-Release"
mkdir "build\Desktop_Qt_6_9_0_MinGW_64_bit-Release"
cd "build\Desktop_Qt_6_9_0_MinGW_64_bit-Release"

echo [INFO] 配置CMake项目...
cmake ..\.. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="%QT_DIR%"
if %errorlevel% neq 0 (
    echo [ERROR] CMake配置失败
    pause & exit /b 1
)

echo [INFO] 编译项目...
cmake --build . --config Release -j %NUMBER_OF_PROCESSORS%
if %errorlevel% neq 0 (
    echo [ERROR] 编译失败
    pause & exit /b 1
)

cd ..\..
echo [OK] 项目构建完成

REM 第2步：部署Qt依赖
echo.
echo [2/6] 部署Qt依赖...
if exist "deploy" rmdir /s /q "deploy"
mkdir "deploy"

REM 复制主程序
if exist "build\Desktop_Qt_6_9_0_MinGW_64_bit-Release\bin\%APP_NAME%.exe" (
    copy "build\Desktop_Qt_6_9_0_MinGW_64_bit-Release\bin\%APP_NAME%.exe" "deploy\"
) else (
    echo [ERROR] 未找到编译后的exe文件
    echo 请检查路径: build\Desktop_Qt_6_9_0_MinGW_64_bit-Release\bin\%APP_NAME%.exe
    pause & exit /b 1
)

REM 使用windeployqt部署Qt依赖
cd "deploy"
echo [INFO] 运行windeployqt...
windeployqt.exe "%APP_NAME%.exe" --compiler-runtime --no-translations --no-system-d3d-compiler --no-opengl-sw --verbose 2
if %errorlevel% neq 0 (
    echo [ERROR] Qt依赖部署失败
    pause & exit /b 1
)
cd ..

echo [OK] Qt依赖部署完成

REM 第3步：复制资源文件
echo.
echo [3/6] 复制资源文件...
if exist "theme_source" xcopy "theme_source" "deploy\theme_source" /e /i /y /q
if exist "icons" xcopy "icons" "deploy\icons" /e /i /y /q
if exist "config" xcopy "config" "deploy\config" /e /i /y /q
if exist "LICENSE" copy "LICENSE" "deploy\"
if exist "README.md" copy "README.md" "deploy\"

REM 创建必要目录
mkdir "deploy\logs" 2>nul
mkdir "deploy\config" 2>nul
mkdir "deploy\themes" 2>nul

echo [OK] 资源文件复制完成

REM 第4步：创建图标文件
echo.
echo [4/6] 准备安装程序资源...
if not exist "icon.ico" (
    echo [INFO] 创建默认图标文件...
    REM 这里可以从项目的图标资源中复制或创建默认图标
    if exist "icons\app_icon.ico" (
        copy "icons\app_icon.ico" "icon.ico"
    ) else (
        echo [WARNING] 未找到图标文件，将使用默认图标
    )
)

REM 第5步：创建许可证文件
if not exist "LICENSE.txt" (
    echo [INFO] 创建LICENSE.txt...
    echo MIT License > LICENSE.txt
    echo. >> LICENSE.txt
    echo Copyright (c) 2025 Desktop Widget System >> LICENSE.txt
    echo. >> LICENSE.txt
    echo Permission is hereby granted, free of charge, to any person obtaining a copy >> LICENSE.txt
    echo of this software and associated documentation files (the "Software"), to deal >> LICENSE.txt
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

echo [OK] 安装程序资源准备完成

REM 第6步：创建NSIS安装程序
echo.
echo [5/6] 创建安装程序...
if exist "%NSIS_DIR%\makensis.exe" (
    echo [INFO] 使用NSIS创建安装程序...
    "%NSIS_DIR%\makensis.exe" installer.nsi
    if %errorlevel% neq 0 (
        echo [ERROR] 安装程序创建失败
        pause & exit /b 1
    ) else (
        echo [OK] 安装程序创建成功
    )
) else (
    echo [ERROR] NSIS未找到: %NSIS_DIR%
    echo 请从 https://nsis.sourceforge.io/ 下载安装NSIS
    echo 或修改脚本中的NSIS_DIR变量
    pause & exit /b 1
)

REM 第7步：测试和验证
echo.
echo [6/6] 验证打包结果...
if exist "DesktopWidgetSystem_Setup.exe" (
    for %%F in ("DesktopWidgetSystem_Setup.exe") do set "INSTALLER_SIZE=%%~zF"
    call :FormatBytes %INSTALLER_SIZE%
    echo [OK] 安装程序已创建: DesktopWidgetSystem_Setup.exe (!FORMATTED_SIZE!)
) else (
    echo [ERROR] 安装程序创建失败
    exit /b 1
)

if exist "deploy\%APP_NAME%.exe" (
    echo [OK] 便携版已准备: deploy\%APP_NAME%.exe
) else (
    echo [WARNING] 便携版验证失败
)

REM 显示结果
echo.
echo ========================================
echo 打包完成！
echo ========================================
echo.
echo 安装程序: DesktopWidgetSystem_Setup.exe
echo 便携版目录: deploy\
echo.
echo 建议测试步骤:
echo 1. 运行便携版: deploy\%APP_NAME%.exe
echo 2. 安装测试: 运行 DesktopWidgetSystem_Setup.exe
echo 3. 在其他机器上测试兼容性
echo.
echo 发布文件清单:
if exist "DesktopWidgetSystem_Setup.exe" echo - DesktopWidgetSystem_Setup.exe (安装程序)
if exist "deploy" echo - deploy\ 目录 (便携版)
echo.
pause
goto :eof

:FormatBytes
set "size=%1"
if %size% lss 1024 (
    set "FORMATTED_SIZE=%size% B"
    goto :eof
)
set /a size_kb=%size%/1024
if %size_kb% lss 1024 (
    set "FORMATTED_SIZE=%size_kb% KB"
    goto :eof
)
set /a size_mb=%size_kb%/1024
set "FORMATTED_SIZE=%size_mb% MB"
goto :eof 