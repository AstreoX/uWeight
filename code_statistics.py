#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
代码统计工具 - 桌面小组件系统项目
Code Statistics Tool for Desktop Widget System

功能特性：
- 递归统计项目中所有代码文件的行数
- 支持多种文件类型（C++、头文件、CMake等）
- 区分总行数、代码行数、注释行数、空行数
- 按文件类型和目录进行分类统计
- 生成详细的统计报告
- 支持排除特定目录和文件

作者：项目团队
日期：2025-5
版本：1.0.0
"""

import os
import re
import argparse
import json
from datetime import datetime
from pathlib import Path
from collections import defaultdict, Counter
from typing import Dict, List, Tuple, Set


class CodeStatistics:
    """代码统计分析器"""
    
    def __init__(self):
        """初始化统计器"""
        # 支持的代码文件扩展名
        self.code_extensions = {
            '.cpp': 'C++源文件',
            '.cxx': 'C++源文件', 
            '.cc': 'C++源文件',
            '.c': 'C源文件',
            '.h': 'C/C++头文件',
            '.hpp': 'C++头文件',
            '.hxx': 'C++头文件',
            '.py': 'Python文件',
            '.js': 'JavaScript文件',
            '.ts': 'TypeScript文件',
            '.java': 'Java文件',
            '.cs': 'C#文件',
            '.go': 'Go文件',
            '.rs': 'Rust文件',
            '.php': 'PHP文件',
            '.rb': 'Ruby文件',
            '.swift': 'Swift文件',
            '.kt': 'Kotlin文件',
            '.scala': 'Scala文件'
        }
        
        # 配置文件扩展名
        self.config_extensions = {
            '.cmake': 'CMake文件',
            '.txt': 'CMake/文本文件',  # CMakeLists.txt
            '.pro': 'Qt项目文件',
            '.pri': 'Qt包含文件',
            '.qrc': 'Qt资源文件',
            '.ui': 'Qt界面文件',
            '.json': 'JSON配置文件',
            '.xml': 'XML配置文件',
            '.yaml': 'YAML配置文件',
            '.yml': 'YAML配置文件',
            '.toml': 'TOML配置文件',
            '.ini': 'INI配置文件',
            '.conf': '配置文件',
            '.cfg': '配置文件'
        }
        
        # 文档文件扩展名
        self.doc_extensions = {
            '.md': 'Markdown文档',
            '.rst': 'reStructuredText文档',
            '.txt': '文本文档',
            '.adoc': 'AsciiDoc文档'
        }
        
        # 需要排除的目录
        self.exclude_dirs = {
            '.git', '.svn', '.hg',           # 版本控制
            '__pycache__', '.pytest_cache',  # Python缓存
            'node_modules',                  # Node.js
            'build', 'dist', 'out',         # 构建输出
            '.vscode', '.idea',             # IDE配置
            'bin', 'obj',                   # 编译输出
            'Debug', 'Release',             # Visual Studio
            'x64', 'x86',                   # 平台目录
            '.vs'                           # Visual Studio
        }
        
        # 需要排除的文件模式
        self.exclude_patterns = {
            r'.*\.o$',          # 目标文件
            r'.*\.obj$',        # 目标文件
            r'.*\.exe$',        # 可执行文件
            r'.*\.dll$',        # 动态库
            r'.*\.so$',         # 共享库
            r'.*\.a$',          # 静态库
            r'.*\.lib$',        # 静态库
            r'.*\.tmp$',        # 临时文件
            r'.*\.log$',        # 日志文件
            r'.*\.bak$',        # 备份文件
            r'.*~$',            # 临时文件
            r'.*\.swp$',        # Vim临时文件
            r'.*\.DS_Store$',   # macOS文件
            r'Thumbs\.db$'      # Windows缩略图
        }
        
        # 统计数据
        self.reset_statistics()
    
    def reset_statistics(self):
        """重置统计数据"""
        self.total_files = 0
        self.total_lines = 0
        self.total_code_lines = 0
        self.total_comment_lines = 0
        self.total_blank_lines = 0
        
        # 按文件类型统计
        self.stats_by_type = defaultdict(lambda: {
            'files': 0,
            'lines': 0,
            'code_lines': 0,
            'comment_lines': 0,
            'blank_lines': 0,
            'file_list': []
        })
        
        # 按目录统计
        self.stats_by_dir = defaultdict(lambda: {
            'files': 0,
            'lines': 0,
            'code_lines': 0,
            'comment_lines': 0,
            'blank_lines': 0
        })
        
        # 文件详细信息
        self.file_details = []
    
    def is_excluded_dir(self, dir_name: str) -> bool:
        """检查目录是否应该被排除"""
        return dir_name in self.exclude_dirs
    
    def is_excluded_file(self, file_name: str) -> bool:
        """检查文件是否应该被排除"""
        for pattern in self.exclude_patterns:
            if re.match(pattern, file_name, re.IGNORECASE):
                return True
        return False
    
    def get_file_type(self, file_path: str) -> str:
        """获取文件类型"""
        ext = Path(file_path).suffix.lower()
        
        # 特殊处理CMakeLists.txt
        if Path(file_path).name == 'CMakeLists.txt':
            return 'CMake文件'
        
        # 检查代码文件
        if ext in self.code_extensions:
            return self.code_extensions[ext]
        
        # 检查配置文件
        if ext in self.config_extensions:
            return self.config_extensions[ext]
        
        # 检查文档文件
        if ext in self.doc_extensions:
            return self.doc_extensions[ext]
        
        return f'其他文件({ext})'
    
    def is_code_file(self, file_path: str) -> bool:
        """判断是否为代码文件"""
        ext = Path(file_path).suffix.lower()
        file_name = Path(file_path).name
        
        return (ext in self.code_extensions or 
                ext in self.config_extensions or
                file_name == 'CMakeLists.txt')
    
    def analyze_file_content(self, file_path: str) -> Tuple[int, int, int, int]:
        """
        分析文件内容，返回(总行数, 代码行数, 注释行数, 空行数)
        """
        try:
            # 尝试不同的编码
            encodings = ['utf-8', 'gbk', 'gb2312', 'latin-1']
            content = None
            
            for encoding in encodings:
                try:
                    with open(file_path, 'r', encoding=encoding) as f:
                        content = f.readlines()
                    break
                except UnicodeDecodeError:
                    continue
            
            if content is None:
                print(f"警告: 无法读取文件 {file_path}")
                return 0, 0, 0, 0
            
            total_lines = len(content)
            code_lines = 0
            comment_lines = 0
            blank_lines = 0
            
            in_multiline_comment = False
            ext = Path(file_path).suffix.lower()
            
            for line in content:
                line = line.strip()
                
                # 空行
                if not line:
                    blank_lines += 1
                    continue
                
                # C/C++风格的注释
                if ext in ['.cpp', '.cxx', '.cc', '.c', '.h', '.hpp', '.hxx', '.js', '.ts', '.java', '.cs', '.go', '.rs', '.php', '.swift', '.kt', '.scala']:
                    # 多行注释处理
                    if in_multiline_comment:
                        comment_lines += 1
                        if '*/' in line:
                            in_multiline_comment = False
                        continue
                    
                    if line.startswith('/*'):
                        comment_lines += 1
                        if '*/' not in line:
                            in_multiline_comment = True
                        continue
                    
                    # 单行注释
                    if line.startswith('//'):
                        comment_lines += 1
                        continue
                
                # Python风格的注释
                elif ext == '.py':
                    if line.startswith('#'):
                        comment_lines += 1
                        continue
                    # Python多行字符串作为注释
                    if line.startswith('"""') or line.startswith("'''"):
                        comment_lines += 1
                        continue
                
                # CMake注释
                elif ext in ['.cmake'] or Path(file_path).name == 'CMakeLists.txt':
                    if line.startswith('#'):
                        comment_lines += 1
                        continue
                
                # 其他情况视为代码行
                code_lines += 1
            
            return total_lines, code_lines, comment_lines, blank_lines
            
        except Exception as e:
            print(f"错误: 分析文件 {file_path} 时出错: {e}")
            return 0, 0, 0, 0
    
    def scan_directory(self, root_path: str, include_docs: bool = False) -> None:
        """扫描目录统计代码"""
        root_path = Path(root_path).resolve()
        
        print(f"正在扫描目录: {root_path}")
        
        for root, dirs, files in os.walk(root_path):
            # 排除特定目录
            dirs[:] = [d for d in dirs if not self.is_excluded_dir(d)]
            
            rel_root = Path(root).relative_to(root_path)
            
            for file in files:
                if self.is_excluded_file(file):
                    continue
                
                file_path = Path(root) / file
                rel_file_path = rel_root / file
                
                # 检查是否为需要统计的文件
                if self.is_code_file(str(file_path)) or (include_docs and Path(file).suffix.lower() in self.doc_extensions):
                    file_type = self.get_file_type(str(file_path))
                    
                    # 分析文件内容
                    total, code, comment, blank = self.analyze_file_content(str(file_path))
                    
                    if total > 0:  # 只统计非空文件
                        # 更新总计
                        self.total_files += 1
                        self.total_lines += total
                        self.total_code_lines += code
                        self.total_comment_lines += comment
                        self.total_blank_lines += blank
                        
                        # 按类型统计
                        type_stats = self.stats_by_type[file_type]
                        type_stats['files'] += 1
                        type_stats['lines'] += total
                        type_stats['code_lines'] += code
                        type_stats['comment_lines'] += comment
                        type_stats['blank_lines'] += blank
                        type_stats['file_list'].append(str(rel_file_path))
                        
                        # 按目录统计
                        dir_key = str(rel_root) if str(rel_root) != '.' else '根目录'
                        dir_stats = self.stats_by_dir[dir_key]
                        dir_stats['files'] += 1
                        dir_stats['lines'] += total
                        dir_stats['code_lines'] += code
                        dir_stats['comment_lines'] += comment
                        dir_stats['blank_lines'] += blank
                        
                        # 保存文件详细信息
                        self.file_details.append({
                            'path': str(rel_file_path),
                            'type': file_type,
                            'lines': total,
                            'code_lines': code,
                            'comment_lines': comment,
                            'blank_lines': blank,
                            'size': file_path.stat().st_size
                        })
                        
                        print(f"  统计: {rel_file_path} ({file_type}) - {total} 行")
    
    def generate_report(self, output_file: str = None) -> str:
        """生成统计报告"""
        report_lines = []
        
        # 报告头部
        report_lines.append("=" * 80)
        report_lines.append("桌面小组件系统 - 代码统计报告")
        report_lines.append("=" * 80)
        report_lines.append(f"生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        report_lines.append("")
        
        # 总体统计
        report_lines.append("📊 总体统计")
        report_lines.append("-" * 40)
        report_lines.append(f"文件总数:     {self.total_files:>8,}")
        report_lines.append(f"代码总行数:   {self.total_lines:>8,}")
        report_lines.append(f"有效代码行:   {self.total_code_lines:>8,}")
        report_lines.append(f"注释行数:     {self.total_comment_lines:>8,}")
        report_lines.append(f"空行数:       {self.total_blank_lines:>8,}")
        
        if self.total_lines > 0:
            code_ratio = (self.total_code_lines / self.total_lines) * 100
            comment_ratio = (self.total_comment_lines / self.total_lines) * 100
            blank_ratio = (self.total_blank_lines / self.total_lines) * 100
            
            report_lines.append("")
            report_lines.append("📈 代码组成比例")
            report_lines.append("-" * 40)
            report_lines.append(f"有效代码:     {code_ratio:>6.1f}%")
            report_lines.append(f"注释:         {comment_ratio:>6.1f}%")
            report_lines.append(f"空行:         {blank_ratio:>6.1f}%")
        
        report_lines.append("")
        
        # 按文件类型统计
        report_lines.append("📁 按文件类型统计")
        report_lines.append("-" * 80)
        report_lines.append(f"{'文件类型':<20} {'文件数':<8} {'总行数':<10} {'代码行':<10} {'注释行':<10} {'空行':<8}")
        report_lines.append("-" * 80)
        
        # 按行数排序
        sorted_types = sorted(self.stats_by_type.items(), 
                            key=lambda x: x[1]['lines'], reverse=True)
        
        for file_type, stats in sorted_types:
            report_lines.append(
                f"{file_type:<20} "
                f"{stats['files']:<8} "
                f"{stats['lines']:<10,} "
                f"{stats['code_lines']:<10,} "
                f"{stats['comment_lines']:<10,} "
                f"{stats['blank_lines']:<8,}"
            )
        
        report_lines.append("")
        
        # 按目录统计
        report_lines.append("📂 按目录统计")
        report_lines.append("-" * 80)
        report_lines.append(f"{'目录':<30} {'文件数':<8} {'总行数':<10} {'代码行':<10} {'注释行':<10} {'空行':<8}")
        report_lines.append("-" * 80)
        
        # 按行数排序
        sorted_dirs = sorted(self.stats_by_dir.items(), 
                           key=lambda x: x[1]['lines'], reverse=True)
        
        for dir_name, stats in sorted_dirs:
            report_lines.append(
                f"{dir_name:<30} "
                f"{stats['files']:<8} "
                f"{stats['lines']:<10,} "
                f"{stats['code_lines']:<10,} "
                f"{stats['comment_lines']:<10,} "
                f"{stats['blank_lines']:<8,}"
            )
        
        report_lines.append("")
        
        # 文件详细列表（前20个最大的文件）
        report_lines.append("📄 文件详细信息（按行数排序，前20个）")
        report_lines.append("-" * 100)
        report_lines.append(f"{'文件路径':<50} {'类型':<15} {'总行数':<8} {'代码行':<8} {'注释行':<8} {'大小(KB)':<10}")
        report_lines.append("-" * 100)
        
        # 按行数排序，取前20个
        sorted_files = sorted(self.file_details, key=lambda x: x['lines'], reverse=True)[:20]
        
        for file_info in sorted_files:
            size_kb = file_info['size'] / 1024
            report_lines.append(
                f"{file_info['path']:<50} "
                f"{file_info['type']:<15} "
                f"{file_info['lines']:<8,} "
                f"{file_info['code_lines']:<8,} "
                f"{file_info['comment_lines']:<8,} "
                f"{size_kb:<10.1f}"
            )
        
        report_lines.append("")
        report_lines.append("=" * 80)
        report_lines.append(f"报告生成完成 - 共分析 {self.total_files} 个文件")
        report_lines.append("=" * 80)
        
        report_text = "\n".join(report_lines)
        
        # 保存到文件
        if output_file:
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write(report_text)
            print(f"\n📝 统计报告已保存到: {output_file}")
        
        return report_text
    
    def export_json(self, output_file: str) -> None:
        """导出JSON格式的统计数据"""
        data = {
            'timestamp': datetime.now().isoformat(),
            'summary': {
                'total_files': self.total_files,
                'total_lines': self.total_lines,
                'total_code_lines': self.total_code_lines,
                'total_comment_lines': self.total_comment_lines,
                'total_blank_lines': self.total_blank_lines
            },
            'stats_by_type': dict(self.stats_by_type),
            'stats_by_dir': dict(self.stats_by_dir),
            'file_details': self.file_details
        }
        
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(data, f, ensure_ascii=False, indent=2)
        
        print(f"📊 JSON数据已导出到: {output_file}")


def main():
    """主函数"""
    parser = argparse.ArgumentParser(
        description='桌面小组件系统代码统计工具',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例用法:
  python code_statistics.py                              # 统计当前目录
  python code_statistics.py /path/to/project            # 统计指定目录
  python code_statistics.py -o report.txt               # 保存报告到文件
  python code_statistics.py --include-docs              # 包含文档文件
  python code_statistics.py --json stats.json           # 导出JSON数据
        """
    )
    
    parser.add_argument('path', nargs='?', default='.', 
                       help='要统计的项目路径 (默认: 当前目录)')
    parser.add_argument('-o', '--output', 
                       help='保存报告到指定文件')
    parser.add_argument('--json', 
                       help='导出JSON格式数据到指定文件')
    parser.add_argument('--include-docs', action='store_true',
                       help='包含文档文件统计')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='显示详细输出')
    
    args = parser.parse_args()
    
    # 检查路径是否存在
    if not os.path.exists(args.path):
        print(f"❌ 错误: 路径不存在: {args.path}")
        return 1
    
    # 创建统计器
    stats = CodeStatistics()
    
    print(f"🚀 开始统计项目代码...")
    print(f"📁 项目路径: {os.path.abspath(args.path)}")
    print(f"📚 包含文档: {'是' if args.include_docs else '否'}")
    print()
    
    try:
        # 扫描目录
        stats.scan_directory(args.path, args.include_docs)
        
        print(f"\n✅ 扫描完成!")
        print(f"📊 统计结果: {stats.total_files} 个文件，{stats.total_lines:,} 行代码")
        print()
        
        # 生成报告
        report = stats.generate_report(args.output)
        
        # 如果没有指定输出文件，则打印到控制台
        if not args.output:
            print(report)
        
        # 导出JSON数据
        if args.json:
            stats.export_json(args.json)
        
        print(f"\n🎉 统计完成!")
        
    except KeyboardInterrupt:
        print(f"\n⚠️ 用户中断操作")
        return 1
    except Exception as e:
        print(f"\n❌ 发生错误: {e}")
        if args.verbose:
            import traceback
            traceback.print_exc()
        return 1
    
    return 0


if __name__ == '__main__':
    exit(main()) 