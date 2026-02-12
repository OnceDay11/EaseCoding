/*
 * SPDX-License-Identifier: Ruijie Network CSBU Private
 *
 * Copyright (c) 2025 Ruijie Networks (CSBU) Co., Ltd., All rights reserved.
 *
 * @FilePath: /linux/C/easeds/src/easeds-log.c
 * @Author: Once Day <once_day@qq.com> <chenguang1@ruijie.com.cn>.
 * @Date: 2025-04-01 11:04
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  Easeds 日志输出相关函数和宏定义, 用于统一输出日志信息.
 *
 * @History:
 *  2025年4月1日, Once Day <once_day@qq.com>, 创建.
 *
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "easeds-log.h"

/* C标准库头文件 */
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

/* 内部工具头文件 */
#include "easeds-utils.h"

// 全局 Easeds 日志配置实例(支持缺省日志信息输出)
static struct easeds_log_settings g_easeds_log_global_config = {
    .log_callback = NULL,             /* 默认日志回调函数 */
    .always_print = EASEDS_LOG_EMERG, /* 默认输出到stderr的日志级别 */
    .debug_on     = false,            /* 默认不开启debug模式 */
    .verbose_on   = false,            /* 默认不开启verbose模式 */
    .measure_on   = false,            /* 默认不开启性能测量 */
    .stderr_on    = false,            /* 默认不开启stderr输出 */
};

// 获取全局 Easeds 日志配置实例
struct easeds_log_settings *easeds_log_get_global_setting(void)
{
    // 返回全局 Easeds 日志配置实例指针
    return &g_easeds_log_global_config;
}

#define log_settings (g_easeds_log_global_config)

/* 设置当前 Easeds Log Debug On 状态, 非线程安全 */
void easeds_log_debug_state_switch(bool debug)
{
    log_settings.debug_on = debug;
}

/* 设置当前 Easeds Log Verbose On 状态, 非线程安全 */
void easeds_log_verbose_state_switch(bool verbose)
{
    log_settings.verbose_on = verbose;
}

/* 设置当前 Easeds Log Measure On 状态, 非线程安全 */
void easeds_log_measure_state_switch(bool measure)
{
    log_settings.measure_on = measure;
}

/* 获取当前 Easeds Log Measure On 值 */
bool easeds_log_measure_state(void)
{
    return log_settings.measure_on;
}

/* 获取当前的 Easeds Log Verbose On 值 */
bool easeds_log_verbose_state(void)
{
    return log_settings.verbose_on;
}

/**
 * 获取当前 easeds log 的debug on值, 有多种方法判断, 且优先级不一样.
 * 目前认为环境变量的优先级高于实例初始化参数.
 *
 * @param debug: 是否开启debug模式
 * @return: 返回当前的debug模式值
 */
static bool easeds_log_debug_on_init(bool debug)
{
    bool debug_on;

    /* 获取环境变量EASEDS_DEBUG_ON的值 */
    debug_on = debug || (getenv("EASEDS_DEBUG_ON") != NULL);

    /* 设置 Easeds Log Debug On 状态 */
    easeds_log_debug_state_switch(debug_on);

    PFL_DEBUG_COND(
        debug_on, "[log debug]: The easeds log debug state: [ %s ].", debug_on ? "true" : "false");
    return debug_on;
}

/* 返回当前 Easeds Debug 日志使能情况, 非线程安全 */
bool easeds_log_debug_state(void)
{
    static bool g_easeds_log_first_init = false;
    bool        debug_on;

    debug_on = log_settings.debug_on;

    /* 如果没有初始化过, 则进行初始化 */
    if (unlikely(!g_easeds_log_first_init)) {
        g_easeds_log_first_init = true;
        easeds_wmb();
        debug_on = easeds_log_debug_on_init(debug_on);
    }

    return debug_on;
}

/* Easeds 设置标准错误STDERR输出日志级别 */
void easeds_set_log_always_print_level(int32_t level)
{
    /* 异常级别日志, 默认为 EASEDS_LOG_EMERG */
    if (level < EASEDS_LOG_EMERG || level > EASEDS_LOG_DEBUG) {
        EASEDS_ERR("Invalid log level %d, set to default %d.", level, EASEDS_LOG_EMERG);
        log_settings.always_print = EASEDS_LOG_EMERG;
    }

    /* 设置输出到stderr的日志级别, 低于该级别的日志会额外输出到stderr */
    log_settings.always_print = level;
}

/* 判断 Easeds 日志是否可以被输出到标准错误 STDERR */
static inline bool easeds_log_is_always_print(int32_t level)
{
    /* 低于指定级别的日志, 则额外输出一份到STDERR */
    return level <= log_settings.always_print;
}

/* Easeds 默认syslog日志输出回调函数 */
static int32_t easeds_log_syslog_cb(int32_t level, const char *prefix_str, const char *msg)
{
    syslog(level, "%s%s\n", prefix_str, msg);
    return 0;
}

/* Easeds 默认STDERR日志输出回调函数 */
static int32_t easeds_log_stderr_cb(int32_t level, const char *prefix_str, const char *msg)
{
    easeds_unused(level);
    if (fprintf(stderr, "%s%s\n", prefix_str, msg) < 0) {
        return -1;
    }
    return 0;
}

/* 设置 easeds 日志终端调试模式, 固定输出日志到 stderr 通道 */
void easeds_log_debug_mode_enable(bool enabled)
{
    log_settings.stderr_on    = enabled;
    log_settings.log_callback = enabled ? easeds_log_stderr_cb : NULL;
}

/* 获取 easeds 日志终端调试模式, 是否设置输出日志到 stderr 通道 */
bool easeds_log_is_enabled_stderr_output(void)
{
    return log_settings.stderr_on;
}

/* note: 单元测试时, 日志默认输出到stderr */
#ifdef _EASEDS_UNITTEST
#define EASEDS_LOG_DEFAULT_CALLBACK easeds_log_stderr_cb
#else
#define EASEDS_LOG_DEFAULT_CALLBACK easeds_log_syslog_cb
#endif

/* 获取 Easeds 日志输出回调函数, 总是返回一个有效的回调函数 */
static easeds_log_cb_t easeds_get_log_callback(void)
{
    easeds_log_cb_t log_cb = log_settings.log_callback;

    /* 如果没有设置日志输出回调函数, 则使用默认的syslog输出 */
    if (unlikely(log_cb == NULL)) {
        /* 显式强调回调函数使用, 并不是无效代码 */
        easeds_unused(easeds_log_stderr_cb);
        easeds_unused(easeds_log_syslog_cb);
        log_cb = EASEDS_LOG_DEFAULT_CALLBACK;
    }

    return log_cb;
}

/* 设置 Easeds 日志输出回调函数, 不具备线程安全性 */
void easeds_set_log_callback(easeds_log_cb_t log_cb)
{
    /* 如果没有设置日志输出回调函数, 则使用默认的syslog输出 */
    log_settings.log_callback = log_cb ? log_cb : EASEDS_LOG_DEFAULT_CALLBACK;
}

/* 定义 Easeds 日志信息结构体 */
struct easeds_log_info {
    bool        print_errno; /* 是否打印errno */
    bool        is_text;     /* 是否是文本日志 */
    uint8_t     pad[2];      /* 填充, 4字节对齐 */
    int32_t     errno_num;   /* 错误码 */
    int32_t     level;       /* 日志级别 */
    int32_t     line;        /* 日志行号 */
    const char *file;        /* 日志文件名 */
    const char *func;        /* 日志函数名 */
    va_list     va_args;     /* 可变参数列表 */
};

/* Easeds 日志信息结构体初始化 */
static void easeds_log_info_init(struct easeds_log_info *log_info, int32_t level, const char *file,
    int32_t line, const char *func)
{
    /* 设置日志信息 */
    log_info->print_errno = false;
    log_info->is_text     = false;
    log_info->errno_num   = errno;
    log_info->level       = level;
    log_info->line        = line;
    log_info->file        = file;
    log_info->func        = func;
    memset(log_info->pad, 0x0, sizeof(log_info->pad));
    memset(&log_info->va_args, 0x0, sizeof(log_info->va_args));
}

/* 定义 Easeds 日志输出数据结构体 */
struct easeds_log_data {
    struct easeds_log_info *log_info;       /* 日志信息 */
    int32_t                 prefix_str_len; /* 日志前缀长度 */
    int32_t                 msg_len;        /* 日志内容长度 */
    const char             *prefix_str;     /* 日志前缀字符串 */
    const char             *msg;            /* 日志内容 */
};

/* Easeds 日志输出数据结构体初始化 */
static void easeds_log_data_init(struct easeds_log_data *log_data, struct easeds_log_info *log_info,
    const char *prefix_str, int32_t prefix_str_len, const char *msg, int32_t msg_len)
{
    log_data->log_info       = log_info;
    log_data->prefix_str_len = prefix_str_len;
    log_data->msg_len        = msg_len;
    log_data->prefix_str     = prefix_str;
    log_data->msg            = msg;
}

/* 调用 Easeds 日志输出回调函数, 默认输出到 syslog, 低于指定级别的额外输出一份到STDERR */
static int32_t easeds_log_output(const struct easeds_log_data *log_data)
{
    easeds_log_cb_t log_cb = easeds_get_log_callback();
    easeds_assert(log_cb != NULL);

    int32_t level = log_data->log_info->level;
    if (level == EASEDS_LOG_MEASURE) {
        // 将 EASEDS_LOG_MEASURE 级别的日志转换为 EASEDS_LOG_INFO 级别
        level = EASEDS_LOG_INFO;
    } else if (level == EASEDS_LOG_DIAGNOSIS) {
        // 将 EASEDS_LOG_DIAGNOSIS 级别的日志转换为 EASEDS_LOG_DEBUG 级别
        level = EASEDS_LOG_DEBUG;
    } else if (level == EASEDS_LOG_CLI) {
        // 将 EASEDS_LOG_CLI 级别的日志转换为 EASEDS_LOG_INFO 级别
        level = EASEDS_LOG_INFO;
    }

    const int32_t ret = log_cb(level, log_data->prefix_str, log_data->msg);

    /* 如果是低于指定级别的日志, 则额外输出一份到STDERR */
    if (unlikely(easeds_log_is_always_print(level))) {
        easeds_log_stderr_cb(level, log_data->prefix_str, log_data->msg);
    }

    return ret;
}

/* 判断日志级别是否比 EASEDS_LOG_ERR 更严重(包括EASEDS_LOG_ERR自身) */
static inline bool easeds_log_level_is_serious(int32_t level)
{
    /* 判断日志级别是否大于等于 EASEDS_LOG_ERR */
    return level <= EASEDS_LOG_ERR;
}

/* 判断日志输出时是否需要跳过源码信息 */
static inline bool easeds_log_level_skip_source_info(int32_t level)
{
    // 这些日志都是固定的源码位置, 不需要打印
    // EASEDS_LOG_MEASURE/EASEDS_LOG_DIAGNOSIS/EASEDS_LOG_CLI
    return level >= EASEDS_LOG_MEASURE;
}

/* 从prioritynames中获取对应日志的错误名称 */
static inline const char *easeds_log_get_priority_name(int32_t level)
{
    switch (level) {
    case EASEDS_LOG_EMERG:
        return "panic";
    case EASEDS_LOG_ALERT:
        return "alert";
    case EASEDS_LOG_CRIT:
        return "critical";
    case EASEDS_LOG_ERR:
        return "error";
    case EASEDS_LOG_WARNING:
        return "warning";
    case EASEDS_LOG_NOTICE:
        return "notice";
    case EASEDS_LOG_INFO:
        return "info";
    case EASEDS_LOG_DEBUG:
        return "debug";
    case EASEDS_LOG_MEASURE:
        return "measure";
    case EASEDS_LOG_VERBOSE:
        return "verbose";
    case EASEDS_LOG_DIAGNOSIS:
        return "diagnosis";
    case EASEDS_LOG_CLI:
        return "easeds-cli";
    default:
        return "unknown";
    }
}

/* 剥离文件路径前面的目录, 只显示源文件名字 */
static inline const char *easeds_log_get_file_name(const char *file)
{
    /* 可以使用 __builtin_strrchr 代替 */
    const char *p = strrchr(file, '/');

    return p != NULL ? p + 1 : file;
}

#define EASEDS_LOG_PREFIX_BUFFER_SIZE 128  /* 日志前缀缓冲区大小 */
#define EASEDS_LOG_STACK_BUFFER_SIZE  4096 /* 日志栈内存缓冲区大小 */

/**
 * easeds内部统一使用的日志函数接口, 屏蔽底层实现.
 * 该日志接口假设输出的日志信息较少, 不超过1024字节, 使用栈内存优化效率.
 * 对于有特殊需求的较大日志输出(1kb以上), 需要使用 easeds_log_text 函数.
 */
static int32_t easeds_log_format(struct easeds_log_info *log_info, const char *format)
{
    char                   prefix[EASEDS_LOG_PREFIX_BUFFER_SIZE];
    char                   msg[EASEDS_LOG_STACK_BUFFER_SIZE];
    int32_t                ret;
    struct easeds_log_data log_data;

    /* 所有级别日志输出一样的信息, 但是单元测试时不需要携带过多前缀信息 */
    // const char *program_name = easeds_get_current_program_name();
    double      epoch_time   = easeds_get_relative_time();
    const char *thread_name  = easeds_get_current_thread_name();
    const char *priority_str = easeds_log_get_priority_name(log_info->level);
    log_info->file           = easeds_log_get_file_name(log_info->file);

    // Measure/Diagnosis 日志不打印代码信息, 其源码信息是固定的.
    if (easeds_log_level_skip_source_info(log_info->level)) {
        ret = easeds_snprintf(
            prefix, sizeof(prefix), "[%.6lfs][%s][%s]: ", epoch_time, thread_name, priority_str);
    } else {
        // 默认日志风格
        ret = easeds_snprintf(prefix, sizeof(prefix), "[%.6lfs][%s][%s][%s(%s:%d)]: ", epoch_time,
            thread_name, priority_str, log_info->file, log_info->func, log_info->line);
    }

    // 强调这些变量会被使用(也许现在还没有)
    easeds_unused(msg);

    if (unlikely(ret < 0)) {
        // 对于 error 级别以上的日志, 如果无法输出, 则直接中止程序, 从而保留问题现场
        if (easeds_log_level_is_serious(log_info->level)) {
            easeds_abort("[log deal]: snprintf failed");
        }
        return ret;
    }

    /* 设置日志前缀字符串地址和长度(不包括'\0') */
    const int32_t prefix_len = ret;

    /* 文本日志和Debug日志分别处理 */
    if (unlikely(log_info->is_text)) {
        /* 文本日志直接输出, 不需要格式化. 前缀字符串冒号后的 ' ' 修改为 '\n' 换行符 */
        prefix[prefix_len - 1] = '\n';
        easeds_log_data_init(&log_data, log_info, prefix, prefix_len, format, -1);
    } else {
        /* 转化格式化字符串 */
        ret = easeds_vsnprintf(msg, sizeof(msg), format, log_info->va_args);
        /* 如果格式化字符串失败, 直接退出 */
        if (unlikely(ret < 0)) {
            easeds_abort("[log deal]: vasprintf failed");
            return ret;
        }

        int32_t msg_len = ret;
        /* TODO: 需要考虑溢出的情况 */
        /* 如果需要打印errno, 则在日志内容后面追加errno信息 */
        if (log_info->print_errno) {
            /* 使用 GNU 版本的线程安全函数 char *strerror_r(int errnum, char *buf, size_t buflen) */
            char          error_buffer[EASEDS_STRING_BUFFER_LEN];
            const int32_t errno_num = log_info->errno_num;
            const char   *error_str = strerror_r(errno_num, error_buffer, sizeof(error_buffer));
            const int32_t max_len   = EASEDS_CAST(int32_t, sizeof(msg)) - ret;

            ret = easeds_snprintf(msg + ret, max_len, " [errno: %s(%d)]", error_str, errno_num);
            if (unlikely(ret < 0)) {
                easeds_abort("[log deal]: snprintf failed");
                return ret;
            }

            msg_len += ret;
        }

        /* 生成日志输出数据 */
        easeds_log_data_init(&log_data, log_info, prefix, prefix_len, msg, msg_len);
    }

    /* 输出日志内容 */
    ret = easeds_log_output(&log_data);
    if (unlikely(ret < 0)) {
        easeds_abort("[log deal]: log output failed");
        return ret;
    }

    return ret;
}

/* 日志函数定义 */
int32_t easeds_vlog(int32_t level, const char *file, const int32_t line, const char *func,
    const char *format, va_list ap)
{
    int32_t                ret;
    struct easeds_log_info log_info;

    /* 初始化日志信息, 记录 errno 值 */
    easeds_log_info_init(&log_info, level, file, line, func);

    /* 传递可变参数 */
    va_copy(log_info.va_args, ap);
    ret = easeds_log_format(&log_info, format);
    va_end(log_info.va_args);

    /* 恢复 errno 值 */
    errno = log_info.errno_num;

    return ret;
}

/* 日志函数定义 */
int32_t easeds_log(int32_t level, const char *__restrict file, const int32_t line,
    const char *__restrict func, const char *__restrict format, ...)
{
    int32_t                ret;
    struct easeds_log_info log_info;

    /* 初始化日志信息, 记录 errno 值 */
    easeds_log_info_init(&log_info, level, file, line, func);

    va_start(log_info.va_args, format);
    ret = easeds_log_format(&log_info, format);
    va_end(log_info.va_args);

    /* 恢复 errno 值 */
    errno = log_info.errno_num;

    return ret;
}

/* 日志函数-非可变参数版本定义, 用于提供跨编程语言的简易接口 */
int32_t easeds_log_callback(int32_t level, const char *__restrict file, const int32_t line,
    const char *__restrict func, const char *__restrict message)
{
    return easeds_log(level, file, line, func, "%s", message);
}

/* 日志函数 - 额外输出大量的文本信息, 用于 Dump 数据结构和辅助信息 */
int32_t easeds_log_text(int32_t level, const char *__restrict file, const int32_t line,
    const char *__restrict func, const char *__restrict message)
{
    int32_t                ret;
    struct easeds_log_info log_info;

    /* 初始化日志信息, 记录 errno 值 */
    easeds_log_info_init(&log_info, level, file, line, func);

    /* 设置日志信息为文本, 直接输出文本 */
    log_info.is_text = true;

    /* 没有格式化参数, 直接输出文本, 避免拷贝 */
    ret = easeds_log_format(&log_info, message);

    /* 恢复 errno 值 */
    errno = log_info.errno_num;

    return ret;
}

/* 系统错误日志函数定义 */
int32_t easeds_log_errno(int32_t level, const char *__restrict file, const int32_t line,
    const char *__restrict func, const char *__restrict format, ...)
{
    int32_t                ret;
    struct easeds_log_info log_info;

    /* 初始化日志信息, 记录 errno 值 */
    easeds_log_info_init(&log_info, level, file, line, func);

    /* 打印errno信息 */
    log_info.print_errno = true;

    va_start(log_info.va_args, format);
    ret = easeds_log_format(&log_info, format);
    va_end(log_info.va_args);

    /* 恢复 errno 值 */
    errno = log_info.errno_num;

    return ret;
}
