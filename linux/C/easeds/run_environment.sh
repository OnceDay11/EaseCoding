#!/bin/bash

# 开启shell调试
# set -x

# 当前目录路径, 默认是当前目录的父目录
export SOURCE_DIR=${SOURCE_DIR:-$(pwd)}

# 导入基本shell工具函数
source $SOURCE_DIR/utils.sh

# 设置关键环境变量, 需要按照build.sh的顺序设置
# 构建目录
export BUILD_SUFFIX=${BUILD_SUFFIX:-local}
export BUILD_TYPE=${BUILD_TYPE:-debug}
export TARGET_ARCH=${TARGET_ARCH:-$(uname -m)}
export BUILD_ID=${BUILD_ID:-${BUILD_TYPE}-${TARGET_ARCH}-${BUILD_SUFFIX}}
export BUILD_DIR=${BUILD_DIR:-${SOURCE_DIR}/build/${BUILD_ID}}
# 输出目录
export REPORT_DIR=${REPORT_DIR:-${SOURCE_DIR}/build/output/report}
export OUTPUT_DIR=${OUTPUT_DIR:-${SOURCE_DIR}/build/output/${BUILD_ID}}
export OUTPUT_BIN=${OUTPUT_DIR}/usr/bin
export OUTPUT_LIB=${OUTPUT_DIR}/usr/lib
export OUTPUT_ETC=${OUTPUT_DIR}/etc
export OUTPUT_TEST=${OUTPUT_DIR}/usr/test

INFO "BUILD_DIR: [$BUILD_DIR]"
INFO "REPORT_DIR: [$REPORT_DIR]"
INFO "OUTPUT_BIN: [$OUTPUT_BIN]"
INFO "OUTPUT_LIB: [$OUTPUT_LIB]"
INFO "OUTPUT_ETC: [$OUTPUT_ETC]"
INFO "OUTPUT_TEST: [$OUTPUT_TEST]"

# 构建Host运行环境
export EASEDS_ROOT_PATH=${OUTPUT_ETC}/easeds
INFO "EASEDS_ROOT_PATH: [$EASEDS_ROOT_PATH]"
export EASEDS_UNITTEST_PATH=${EASEDS_ROOT_PATH}/unittest/bin
INFO "EASEDS_UNITTEST_PATH: [$EASEDS_UNITTEST_PATH]"
export PATH=${OUTPUT_BIN}:${OUTPUT_TEST}:${EASEDS_UNITTEST_PATH}:$PATH
INFO "PATH: [$PATH]"
export LD_LIBRARY_PATH=${OUTPUT_LIB}:$LD_LIBRARY_PATH
INFO "LD_LIBRARY_PATH: [$LD_LIBRARY_PATH]"

# 是否设置 coredump 文件
CHANGE_COREDUMP_SETTING=${CHANGE_COREDUMP_SETTING:-ON}
# 只有位于 root 用户才能设置coredump文件
if [ $(id -u) -ne 0 ]; then
    WARN "Only root can set coredump file, please use sudo to run this script!"
    WARN "Skip support coredump file."
elif in_docker; then
    # docker 容器中(或者wsl), 无需设置coredump文件, cat /proc/1/comm != "systemd"
    WARN "Skip support coredump file in docker or wsl."
elif [ "${CHANGE_COREDUMP_SETTING}" = "ON" ]; then
    # 设置coredump文件的生成路径
    export COREDUMP_DIR=${SOURCE_DIR}/crash
    mkdir -p ${COREDUMP_DIR}
    chmod -R 777 ${COREDUMP_DIR}
    INFO "COREDUMP_DIR: [$COREDUMP_DIR]"

    # 保存旧的coredump core_pattern
    OLD_CORE_PATTERN=$(cat /proc/sys/kernel/core_pattern)
    INFO "Old coredump core_pattern: [$OLD_CORE_PATTERN]"

    # 配置coredump文件的生成路径
    # echo "|gzip -c > ${PWD}/crash/%e-%p-%t-%s-${TARGET_VERSION}.coredump.gz" | \
    #    sudo tee /proc/sys/kernel/core_pattern
    STR="${COREDUMP_DIR}/%e-%p-%t-%s.coredump"
    bash -c "echo '${STR}' > /proc/sys/kernel/core_pattern"
    WARN "Set coredump core_pattern: [$STR]"

    # 允许生成coredump文件
    ulimit -c unlimited
    WARN "Allow generate coredump file"
else
    INFO "Skip support coredump file."
fi

# 恢复环境, 执行完毕后需要调用
environment_reset() {
    INFO "environment reset!"

    if [ -z "${CHANGE_COREDUMP_SETTING}" ] || [ "${CHANGE_COREDUMP_SETTING}" != "ON" ]; then
        # 没有设置coredump文件, 直接返回
        return 0
    fi

    # 删除虚假根目录 EASEDS_FAKE_ROOT
    if [ -d "${EASEDS_FAKE_ROOT}" ]; then
        rm -rf ${EASEDS_FAKE_ROOT}
        INFO "Remove fake root path: [$EASEDS_FAKE_ROOT]"
    fi

    # 没有设置coredump文件, 直接返回
    if [ -z "${OLD_CORE_PATTERN}" ]; then
        return 0
    fi

    if [ $(id -u) -eq 0 ]; then
        # 禁止生成coredump文件
        ulimit -c 0
        WARN "Disable generate coredump file"
        # 恢复coredump core_pattern
        bash -c "echo '${OLD_CORE_PATTERN}' > /proc/sys/kernel/core_pattern"
        WARN "Restore coredump core_pattern: [$OLD_CORE_PATTERN]"
    fi

    return 0
}
