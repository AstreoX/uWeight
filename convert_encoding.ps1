# 将NSIS安装脚本转换为UTF-8编码（不带BOM）
$files = @(
    "installer_enhanced.nsi",
    "installer_simple.nsi",
    "installer_fixed.nsi",
    "installer.nsi"
)

foreach ($file in $files) {
    if (Test-Path $file) {
        Write-Host "Converting $file to UTF-8 without BOM..."
        $content = Get-Content $file -Raw
        $utf8NoBomEncoding = New-Object System.Text.UTF8Encoding $false
        [System.IO.File]::WriteAllText($file, $content, $utf8NoBomEncoding)
        Write-Host "Converted $file successfully."
    } else {
        Write-Host "File not found: $file"
    }
}

Write-Host "`nAll files have been converted to UTF-8 without BOM."
Write-Host "Press any key to continue..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown") 