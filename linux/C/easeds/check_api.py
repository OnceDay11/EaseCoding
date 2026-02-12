#!/usr/bin/env python3
'''
SPDX-License-Identifier: BSD-3-Clause

Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.

FilePath: /linux/C/easeds/check_api.py
@Author: Once Day <once_day@qq.com>.
Date: 2026-02-12 22:48
@info: Encoder=utf-8, TabSize=4, Eol=\n.

@Description:
    This script is used to check the API symbols in the shared library.

@History:
    2026年2月12日, Once Day <once_day@qq.com>, 创建.

'''

import os
import sys
import subprocess


def get_ld_map_file_symbols(ld_map_file: str) -> set:
    """
    获取链接脚本中的符号列表:
    LIBISM_1.0 {
        global:
            # Sysrepo初始化函数
            ism_init_global_context;
            SysrepoRegisterIsmService;
            SysrepoRegisterIsmServiceCallback;
            # 日志Debug相关函数
            ism_log_enabled_debug;
            ism_log_enabled_verbose;
            ism_log_enabled_stderr_output;
            # 接口配置API函数
            ism_iface_type_to_str;
            ism_iface_config_alloc;
            ism_iface_config_free;
            ism_iface_config_set_name;
            ism_iface_config_get_name;
            ism_physical_iface_config_set_mac;
            ism_physical_iface_config_get_mac;
        local: *;
    };
    """
    symbols = set()
    version = ""
    with open(ld_map_file, "r") as f:
        for line in f:
            line = line.strip()
            # 跳过无关行
            if not line:
                continue
            if line.startswith("global:"):
                continue
            if line.startswith("local:"):
                continue
            if line.startswith("#"):
                continue
            # 如果 包括 "{", 记录符号版本
            if "{" in line:
                version = line.split("{")[0].strip()
                continue
            # 如果行中有 "}", 结束符号提取
            if "}" in line:
                if version:
                    version = ""
                continue
            # 提取符号名
            part = line.split(";")[0].strip()
            symbol = part + ("@@" + version) if version else part.strip()
            symbols.add(symbol)

    print(f"Found {len(symbols)} symbols in {ld_map_file}.")
    return symbols


def get_so_file_symbols(so_file: str) -> set:
    """
    获取共享库中的符号列表, 使用 nm -D --defined-only so_file.
    00000000000a6a13 T ism_iface_config_alloc@@LIBISM_1.0
    00000000000a6c13 T ism_iface_config_free@@LIBISM_1.0
    00000000000a6e56 T ism_iface_config_get_name@@LIBISM_1.0
    00000000000a6c95 T ism_iface_config_set_name@@LIBISM_1.0
    00000000000a69b9 T ism_iface_type_to_str@@LIBISM_1.0
    """
    symbols = set()
    if not os.path.exists(so_file):
        print(f"Shared object file {so_file} does not exist.")
        return symbols
    try:
        result = subprocess.run(
            ["nm", "-D", "--defined-only", so_file],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=True,
        )
        for line in result.stdout.splitlines():
            # 只有包含 T 的行才处理
            if " T " not in line:
                continue
            # 提取符号名, 例如: ism_iface_config_alloc@@LIBISM_1.0
            parts = line.split()
            if len(parts) < 3:
                continue
            symbol = parts[2]
            symbols.add(symbol)
    except subprocess.CalledProcessError as e:
        print(f"Error running nm on {so_file}: {e.stderr.strip()}")
    except FileNotFoundError:
        print("nm command not found. Please ensure it is installed and in your PATH.")
    except Exception as e:
        print(f"An unexpected error occurred: {str(e)}")
    # 返回提取的符号列表
    if not symbols:
        print(f"No symbols found in {so_file}.")
    else:
        print(f"Found {len(symbols)} symbols in {so_file}.")
    return symbols


def check_api_symbols(ld_map_file: str, so_file: str) -> bool:
    """
    map_file: The file containing the list of API symbols.
    so_file: The shared object file to check against the API symbols.
    """
    ld_symbols = get_ld_map_file_symbols(ld_map_file)
    so_symbols = get_so_file_symbols(so_file)

    result = True
    for symbol in ld_symbols:
        if symbol not in so_symbols:
            print(f"Missing API symbol: {symbol}")
            result = False
        # else:
        #     print(f"API symbol found: {symbol}")

    if result:
        for symbol in ld_symbols:
            print(f"LD MAP API symbol found: {symbol}")
        for symbol in so_symbols:
            print(f"SO API symbol found: {symbol}")

    return result


if __name__ == "__main__":
    # 从命令行参数获取链接脚本和共享库文件路径
    if len(sys.argv) != 3:
        print("Usage: python check_api.py <ld_map_file> <so_file>")
        sys.exit(1)
    ld_map_file = sys.argv[1]
    so_file = sys.argv[2]
    if not os.path.exists(ld_map_file):
        print(f"Link map file {ld_map_file} does not exist.")
        sys.exit(1)
    if not os.path.exists(so_file):
        print(f"Shared object file {so_file} does not exist.")
        sys.exit(1)
    # 检查API符号
    ret = check_api_symbols(ld_map_file, so_file)
    if ret:
        print("All API symbols are present in the shared library.")
    else:
        print("Some API symbols are missing in the shared library.")
    sys.exit(0 if ret else 1)
