#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
修复文件编码为UTF-8
"""

import os
import sys
from pathlib import Path

def convert_to_utf8(file_path):
    """将文件转换为UTF-8编码"""
    try:
        # 尝试多种编码读取
        encodings = ['utf-8', 'gbk', 'gb2312', 'iso-8859-1', 'latin-1']
        
        content = None
        used_encoding = None
        
        for encoding in encodings:
            try:
                with open(file_path, 'r', encoding=encoding) as f:
                    content = f.read()
                used_encoding = encoding
                break
            except (UnicodeDecodeError, UnicodeError):
                continue
        
        if content is None:
            print(f"❌ 无法读取文件: {file_path}")
            return False
        
        # 如果已经是UTF-8，不需要转换
        if used_encoding == 'utf-8':
            print(f"✓ 已是UTF-8: {file_path}")
            return True
        
        # 转换为UTF-8
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        
        print(f"✓ 转换成功: {file_path} ({used_encoding} -> UTF-8)")
        return True
        
    except Exception as e:
        print(f"❌ 错误 {file_path}: {str(e)}")
        return False

def main():
    # 获取项目目录
    script_dir = Path(__file__).parent
    project_dir = script_dir
    
    print("=" * 60)
    print("转换源文件编码为UTF-8")
    print("=" * 60)
    
    # 转换所有 .c 和 .h 文件
    file_extensions = ['*.c', '*.h']
    total_files = 0
    success_files = 0
    
    for ext in file_extensions:
        for file_path in project_dir.glob(ext):  # 当前目录
            total_files += 1
            if convert_to_utf8(file_path):
                success_files += 1
        
        for file_path in (project_dir / 'UserApp').glob(ext):  # UserApp目录
            total_files += 1
            if convert_to_utf8(file_path):
                success_files += 1
        
        for file_path in (project_dir / 'Driver').glob(ext):  # Driver目录
            total_files += 1
            if convert_to_utf8(file_path):
                success_files += 1
    
    print()
    print("=" * 60)
    print(f"转换完成: {success_files}/{total_files} 个文件")
    print("=" * 60)

if __name__ == '__main__':
    main()
