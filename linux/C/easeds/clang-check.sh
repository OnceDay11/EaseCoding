#!/bin/bash

# 开启调试
# set -x

# 当前目录路径
export SOURCE_DIR=${SOURCE_DIR:-$(pwd)}

# 导入基本ANMK shell工具函数
source $SOURCE_DIR/utils.sh

# 定义参数
CHECK_ARGS="--status-bugs"

# 指定输出目录
OUTPUT_DIR="$SOURCE_DIR/clang-check"
rm -rf $OUTPUT_DIR
# 创建输出目录
mkdir -p $OUTPUT_DIR

INFO "OUTPUT_DIR: [$OUTPUT_DIR]"

# 运行检查命令
scan-build -o $OUTPUT_DIR $CHECK_ARGS ./build.sh
if [ $? -ne 0 ]; then
    ERROR "clang-check failed"
    exit 1
fi

# 未检测到问题
INFO "clang-check success"
exit 0

# TODO: clang-tidy的使用
