#!/bin/bash

# 开启调试
# set -x

# 当前目录路径
export SOURCE_DIR=${SOURCE_DIR:-$(pwd)}

# 导入基本shell工具函数
source $SOURCE_DIR/utils.sh

# 导入运行环境变量
source $SOURCE_DIR/run_environment.sh

INFO "Run environment setup completed."

# 额外运行参数, 来自脚本参数
EXECUTE_ARGS="$*"

# 指定单元测试二进制文件
UNIT_PROGRAMS="easeds-unittest"

# 运行所有单元测试
$UNIT_PROGRAMS $EXECUTE_ARGS
return_code=$?

# 重置环境
environment_reset

# 返回值
if [ $return_code -ne 0 ]; then
    ERROR "Run unittest failed"
    exit 1
fi

INFO "Run unittest success"
exit 0
