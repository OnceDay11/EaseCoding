#!/bin/bash

# 开启调试
# set -x
# 关闭调试
# set +x

# 避免重复导入
if [ -n "$UTILS_SH" ]; then
    return
fi
UTILS_SH="UTILS_SH"

# 检查shell是否支持颜色输出
# check_support_color return 0 if support color, otherwise return 1
check_support_color() {
    if [[ -t 1 ]] && [[ -n "$TERM" ]] && [[ "$TERM" != "dumb" ]]; then
        # 终端支持颜色输出
        return 0
    else
        # 终端不支持颜色输出
        return 1
    fi
}

# Shell 日志函数
# log "file" "line" "INFO" "This is a info message"
log() {
    local timestamp=$(date +"%Y-%m-%d %H:%M:%S")
    local script_name=$(basename "$1")
    local line_number=$2
    local level=$3
    local message=$4
    # local pid=$$
    # local user=$USER

    local color_red=""
    local color_green=""
    local color_yellow=""
    local color_blue=""
    local color_purple=""
    local color_cyan=""
    local color_reset=""
    if check_support_color; then
        # 终端支持颜色输出
        color_red="\033[31m"    # \033[31m 红色
        color_green="\033[32m"  # \033[32m 绿色
        color_yellow="\033[33m" # \033[33m 黄色
        color_blue="\033[34m"   # \033[34m 蓝色
        color_purple="\033[35m" # \033[35m 紫色
        color_cyan="\033[36m"   # \033[36m 青色
        color_reset="\033[0m"   # \033[0m 重置
    fi

    local message_str=""
    # 默认日志级别为DEBUG
    case $level in
    INFO)
        message_str="${color_green}[INFO ]: $message${color_reset}"
        ;;
    WARN)
        message_str="${color_yellow}[WARN ]: $message${color_reset}"
        ;;
    ERROR)
        message_str="${color_red}[ERROR]: $message${color_reset}"
        ;;
    DEBUG | *)
        message_str="${color_blue}[DEBUG]: $message${color_reset}"
        ;;
    esac

    # 输出日志
    echo -e "${color_cyan}[$timestamp]${color_reset} ${color_purple}[$script_name:$line_number]${color_reset} $message_str"
}

# 封装四个日志级别函数接口
# BASH_SOURCE[1] 表示调用者的文件名
# BASH_LINENO 表示调用者的行号
INFO() {
    log ${BASH_SOURCE[1]} $BASH_LINENO "INFO" "$1"
}

WARN() {
    log ${BASH_SOURCE[1]} $BASH_LINENO "WARN" "$1"
}

ERROR() {
    log ${BASH_SOURCE[1]} $BASH_LINENO "ERROR" "$1"
}

DEBUG() {
    log ${BASH_SOURCE[1]} $BASH_LINENO "DEBUG" "$1"
}

# 示例用法
# INFO "This is a info message"
# WARN "This is a warn message"
# ERROR "This is a error message"
# DEBUG "This is a debug message"
