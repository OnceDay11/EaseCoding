/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 *
 * @FilePath: /linux/C/easeds/src/easeds-utils.c
 * @Author: Once Day <once_day@qq.com>.
 * @Date: 2026-02-12 23:14
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  2026年2月12日, Once Day <once_day@qq.com>, 创建此文件.
 *
 * @History:
 *  2026年2月12日, Once Day <once_day@qq.com>, 创建此文件.
 *
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

/* 系统库头文件 */
#include <sys/socket.h>
#include <sys/un.h>
#include <syscall.h>
#include <unistd.h>

/* C标准库头文件 */
#include <errno.h>
#include <features.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

/* 项目内头文件 */
#include "easeds-log.h"
#include "easeds-utils.h"

/**
 * easeds打印输出模块封装函数, 溢出自动截断, 总是返回实际写入的字符长度, 功能实现.
 */
static int32_t easeds_vsnprintf_impl(
    bool warn, char *buffer, int32_t len, const char *fmt, va_list args)
{
    int32_t str_len;

    /* 将数据写入到缓冲区中 */
    str_len = vsnprintf(buffer, EASEDS_CAST(uint64_t, len), fmt, args);

    /**
     * snprintf等函数返回的长度是格式化字符串解析后的长度, 不一定等于实际写入的长度, 会被截断.
     * 必须有等于号, 因为默认结尾会写入'\0', 但这个值并不会算到str_len里面.
     * 如果str_len大于等于len, 说明buffer的长度不够, 会被截断, 且最后字符一定是'\0'.
     */
    if (unlikely(str_len >= len)) {
        if (warn) {
            EASEDS_WARNING(
                "[vsnprintf]: Truncate strings, current %d, but should %d.", len - 1, str_len);
        }
        return EASEDS_CAST(int32_t, len - 1);
    }

    /* 返回实际写入的长度 */
    return str_len;
}

/**
 * easeds打印输出模块封装函数, 溢出自动截断, 总是返回实际写入的字符长度.
 * @attention 内部函数(参数始终有效), 线程安全, 可重入, 截断无警告日志.
 */
int32_t easeds_snprintf_unsafe(char *buffer, int32_t len, const char *fmt, ...)
{
    int32_t str_len;
    va_list args;

    /* 将数据写入到缓冲区中 */
    va_start(args, fmt);
    str_len = easeds_vsnprintf_impl(false, buffer, len, fmt, args);
    va_end(args);

    return str_len;
}

/**
 * easeds打印输出模块封装函数, 溢出自动截断, 总是返回实际写入的字符长度.
 * @attention 内部函数(参数始终有效), 线程安全, 可重入.
 * @param buffer: 输出缓冲区
 * @param len: 输出缓冲区长度
 * @param fmt: 格式化字符串
 * @param ...: 可变参数列表
 * @return: 返回实际写入的字符长度, 如果buffer长度不够, 则返回len-1.
 */
int32_t easeds_snprintf(char *buffer, int32_t len, const char *fmt, ...)
{
    int32_t str_len;
    va_list args;

    easeds_assert(buffer != NULL);
    easeds_assert(len > 0);

    /* 将数据写入到缓冲区中 */
    va_start(args, fmt);
    str_len = easeds_vsnprintf_impl(true, buffer, len, fmt, args);
    va_end(args);

    /* 返回实际写入的长度 */
    return str_len;
}

/* easeds打印输出模块封装函数, 总是返回实际写入的字符长度 */
int32_t easeds_vsnprintf(char *buffer, int32_t len, const char *fmt, va_list args)
{
    /* 返回实际写入的长度 */
    return easeds_vsnprintf_impl(true, buffer, len, fmt, args);
}

/* 获取当前进程的名字 */
const char *easeds_get_current_program_name(void)
{
    return program_invocation_short_name != NULL ? program_invocation_short_name : "(unknown)";
}

/* 获取当前进程的PID */
int32_t easeds_get_current_program_pid(void)
{
    static pid_t g_easeds_current_program_pid = -1;

    /* 第一次会尝试获取当前进程的PID */
    if (g_easeds_current_program_pid == -1) {
        g_easeds_current_program_pid = getpid();
    }

    return g_easeds_current_program_pid;
}

/* 线程名字存储存储变量 */
static char *easeds_get_current_thread_name_cache(void)
{
    /* 禁止非必要的全局构造函数, 使用函数静态变量代替, 仅在第一次执行时初始化 */
    static EASEDS_THREAD_DEFINE(char[EASEDS_THREAD_NAME_LEN], easeds_current_thread_name) = {'\0'};

    return EASEDS_THREAD_VAR(easeds_current_thread_name);
}

/* 获取当前线程的名字, 缓存在线程本地变量里面 */
const char *easeds_get_current_thread_name(void)
{
    char *thread_name = easeds_get_current_thread_name_cache();

    if (unlikely(thread_name[0] == '\0')) {
        /* 通过 pthread 库获取当前线程名字 */
        const int32_t ret = pthread_getname_np(pthread_self(), thread_name, EASEDS_THREAD_NAME_LEN);
        if (unlikely(ret != 0)) {
            EASEDS_PERR("[thread name]: Get thread name failed, using default name.");
            /* 获取线程TID, 用作线程名字 */
            const int32_t tid = easeds_get_current_thread_id();
            easeds_snprintf(thread_name, EASEDS_THREAD_NAME_LEN, "(%d)", tid);
        }
        thread_name[EASEDS_THREAD_NAME_LEN - 1] = '\0';
    }

    return thread_name;
}

/* 设置当前线程的名字 */
void easeds_set_current_thread_name(const char *name)
{
    char    real_name[EASEDS_THREAD_NAME_LEN];
    int32_t ret;

    strncpy(real_name, name, EASEDS_THREAD_NAME_LEN);
    real_name[EASEDS_THREAD_NAME_LEN - 1] = '\0';
    /* 超过15个字符, 输出警告信息 */
    if (strlen(name) >= EASEDS_THREAD_NAME_LEN) {
        EASEDS_WARNING(
            "[thread name]: Thread name is too long, truncate it: %s => %s.", name, real_name);
    }

    /* 设置当前线程的名字 */
    ret = pthread_setname_np(pthread_self(), real_name);
    if (unlikely(ret != 0)) {
        EASEDS_PERR("[thread name]: Set thread name failed: %s.", name);
        return;
    }

    /* 获取当前线程的缓存名字 */
    char *thread_name = easeds_get_current_thread_name_cache();

    /* 更新当前线程的名字 */
    easeds_snprintf(thread_name, EASEDS_THREAD_NAME_LEN, "%s", name);

    EASEDS_INFO("[thread name]: Set current thread name to %s.", thread_name);
    return;
}

/* 获取当前线程的TID */
int32_t easeds_get_current_thread_id(void)
{
    /* 禁止非必要的全局构造函数, 使用函数静态变量代替, 仅在第一次执行时初始化 */
    static EASEDS_THREAD_DEFINE(int32_t, easeds_current_thread_id) = 0;

    /* 第一次会尝试获取当前线程的TID */
    if (unlikely(EASEDS_THREAD_VAR(easeds_current_thread_id == 0))) {
        /* 获取当前线程的TID */
        EASEDS_THREAD_VAR(easeds_current_thread_id) = EASEDS_CAST(int32_t, syscall(SYS_gettid));
    }

    return EASEDS_THREAD_VAR(easeds_current_thread_id);
}

/* 获取当前线程的运行CPU ID */
int32_t easeds_get_current_cpu_id(void)
{
    /* 获取当前线程的CPU ID */
    return sched_getcpu();
}

/* 记录unix时间, 准确性相对较低, 但适合用于确定时间点 */
int64_t easeds_get_unix_time_ns(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * EASEDS_NSEC_PER_SEC + ts.tv_nsec;
}

/* 转换 unix 时间戳为字符串格式: 2025-04-16 11:20:51.418 */
void easeds_get_unix_time_str(int64_t unix_time_ns, char *buffer, int32_t len)
{
    easeds_assert(buffer != NULL);

    // 如果 unix_time_ns == 0, 则返回 (None)
    if (unlikely(unix_time_ns <= 0)) {
        easeds_snprintf(buffer, len, "(None)");
        return;
    }

    // 将unix时间戳转换为秒和毫秒
    int64_t seconds      = unix_time_ns / EASEDS_NSEC_PER_SEC;
    int64_t milliseconds = (unix_time_ns % EASEDS_NSEC_PER_SEC) / EASEDS_NSEC_PER_MSEC;

    // 格式化字符串: 2025-04-16 11:20:51.418
    char time_str[64];

    struct tm tm;
    if (localtime_r(&seconds, &tm) == NULL) {
        EASEDS_PERR("[unix time]: Convert unix time to local time failed.");
        return;
    }

    // 使用strftime格式化时间字符串
    if (strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm) == 0) {
        EASEDS_PERR("[unix time]: Format unix time to string failed.");
        return;
    }

    easeds_snprintf(buffer, len, "%s.%03ld", time_str, milliseconds);
}

// 获取系统滴答计数, 用于绝对时间戳, 0通常是无效值
int64_t easeds_get_current_time_ns(void)
{
    struct timespec ts;
    int64_t         absolute_ns;
    int32_t         ret;

    // clock_gettime()函数已使用VSDO机制, 无需走syscall系统调用路径.
    // 64位无符号数可以存储以ns为单位的时间最长584年, 以ms为单位的时间最长584e6年.
    // CLOCK_MONOTONIC_RAW: 从系统启动这一刻起开始计时, 不受系统时间被用户改变的影响.
    // CLOCK_MONOTONIC: 从系统启动这一刻起开始计时, 不受系统时间被用户改变的影响, 但速度可能改变.
    // CLOCK_MONOTONIC_RAW 不会收到NTP服务的影响, 但也会受到系统的时钟漂移影响.
    ret = clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    if (unlikely(ret != 0)) {
        return EASEDS_NSEC_INVALID;
    }

    // 计算绝对时间戳, 单位为ns, 最多584年
    absolute_ns = ts.tv_sec * EASEDS_NSEC_PER_SEC + ts.tv_nsec;

    return absolute_ns;
}

int64_t easeds_get_current_time_us(void)
{
    return easeds_get_current_time_ns() / EASEDS_NSEC_PER_USEC;
}

int64_t easeds_get_current_time_ms(void)
{
    return easeds_get_current_time_ns() / EASEDS_NSEC_PER_MSEC;
}

int64_t easeds_get_current_time_sec(void)
{
    return easeds_get_current_time_ns() / EASEDS_NSEC_PER_SEC;
}

/* 获取相对时间戳(起始不固定, 单调递增), 返回浮点数, 单位 sec */
double easeds_get_relative_time(void)
{
    /* 这里使用静态变量, 避免每次调用都申请内存 */
    static int64_t g_easeds_first_record_time = 0;

    if (unlikely(g_easeds_first_record_time == 0)) {
        /* 第一次调用, 记录当前时间 */
        g_easeds_first_record_time = easeds_get_current_time_ns();
    }

    /* 获取当前时间戳, 即使出现时序问题, 也只是一个小负值, 无需特别处理 */
    int64_t diff_time = easeds_get_current_time_ns() - g_easeds_first_record_time;

    return (double)diff_time / EASEDS_NSEC_PER_SEC;
}

/**
 * Lehmer线性同余法(Lehmer Linear Congruential Generator, LCG).
 * 常见有如下三种参数, 这里选择第二种(均匀性和独立性较好)
 * 1. m=2^32，a=1103515245，b=12345
 * 2. m=2^32，a=214013，b=2531011
 * 3. m=2^31-1，a=16807，b=0
 * @attention 内部函数(参数始终有效), 非线程安全.
 * @return: 返回随机浮点数, 范围[0, 1).
 */
static double easeds_rand_lehmer(void)
{
    static bool     initialized      = false;
    static uint32_t rand_lehmer_seed = 0;

    if (unlikely(!initialized)) {
        initialized      = true;
        rand_lehmer_seed = EASEDS_CAST_LOST(rand_lehmer_seed, easeds_get_current_time_ns());
    }

#define EASEDS_RAND_LEHMER_A 214013
#define EASEDS_RAND_LEHMER_B 2531011

    rand_lehmer_seed = rand_lehmer_seed * EASEDS_RAND_LEHMER_A + EASEDS_RAND_LEHMER_B;
    /* 需要确保区间为左闭右开, 这样离散概率可以均衡处理 */
    return (double)rand_lehmer_seed / (UINT32_MAX + 1ULL);
}

/**
 * 获取一个随机数, 用于测试.
 * @attention 内部函数(参数始终有效), 非线程安全.
 * @param min 最小值
 * @param max 最大值
 * @return 如果min >= max, 返回min, 否则返回 [min, max]之间的随机数.
 */
static int32_t easeds_rand_range(int32_t min, int32_t max)
{
    if (unlikely(min >= max)) {
        return min;
    }

    /* 对于 [10, 12], 存在 10, 11, 12 三个数字可供选择, 概率区间是 [0, 1), [1, 2), [2, 3) */
    const int32_t range = max - min + 1;
    /* 浮点数到整数的强制类型转换是向零取整, (int)3.7 => 3, (int)-3.7 => -3 */
    const int32_t ret = EASEDS_CAST(int32_t, easeds_rand_lehmer() * range) + min;

    return ret;
}

/**
 * 获取给定长度范围内的随机字符串.
 * @attention 内部函数(参数始终有效), 非线程安全.
 * @param buffer 缓冲区
 * @param min 最小长度
 * @param max 最大长度, 缓冲区至少为 max + 1 字节
 * @return 随机字符串
 */
const char *easeds_get_random_string(char *buffer, int32_t min, int32_t max)
{
    int32_t len = easeds_rand_range(min, max);
    char   *p   = buffer;

    /* 从 !(0x21) 到 ~(0x7e) */
    const int32_t range = 0x7e - 0x21;
    while (len-- > 0) {
        *p++ = EASEDS_CAST(char, 0x21 + easeds_rand_range(0, range));
    }

    *p = '\0';
    return buffer;
}

/**
 * 获取随机的路径名称.
 * 参照 RFC5234 中定义的 扩展巴科斯-诺尔范式(ABNF，Augmented Backus - Naur Form)
 * - 测量指标名称不能为 null 或空字符串.
 * - 第一个字符必须是字母.
 * - 后续字符可以是字母、数字、下划线(_)、点号(.)、连字符(-)和斜杠(/).
 * - 最大长度为 255 个字符.
 * 这里额外要求: 后续字符只能是字母、数字、下划线(_), 第一个字符不限制.
 * @attention 内部函数(参数始终有效), 非线程安全.
 * @param buffer 缓冲区
 * @param min 最小长度
 * @param max 最大长度, 缓冲区至少为 max + 1 字节
 * @return 随机字符串
 */
const char *easeds_get_random_path_name(char *buffer, int32_t min, int32_t max)
{
    int32_t len = easeds_rand_range(min, max);
    char   *p   = buffer;

    /* 0-9, a-z, A-Z, _ */
    const int32_t range1 = '9' - '0' + 1;
    const int32_t range2 = range1 + 'z' - 'a' + 1;
    const int32_t range3 = range2 + 'Z' - 'A' + 1;

    while (len-- > 0) {
        const int32_t rand = easeds_rand_range(0, range3);
        if (rand < range1) {
            *p++ = EASEDS_CAST(char, '0' + rand);
        } else if (rand < range2) {
            *p++ = EASEDS_CAST(char, 'a' + rand - range1);
        } else if (rand < range3) {
            *p++ = EASEDS_CAST(char, 'A' + rand - range2);
        } else {
            *p++ = EASEDS_CAST(char, '_');
        }
    }

    *p = '\0';
    return buffer;
}
