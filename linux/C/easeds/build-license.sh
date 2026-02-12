#!/bin/bash

# 开启调试
# set -x
# 关闭调试
# set +x

# 当前目录路径
SOURCE_DIR=${SOURCE_DIR:-$(pwd)}

# 导入基本 shell工具函数
source $SOURCE_DIR/utils.sh

# 导入版本信息
source $SOURCE_DIR/version.sh
INFO "EASEDS Version: $EASEDS_VERSION"
export TARGET_VERSION=${EASEDS_VERSION:-0.0.0} # 默认版本号

# 默认为开发版本
export TARGET_TYPE=${TARGET_TYPE:-develop}

# Verbose 模式
export V=${V:-1}

# 调用 license.py 脚本
$SOURCE_DIR/license.py

# 检查脚本执行结果
return_code=$?
if [ $return_code -ne 0 ]; then
    ERROR "license.py script failed with return code: $return_code"
    exit $return_code
fi
