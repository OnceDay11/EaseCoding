#!/bin/bash

# 开启调试
# set -x

# 当前目录路径
export SOURCE_DIR=${SOURCE_DIR:-$(pwd)}

# 导入基本shell工具函数
source $SOURCE_DIR/utils.sh

# 定义参数
# 输出详细信息, -j 表示并行检查, 暂时不支持
if [ "$1" == "-v" ]; then
    CPP_ARGS="--verbose"
    shift
fi
# 支持并行检测
CPP_ARGS="$CPP_ARGS -j $(nproc)"
# 除了 missingInclude 其他都是默认开启的, 不扫描 unusedFunction, 库组件无法包含全部功能
CPP_ARGS="$CPP_ARGS --enable=style,performance,portability,information"
CPP_ARGS="$CPP_ARGS --disable=missingInclude"
# 忽略不确定的检查
CPP_ARGS="$CPP_ARGS --inconclusive"
# 强制检查所有配置(预定义宏)情况 => 会导致检查时间过长
CPP_ARGS="$CPP_ARGS --force"
# 忽略内联的警告
CPP_ARGS="$CPP_ARGS --inline-suppr"
# 运行库
CPP_ARGS="$CPP_ARGS --library=posix"
# 指定包含目录, 不要包含系统目录
CPP_ARGS="$CPP_ARGS -I $SOURCE_DIR"
# 排除需要检查预定义宏的头文件目录, 为了避免检查太多的条件宏, 运行层无需检查
CPP_ARGS="$CPP_ARGS --config-exclude=$SOURCE_DIR/abstraction"

# 指定C和C++的标准
CPP_ARGS="$CPP_ARGS --std=c++11 --std=c11 --fsigned-char"
# 发现错误时 exit-code = 1
CPP_ARGS="$CPP_ARGS --error-exitcode=1"

# 输出参数
INFO "CPP_ARGS: [$CPP_ARGS]"

# 指定检查目录
CHECK_DIR="$SOURCE_DIR/src $SOURCE_DIR/util $SOURCE_DIR/mini"
INFO "CHECK_DIR: [$CHECK_DIR]"

# 对代码进行静态检查, 使用tee将stderr输出镜像到本地文件里
LOCAL_FILE="$SOURCE_DIR/build/cppcheck.log"
# 如果路径不存在则创建
mkdir -p $(dirname $LOCAL_FILE)

# 执行检查
cppcheck $CPP_ARGS $CHECK_DIR 2>$LOCAL_FILE
if [ $? -ne 0 ]; then
    ERROR "Cppcheck failed"
    # 输出总结信息
    cat $LOCAL_FILE
    exit 1
fi

INFO "Cppcheck success"
exit 0
