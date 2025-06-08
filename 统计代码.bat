@echo off
rem 桌面小组件系统 - 代码统计工具快速启动脚本
rem Desktop Widget System - Code Statistics Quick Launch Script
rem 
rem 作者：项目团队
rem 日期：2025-5
rem 版本：1.0.0

chcp 65001 >nul 2>&1
title 桌面小组件系统 - 代码统计工具

echo.
echo ========================================================
echo 桌面小组件系统 - 代码统计工具
echo Desktop Widget System - Code Statistics Tool
echo ========================================================
echo.

rem 检查Python是否可用
python --version >nul 2>&1
if %errorlevel% == 0 (
    echo 🐍 检测到Python环境，使用Python版本统计工具...
    echo.
    
    rem 检查是否存在Python脚本
    if exist "code_statistics.py" (
        python code_statistics.py
    ) else (
        echo ❌ 错误: 找不到code_statistics.py文件
        echo 请确保脚本文件与批处理文件在同一目录下
        pause
        exit /b 1
    )
) else (
    echo 🔍 未检测到Python环境，尝试使用PowerShell版本...
    echo.
    
    rem 检查PowerShell脚本是否存在
    if exist "code_statistics.ps1" (
        rem 检查PowerShell执行策略
        powershell -Command "Get-ExecutionPolicy" | findstr /i "restricted" >nul
        if %errorlevel% == 0 (
            echo ⚠️  PowerShell执行策略受限，正在临时调整...
            powershell -Command "Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force; & '.\code_statistics.ps1'"
        ) else (
            powershell -File "code_statistics.ps1"
        )
    ) else (
        echo ❌ 错误: 找不到code_statistics.ps1文件
        echo 请确保脚本文件与批处理文件在同一目录下
        pause
        exit /b 1
    )
)

echo.
echo ✅ 统计完成！
echo.
echo 📝 如需保存报告到文件，请使用以下命令：
echo    - Python版本: python code_statistics.py -o report.txt
echo    - PowerShell版本: .\code_statistics.ps1 -OutputFile report.txt
echo.
echo 💡 如需查看更多选项，请使用以下命令：
echo    - Python版本: python code_statistics.py --help
echo    - PowerShell版本: .\code_statistics.ps1 -Help
echo.
pause 