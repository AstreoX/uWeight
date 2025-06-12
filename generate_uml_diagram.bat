@echo off
chcp 65001 >nul
echo 正在生成桌面小部件系统UML类图...

REM 检查Java是否安装
java -version >nul 2>&1
if %errorlevel% neq 0 (
    echo 错误：未检测到Java环境，请先安装Java。
    echo 可以从以下地址下载：https://www.oracle.com/java/technologies/downloads/
    pause
    exit /b 1
)

REM 检查PlantUML JAR文件是否存在
if not exist plantuml.jar (
    echo 正在下载PlantUML...
    powershell -Command "Invoke-WebRequest -Uri 'https://github.com/plantuml/plantuml/releases/download/v1.2024.8/plantuml-1.2024.8.jar' -OutFile 'plantuml.jar'"
    if %errorlevel% neq 0 (
        echo 下载失败，请手动下载plantuml.jar到当前目录
        echo 下载地址：https://plantuml.com/zh/download
        pause
        exit /b 1
    )
)

REM 检查输入文件是否存在
if not exist "desktop_widget_system_uml.puml" (
    echo 错误：找不到输入文件 desktop_widget_system_uml.puml
    pause
    exit /b 1
)

REM 生成PNG图像
echo 正在生成PNG图像...
java -jar plantuml.jar -tpng "desktop_widget_system_uml.puml"

REM 生成SVG图像
echo 正在生成SVG图像...
java -jar plantuml.jar -tsvg "desktop_widget_system_uml.puml"

REM 生成PDF图像
echo 正在生成PDF图像...
java -jar plantuml.jar -tpdf "desktop_widget_system_uml.puml"

if exist "desktop_widget_system_uml.png" (
    echo ✅ PNG图像生成成功：desktop_widget_system_uml.png
)

if exist "desktop_widget_system_uml.svg" (
    echo ✅ SVG图像生成成功：desktop_widget_system_uml.svg
)

if exist "desktop_widget_system_uml.pdf" (
    echo ✅ PDF图像生成成功：desktop_widget_system_uml.pdf
)

echo.
echo 🎉 UML类图生成完成！
echo 生成的文件位于当前目录：
dir /b *.png *.svg *.pdf 2>nul | findstr "desktop_widget_system_uml"

echo.
pause 