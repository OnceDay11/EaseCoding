/*
 * SPDX-License-Identifier: Ruijie Network CSBU Private
 *
 * Copyright (c) 2025 Ruijie Networks (CSBU) Co., Ltd., All rights reserved.
 *
 * @FilePath: /linux/C/easeds/src/easeds-log.h
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

#ifndef __EASEDS_LOG_H__
#define __EASEDS_LOG_H__

/* C 标准库头文件 */
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* 运行环境头文件 */
#include "easeds-environment.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 日志级别定义 */
#define EASEDS_LOG_EMERG     0  /* panic: system is unusable */
#define EASEDS_LOG_ALERT     1  /* alert: action must be taken immediately */
#define EASEDS_LOG_CRIT      2  /* critical: critical conditions */
#define EASEDS_LOG_ERR       3  /* error: error conditions */
#define EASEDS_LOG_WARNING   4  /* warning: warning conditions */
#define EASEDS_LOG_NOTICE    5  /* notice: normal but significant condition */
#define EASEDS_LOG_INFO      6  /* info: informational */
#define EASEDS_LOG_DEBUG     7  /* debug: debug-level messages */
#define EASEDS_LOG_VERBOSE   8  /* verbose: verbose-level messages */
#define EASEDS_LOG_MEASURE   9  /* measure: performance measurement logs */
#define EASEDS_LOG_DIAGNOSIS 10 /* diagnosis: diagnosis-level messages */
#define EASEDS_LOG_CLI       11 /* cli: cli-level messages */

/* 日志输出回调函数 */
typedef int32_t (*easeds_log_cb_t)(int32_t level, const char *prefix_str, const char *str);

/* Easeds 日志配置数据结构 */
struct easeds_log_settings {
    easeds_log_cb_t log_callback; /* 日志输出回调函数, 默认为 NULL */
    int32_t         always_print; /* 输出到stderr的日志级别, 默认为 EASEDS_LOG_EMERG */
    bool            debug_on;     /* 是否开启debug模式, 默认为 false */
    bool            verbose_on;   /* 是否开启verbose模式, 默认为 false */
    bool            measure_on;   /* 是否开启性能测量, 默认为 false */
    bool            stderr_on;    /* 是否输出到stderr, 默认为 false */
};

/* 获取全局 Easeds 日志配置实例 */
struct easeds_log_settings *easeds_log_get_global_setting(void);

/* 设置当前 Easeds Log Debug On 状态, 非线程安全 */
void easeds_log_debug_state_switch(bool debug);

/* 返回当前 Easeds Debug 日志使能情况, 非线程安全 */
bool easeds_log_debug_state(void);

/* 设置当前 Easeds Log Verbose On 状态, 非线程安全 */
void easeds_log_verbose_state_switch(bool verbose);

/* 返回当前 Easeds Log Verbose On 值 */
bool easeds_log_verbose_state(void);

/* 设置当前 Easeds Log Measure On 状态, 非线程安全 */
void easeds_log_measure_state_switch(bool measure);

/* 返回当前 Easeds Log Measure On 值 */
bool easeds_log_measure_state(void);

/* 设置 easeds 日志终端调试模式, 固定输出日志到 stderr 通道 */
void easeds_log_debug_mode_enable(bool enabled);

/* 获取 easeds 日志终端调试模式, 是否设置输出日志到 stderr 通道 */
bool easeds_log_is_enabled_stderr_output(void);

/* Easeds 设置标准错误STDERR输出日志级别 */
void easeds_set_log_always_print_level(int32_t level);

/* 设置 Easeds 日志输出回调函数, 不具备线程安全性 */
void easeds_set_log_callback(easeds_log_cb_t log_cb);

/* FIXME: 需要限制 Debug 和 Verbose 开关的运行时间, 不超过 60 min */

/**
 * Easeds version definition of easeds_log function
 *
 * @param level: log level.
 * @param file: file name.
 * @param line: line number.
 * @param func: function name.
 * @param format: log format string, must always exists and has '\n' character at end.
 * @param ...: log format string parameters.
 *
 * @return: The actual number of characters output.
 */
extern int32_t easeds_log(int32_t level, const char *__restrict file, int32_t line,
    const char *__restrict func, const char *__restrict format, ...)
    __attribute__((format(printf, 5, 6)));

/* 日志函数定义 - 传递可变参数 */
extern int32_t easeds_vlog(int32_t level, const char *file, const int32_t line, const char *func,
    const char *format, va_list ap);

/* ease日志输出统一封装宏, 输出统一的ease日志格式 */
#define EASEDS_LOG(level, ...) easeds_log(level, __FILE__, __LINE__, __func__, __VA_ARGS__)
/* Panic 系统不可用, 无法运行了(自我abort), 例如关键资源申请失败, 基础系统功能异常等 */
#define EASEDS_CRIT(...)       EASEDS_LOG(EASEDS_LOG_CRIT, __VA_ARGS__)
/* Error 必须是真正的错误, 需要进行处理, 例如系统、配置、运行、程序BUG等 */
#define EASEDS_ERR(...)        EASEDS_LOG(EASEDS_LOG_ERR, __VA_ARGS__)
/* Warning 有一些不符合预期, 但是不必马上处理, 后续优化即可, 例如触发限制、意外值、忽略值等 */
#define EASEDS_WARNING(...)    EASEDS_LOG(EASEDS_LOG_WARNING, __VA_ARGS__)
/* Info 没有发生错误, 但是需要记录一些重要信息, 例如配置变更、核心数据修改、敏感操作等 */
#define EASEDS_INFO(...)       EASEDS_LOG(EASEDS_LOG_INFO, __VA_ARGS__)
/* Notice 重要的通知信息, 例如系统状态变更、配置变更、核心数据变更等 */
#define EASEDS_NOTICE(...)     EASEDS_LOG(EASEDS_LOG_NOTICE, __VA_ARGS__)

/* 系统错误日志函数定义 */
extern int32_t easeds_log_errno(int32_t level, const char *__restrict file, int32_t line,
    const char *__restrict func, const char *__restrict format, ...)
    __attribute__((format(printf, 5, 6)));

/* 额外打印系统错误信息(std errno信息) */
#define EASEDS_PLOG(level, ...) easeds_log_errno(level, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define EASEDS_PERR(...)        EASEDS_PLOG(EASEDS_LOG_ERR, __VA_ARGS__)

/**
 * note: 这里并不存在单纯的 Debug 日志, 请使用示踪日志(Profiling Trace)代替.
 *
 * 为什么没有Debug日志?
 * (1) 理论上所有日志都需要输出, 虽然可以通过日志级别控制, 但往往应该输出的日志没有输出,
 * 不应该输出的日志在疯狂打印, 这就导致了 Debug 日志的存在意义不大.
 * (2) Debug 日志的开关尺度通常是全局的, 缺乏上下文概念, 无法精确控制, 造成输出信息杂乱不齐.
 * (3) Debug 日志通常用于调试, 测试结束后经常会被删除, 无法在生产环境使用, 造成资源浪费.
 * (4) Debug 并不被当成功能的一部分, 缺乏必要的测试和验证, 容易在生产环境触发故障.
 *
 * 所以, 使用示踪日志代替Debug日志, 通过 Trace 的方式来进行调试.
 * (1) Trace 日志是有上下文概念的, 可以在不同的上下文中进行 Trace, 可以精确控制粒度.
 * (2) Trace 日志可以在开发/测试/生产环境使用, 具备功能指标定义和测试验证, 安全可靠.
 * (3) Trace 日志属于可观测性的一部分, 非常容易集成到已有的可观测性功能里面.
 * (4) Trace 日志可以利用示踪日志底层能力, 不仅提高自身性能，也能降低通用日志系统的复杂性和压力.
 *
 * Debug 日志本质是一种全局上下文(可能细分到模块)里的示踪日志, 但终究并不是以上下文视角来处理.
 * 而示踪日志 PFL Trace 以上下文作为对象, 以程序执行流为线索, 输出各种调试日志, 信息非常集中.
 * 示踪日志与代码高度绑定, 是程序在可观测性方向的功能延伸, 而不是简单的日志输出.
 * 实践上, 示踪日志在描述程序正在做什么, 当前具备什么状态, 以及当前的上下文是什么.
 * 借助示踪日志信息, 我们甚至可以分析预期之外的程序行为, 而 Debug 日志只是描述我们已知的问题.
 *
 */
#define EASEDS_DEBUG(...) EASEDS_ASSERT(false)

/* PFL_TRACE 示踪日志简化版本, 使用 Debug Log 代替 */
#define PFL_DEBUG_COND(debug_on, ...)                  \
    do {                                               \
        if (unlikely(!!(debug_on))) {                  \
            EASEDS_LOG(EASEDS_LOG_DEBUG, __VA_ARGS__); \
        };                                             \
    } while (0)

/* 定义PFL的debug信息输出接口, 默认使用全局 Log Debug On 状态作为开关条件 */
#define PFL_DEBUG(...) PFL_DEBUG_COND(easeds_log_debug_state(), __VA_ARGS__)

/* 在 DEBUG ON 使能时, 执行指定的语句体 */
#define PFL_EXECUTE(...)                          \
    do {                                          \
        if (unlikely(easeds_log_debug_state())) { \
            __VA_ARGS__;                          \
        };                                        \
    } while (0)

/* 定义PFL的Verbose消息输出底层接口 */
#define PFL_VERBOSE_COND(verbose_on, ...)                \
    do {                                                 \
        if (unlikely(!!(verbose_on))) {                  \
            EASEDS_LOG(EASEDS_LOG_VERBOSE, __VA_ARGS__); \
        };                                               \
    } while (0)

/* 定义PFL的 Verbose 信息输出接口 */
#define PFL_VERBOSE(...) PFL_VERBOSE_COND(easeds_log_verbose_state(), __VA_ARGS__)

/* 在 Verbose On 使能的情况下, 执行特定的语句体 */
#define PFL_EXECUTE_VERBOSE(...)                    \
    do {                                            \
        if (unlikely(easeds_log_verbose_state())) { \
            __VA_ARGS__;                            \
        };                                          \
    } while (0)

/* 性能测量相关的日志输出 */
#define MEASURE_OUT(...) EASEDS_LOG(EASEDS_LOG_MEASURE, __VA_ARGS__)

/* 定义 Measure 信息条件输出接口 */
#define MEASURE_COND(measure_on, ...)   \
    do {                                \
        if (unlikely(!!(measure_on))) { \
            MEASURE_OUT(__VA_ARGS__);   \
        };                              \
    } while (0)

/* 定义 Measure 信息输出接口 */
#define MEASURE(...) MEASURE_COND(easeds_log_measure_state(), __VA_ARGS__)

/* 在 Measure On 使能的情况下, 执行特定的语句体 */
#define EXECUTE_MEASURE(...)                        \
    do {                                            \
        if (unlikely(easeds_log_measure_state())) { \
            __VA_ARGS__;                            \
        };                                          \
    } while (0)

/* 诊断组件相关的日志输出(诊断需要重载日志输出格式, 且不受日志等级开关的影响) */
#define DIAGNOSIS(...) EASEDS_LOG(EASEDS_LOG_DIAGNOSIS, __VA_ARGS__)

/* Ease标准输出打印函数 */
#define easeds_printf(...) fprintf(stdout, __VA_ARGS__)

/* 日志函数 - 非可变参数版本定义, 用于提供跨编程语言的简易接口 */
int32_t easeds_log_callback(int32_t level, const char *__restrict file, const int32_t line,
    const char *__restrict func, const char *__restrict message);

/* 日志函数 - 额外输出大量的文本信息, 用于 Dump 数据结构和辅助信息 */
int32_t easeds_log_text(int32_t level, const char *__restrict file, const int32_t line,
    const char *__restrict func, const char *__restrict message);

/* 文本日志输出辅助宏 */
#define EASEDS_LOG_TEXT(msg) easeds_log_text(EASEDS_LOG_INFO, __FILE__, __LINE__, __func__, (msg))
#define EASEDS_LOG_ERROR_TEXT(msg) \
    easeds_log_text(EASEDS_LOG_ERR, __FILE__, __LINE__, __func__, (msg))

#define PFL_DEBUG_TEXT(msg)                                                         \
    do {                                                                            \
        if (unlikely(easeds_log_debug_state())) {                                   \
            easeds_log_text(EASEDS_LOG_DEBUG, __FILE__, __LINE__, __func__, (msg)); \
        };                                                                          \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif    // __EASEDS_LOG_H__
