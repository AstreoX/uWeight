# 桌面小组件系统 - 代码统计工具 (PowerShell版本)
# Desktop Widget System - Code Statistics Tool (PowerShell Version)
#
# 功能特性：
# - 统计项目中所有代码文件的行数
# - 支持C++、头文件、CMake等文件类型
# - 区分代码行、注释行、空行
# - 按文件类型和目录分类统计
# - 生成详细统计报告
#
# 作者：项目团队
# 日期：2025-5
# 版本：1.0.0

param(
    [string]$Path = ".",                    # 要统计的路径
    [string]$OutputFile = "",               # 输出报告文件
    [switch]$IncludeDocs = $false,          # 是否包含文档文件
    [switch]$Verbose = $false,              # 详细输出
    [switch]$Help = $false                  # 显示帮助
)

# 显示帮助信息
function Show-Help {
    Write-Host @"
桌面小组件系统 - 代码统计工具 (PowerShell版本)

用法: .\code_statistics.ps1 [参数]

参数:
  -Path <路径>           要统计的项目路径 (默认: 当前目录)
  -OutputFile <文件>     保存报告到指定文件
  -IncludeDocs          包含文档文件统计
  -Verbose              显示详细输出
  -Help                 显示此帮助信息

示例:
  .\code_statistics.ps1                                    # 统计当前目录
  .\code_statistics.ps1 -Path "C:\Projects\MyProject"     # 统计指定目录
  .\code_statistics.ps1 -OutputFile "report.txt"          # 保存报告到文件
  .\code_statistics.ps1 -IncludeDocs                      # 包含文档文件

支持的文件类型:
  - C++源文件: .cpp, .cxx, .cc, .c
  - 头文件: .h, .hpp, .hxx
  - CMake文件: .cmake, CMakeLists.txt
  - Qt文件: .pro, .pri, .qrc, .ui
  - 配置文件: .json, .xml, .ini, .cfg
  - 文档文件: .md, .rst, .txt (需要 -IncludeDocs 参数)
"@
}

# 如果请求帮助，显示帮助并退出
if ($Help) {
    Show-Help
    exit 0
}

# 检查路径是否存在
if (!(Test-Path $Path)) {
    Write-Host "❌ 错误: 路径不存在: $Path" -ForegroundColor Red
    exit 1
}

# 获取绝对路径
$AbsolutePath = Resolve-Path $Path

# 定义支持的文件扩展名
$CodeExtensions = @{
    '.cpp' = 'C++源文件'
    '.cxx' = 'C++源文件'
    '.cc'  = 'C++源文件'
    '.c'   = 'C源文件'
    '.h'   = 'C/C++头文件'
    '.hpp' = 'C++头文件'
    '.hxx' = 'C++头文件'
    '.py'  = 'Python文件'
    '.js'  = 'JavaScript文件'
    '.ts'  = 'TypeScript文件'
    '.java' = 'Java文件'
    '.cs'  = 'C#文件'
    '.go'  = 'Go文件'
    '.rs'  = 'Rust文件'
    '.php' = 'PHP文件'
}

$ConfigExtensions = @{
    '.cmake' = 'CMake文件'
    '.txt'   = 'CMake/文本文件'
    '.pro'   = 'Qt项目文件'
    '.pri'   = 'Qt包含文件'
    '.qrc'   = 'Qt资源文件'
    '.ui'    = 'Qt界面文件'
    '.json'  = 'JSON配置文件'
    '.xml'   = 'XML配置文件'
    '.yaml'  = 'YAML配置文件'
    '.yml'   = 'YAML配置文件'
    '.ini'   = 'INI配置文件'
    '.cfg'   = '配置文件'
}

$DocExtensions = @{
    '.md'   = 'Markdown文档'
    '.rst'  = 'reStructuredText文档'
    '.txt'  = '文本文档'
    '.adoc' = 'AsciiDoc文档'
}

# 需要排除的目录
$ExcludeDirs = @(
    '.git', '.svn', '.hg',
    '__pycache__', '.pytest_cache',
    'node_modules',
    'build', 'dist', 'out',
    '.vscode', '.idea',
    'bin', 'obj',
    'Debug', 'Release',
    'x64', 'x86',
    '.vs'
)

# 获取文件类型
function Get-FileType {
    param([string]$FilePath)
    
    $Extension = [System.IO.Path]::GetExtension($FilePath).ToLower()
    $FileName = [System.IO.Path]::GetFileName($FilePath)
    
    # 特殊处理CMakeLists.txt
    if ($FileName -eq "CMakeLists.txt") {
        return "CMake文件"
    }
    
    # 检查代码文件
    if ($CodeExtensions.ContainsKey($Extension)) {
        return $CodeExtensions[$Extension]
    }
    
    # 检查配置文件
    if ($ConfigExtensions.ContainsKey($Extension)) {
        return $ConfigExtensions[$Extension]
    }
    
    # 检查文档文件
    if ($DocExtensions.ContainsKey($Extension)) {
        return $DocExtensions[$Extension]
    }
    
    return "其他文件($Extension)"
}

# 判断是否为代码文件
function Is-CodeFile {
    param([string]$FilePath)
    
    $Extension = [System.IO.Path]::GetExtension($FilePath).ToLower()
    $FileName = [System.IO.Path]::GetFileName($FilePath)
    
    return ($CodeExtensions.ContainsKey($Extension) -or 
            $ConfigExtensions.ContainsKey($Extension) -or
            $FileName -eq "CMakeLists.txt")
}

# 分析文件内容
function Analyze-FileContent {
    param([string]$FilePath)
    
    try {
        # 尝试读取文件内容
        $Content = Get-Content $FilePath -Encoding UTF8 -ErrorAction SilentlyContinue
        if (!$Content) {
            # 尝试其他编码
            $Content = Get-Content $FilePath -Encoding Default -ErrorAction SilentlyContinue
        }
        
        if (!$Content) {
            if ($Verbose) {
                Write-Host "警告: 无法读取文件 $FilePath" -ForegroundColor Yellow
            }
            return @{
                TotalLines = 0
                CodeLines = 0
                CommentLines = 0
                BlankLines = 0
            }
        }
        
        $TotalLines = $Content.Count
        $CodeLines = 0
        $CommentLines = 0
        $BlankLines = 0
        $InMultilineComment = $false
        
        $Extension = [System.IO.Path]::GetExtension($FilePath).ToLower()
        $FileName = [System.IO.Path]::GetFileName($FilePath)
        
        foreach ($Line in $Content) {
            $TrimmedLine = $Line.Trim()
            
            # 空行
            if ([string]::IsNullOrEmpty($TrimmedLine)) {
                $BlankLines++
                continue
            }
            
            # C/C++风格的注释
            if ($Extension -in @('.cpp', '.cxx', '.cc', '.c', '.h', '.hpp', '.hxx', '.js', '.ts', '.java', '.cs', '.go', '.rs', '.php')) {
                # 多行注释处理
                if ($InMultilineComment) {
                    $CommentLines++
                    if ($TrimmedLine -match '\*/') {
                        $InMultilineComment = $false
                    }
                    continue
                }
                
                if ($TrimmedLine -match '^/\*') {
                    $CommentLines++
                    if ($TrimmedLine -notmatch '\*/') {
                        $InMultilineComment = $true
                    }
                    continue
                }
                
                # 单行注释
                if ($TrimmedLine -match '^//') {
                    $CommentLines++
                    continue
                }
            }
            # Python风格的注释
            elseif ($Extension -eq '.py') {
                if ($TrimmedLine -match '^#') {
                    $CommentLines++
                    continue
                }
                # Python多行字符串作为注释
                if ($TrimmedLine -match '^"""' -or $TrimmedLine -match "^'''") {
                    $CommentLines++
                    continue
                }
            }
            # CMake注释
            elseif ($Extension -eq '.cmake' -or $FileName -eq 'CMakeLists.txt') {
                if ($TrimmedLine -match '^#') {
                    $CommentLines++
                    continue
                }
            }
            
            # 其他情况视为代码行
            $CodeLines++
        }
        
        return @{
            TotalLines = $TotalLines
            CodeLines = $CodeLines
            CommentLines = $CommentLines
            BlankLines = $BlankLines
        }
    }
    catch {
        if ($Verbose) {
            Write-Host "错误: 分析文件 $FilePath 时出错: $($_.Exception.Message)" -ForegroundColor Red
        }
        return @{
            TotalLines = 0
            CodeLines = 0
            CommentLines = 0
            BlankLines = 0
        }
    }
}

# 初始化统计变量
$TotalFiles = 0
$TotalLines = 0
$TotalCodeLines = 0
$TotalCommentLines = 0
$TotalBlankLines = 0

$StatsByType = @{}
$StatsByDir = @{}
$FileDetails = @()

Write-Host "🚀 开始统计项目代码..." -ForegroundColor Green
Write-Host "📁 项目路径: $AbsolutePath" -ForegroundColor Cyan
Write-Host "📚 包含文档: $(if ($IncludeDocs) { '是' } else { '否' })" -ForegroundColor Cyan
Write-Host ""

# 扫描目录
$Files = Get-ChildItem -Path $AbsolutePath -Recurse -File | Where-Object {
    # 排除特定目录
    $RelativePath = $_.FullName.Substring($AbsolutePath.Path.Length + 1)
    $DirParts = $RelativePath.Split([System.IO.Path]::DirectorySeparatorChar)
    $ShouldExclude = $false
    
    foreach ($DirPart in $DirParts) {
        if ($DirPart -in $ExcludeDirs) {
            $ShouldExclude = $true
            break
        }
    }
    
    if ($ShouldExclude) {
        return $false
    }
    
    # 检查文件类型
    $IsCode = Is-CodeFile $_.FullName
    $IsDoc = $IncludeDocs -and $DocExtensions.ContainsKey([System.IO.Path]::GetExtension($_.Name).ToLower())
    
    return $IsCode -or $IsDoc
}

$FileCount = 0
foreach ($File in $Files) {
    $FileCount++
    $RelativePath = $File.FullName.Substring($AbsolutePath.Path.Length + 1)
    $FileType = Get-FileType $File.FullName
    
    if ($Verbose) {
        Write-Host "  统计: $RelativePath ($FileType)" -ForegroundColor Gray
    } else {
        # 显示进度
        if ($FileCount % 10 -eq 0) {
            Write-Host "." -NoNewline -ForegroundColor Yellow
        }
    }
    
    # 分析文件内容
    $Analysis = Analyze-FileContent $File.FullName
    
    if ($Analysis.TotalLines -gt 0) {
        # 更新总计
        $TotalFiles++
        $TotalLines += $Analysis.TotalLines
        $TotalCodeLines += $Analysis.CodeLines
        $TotalCommentLines += $Analysis.CommentLines
        $TotalBlankLines += $Analysis.BlankLines
        
        # 按类型统计
        if (!$StatsByType.ContainsKey($FileType)) {
            $StatsByType[$FileType] = @{
                Files = 0
                Lines = 0
                CodeLines = 0
                CommentLines = 0
                BlankLines = 0
                FileList = @()
            }
        }
        
        $StatsByType[$FileType].Files++
        $StatsByType[$FileType].Lines += $Analysis.TotalLines
        $StatsByType[$FileType].CodeLines += $Analysis.CodeLines
        $StatsByType[$FileType].CommentLines += $Analysis.CommentLines
        $StatsByType[$FileType].BlankLines += $Analysis.BlankLines
        $StatsByType[$FileType].FileList += $RelativePath
        
        # 按目录统计
        $DirKey = [System.IO.Path]::GetDirectoryName($RelativePath)
        if ([string]::IsNullOrEmpty($DirKey)) {
            $DirKey = "根目录"
        }
        
        if (!$StatsByDir.ContainsKey($DirKey)) {
            $StatsByDir[$DirKey] = @{
                Files = 0
                Lines = 0
                CodeLines = 0
                CommentLines = 0
                BlankLines = 0
            }
        }
        
        $StatsByDir[$DirKey].Files++
        $StatsByDir[$DirKey].Lines += $Analysis.TotalLines
        $StatsByDir[$DirKey].CodeLines += $Analysis.CodeLines
        $StatsByDir[$DirKey].CommentLines += $Analysis.CommentLines
        $StatsByDir[$DirKey].BlankLines += $Analysis.BlankLines
        
        # 保存文件详细信息
        $FileDetails += @{
            Path = $RelativePath
            Type = $FileType
            Lines = $Analysis.TotalLines
            CodeLines = $Analysis.CodeLines
            CommentLines = $Analysis.CommentLines
            BlankLines = $Analysis.BlankLines
            Size = $File.Length
        }
    }
}

if (!$Verbose) {
    Write-Host ""  # 换行
}

Write-Host ""
Write-Host "✅ 扫描完成!" -ForegroundColor Green
Write-Host "📊 统计结果: $TotalFiles 个文件，$($TotalLines.ToString('N0')) 行代码" -ForegroundColor Cyan
Write-Host ""

# 生成报告
$ReportLines = @()
$ReportLines += "=" * 80
$ReportLines += "桌面小组件系统 - 代码统计报告"
$ReportLines += "=" * 80
$ReportLines += "生成时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
$ReportLines += ""

# 总体统计
$ReportLines += "📊 总体统计"
$ReportLines += "-" * 40
$ReportLines += "文件总数:     $($TotalFiles.ToString('N0').PadLeft(8))"
$ReportLines += "代码总行数:   $($TotalLines.ToString('N0').PadLeft(8))"
$ReportLines += "有效代码行:   $($TotalCodeLines.ToString('N0').PadLeft(8))"
$ReportLines += "注释行数:     $($TotalCommentLines.ToString('N0').PadLeft(8))"
$ReportLines += "空行数:       $($TotalBlankLines.ToString('N0').PadLeft(8))"

if ($TotalLines -gt 0) {
    $CodeRatio = ($TotalCodeLines / $TotalLines) * 100
    $CommentRatio = ($TotalCommentLines / $TotalLines) * 100
    $BlankRatio = ($TotalBlankLines / $TotalLines) * 100
    
    $ReportLines += ""
    $ReportLines += "📈 代码组成比例"
    $ReportLines += "-" * 40
    $ReportLines += "有效代码:     $($CodeRatio.ToString('F1').PadLeft(6))%"
    $ReportLines += "注释:         $($CommentRatio.ToString('F1').PadLeft(6))%"
    $ReportLines += "空行:         $($BlankRatio.ToString('F1').PadLeft(6))%"
}

$ReportLines += ""

# 按文件类型统计
$ReportLines += "📁 按文件类型统计"
$ReportLines += "-" * 80
$ReportLines += "文件类型".PadRight(20) + "文件数".PadLeft(8) + "总行数".PadLeft(10) + "代码行".PadLeft(10) + "注释行".PadLeft(10) + "空行".PadLeft(8)
$ReportLines += "-" * 80

# 按行数排序
$SortedTypes = $StatsByType.GetEnumerator() | Sort-Object { $_.Value.Lines } -Descending

foreach ($TypeEntry in $SortedTypes) {
    $TypeName = $TypeEntry.Key
    $Stats = $TypeEntry.Value
    $ReportLines += "$($TypeName.PadRight(20)) $($Stats.Files.ToString().PadLeft(8)) $($Stats.Lines.ToString('N0').PadLeft(10)) $($Stats.CodeLines.ToString('N0').PadLeft(10)) $($Stats.CommentLines.ToString('N0').PadLeft(10)) $($Stats.BlankLines.ToString('N0').PadLeft(8))"
}

$ReportLines += ""

# 按目录统计
$ReportLines += "📂 按目录统计"
$ReportLines += "-" * 80
$ReportLines += "目录".PadRight(30) + "文件数".PadLeft(8) + "总行数".PadLeft(10) + "代码行".PadLeft(10) + "注释行".PadLeft(10) + "空行".PadLeft(8)
$ReportLines += "-" * 80

# 按行数排序
$SortedDirs = $StatsByDir.GetEnumerator() | Sort-Object { $_.Value.Lines } -Descending

foreach ($DirEntry in $SortedDirs) {
    $DirName = $DirEntry.Key
    $Stats = $DirEntry.Value
    $ReportLines += "$($DirName.PadRight(30)) $($Stats.Files.ToString().PadLeft(8)) $($Stats.Lines.ToString('N0').PadLeft(10)) $($Stats.CodeLines.ToString('N0').PadLeft(10)) $($Stats.CommentLines.ToString('N0').PadLeft(10)) $($Stats.BlankLines.ToString('N0').PadLeft(8))"
}

$ReportLines += ""

# 文件详细列表（前20个最大的文件）
$ReportLines += "📄 文件详细信息（按行数排序，前20个）"
$ReportLines += "-" * 100
$ReportLines += "文件路径".PadRight(50) + "类型".PadRight(15) + "总行数".PadLeft(8) + "代码行".PadLeft(8) + "注释行".PadLeft(8) + "大小(KB)".PadLeft(10)
$ReportLines += "-" * 100

# 按行数排序，取前20个
$SortedFiles = $FileDetails | Sort-Object Lines -Descending | Select-Object -First 20

foreach ($FileInfo in $SortedFiles) {
    $SizeKB = [math]::Round($FileInfo.Size / 1024, 1)
    $ReportLines += "$($FileInfo.Path.PadRight(50)) $($FileInfo.Type.PadRight(15)) $($FileInfo.Lines.ToString('N0').PadLeft(8)) $($FileInfo.CodeLines.ToString('N0').PadLeft(8)) $($FileInfo.CommentLines.ToString('N0').PadLeft(8)) $($SizeKB.ToString('F1').PadLeft(10))"
}

$ReportLines += ""
$ReportLines += "=" * 80
$ReportLines += "报告生成完成 - 共分析 $TotalFiles 个文件"
$ReportLines += "=" * 80

$Report = $ReportLines -join "`n"

# 输出报告
if ($OutputFile) {
    $Report | Out-File -FilePath $OutputFile -Encoding UTF8
    Write-Host "📝 统计报告已保存到: $OutputFile" -ForegroundColor Green
} else {
    Write-Host $Report
}

Write-Host ""
Write-Host "🎉 统计完成!" -ForegroundColor Green 