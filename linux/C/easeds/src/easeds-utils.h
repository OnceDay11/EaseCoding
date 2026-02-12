/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 *
 * @FilePath: /linux/C/easeds/src/easeds-utils.h
 * @Author: Once Day <once_day@qq.com>.
 * @Date: 2026-02-12 23:12
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  Easeds 工具类函数声明.
 *
 * @History:
 *  2026年2月12日, Once Day <once_day@qq.com>, 创建此文件.
 *
 */

#ifndef __EASEDS_UTILS_H__
#define __EASEDS_UTILS_H__

/* C标准库头文件 */
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EASEDS_MAX(a, b) \
    (const EASEDS_TYPE(a) _a = (a); const EASEDS_TYPE(b) _b = (b); _a > _b ? _a : _b)
#define EASEDS_MIN(a, b) \
    (const EASEDS_TYPE(a) _a = (a); const EASEDS_TYPE(b) _b = (b); _a < _b ? _a : _b)

#define EASEDS_STRING_BUFFER_LEN 256 /* easeds字符串化的缓冲区长度 */
#define EASEDS_THREAD_NAME_LEN   16  /* easeds线程名称长度(包括'\0') */

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
    __attribute__((format(printf, 3, 4)));

/* easeds打印输出模块封装函数, 总是返回实际写入的字符长度 */
int32_t easeds_vsnprintf(char *buffer, int32_t len, const char *fmt, va_list args);

/* easeds打印输出模块封装函数, 溢出自动截断, 总是返回实际写入的字符长度, 截断无警告日志. */
int32_t easeds_snprintf_unsafe(char *buffer, int32_t len, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));

/* 获取当前进程的名字 */
const char *easeds_get_current_program_name(void);

/* 获取当前线程的名字, 每个线程只有初始化时会更新一次, 需要在设置名字后才能调用 */
const char *easeds_get_current_thread_name(void);

/* 设置当前线程的名字 */
void easeds_set_current_thread_name(const char *name);

/* 获取当前线程的TID */
int32_t easeds_get_current_thread_id(void);

/* 获取当前进程的PID */
int32_t easeds_get_current_program_pid(void);

/* 获取当前线程的运行CPU ID */
int32_t easeds_get_current_cpu_id(void);

/* 定义 秒/毫秒/微秒 到 纳秒 的比例 */
#define EASEDS_NSEC_PER_SEC  1000000000LL
#define EASEDS_NSEC_PER_MSEC 1000000LL
#define EASEDS_NSEC_PER_USEC 1000LL
#define EASEDS_NSEC_INVALID  (-1LL) /* 无效的纳秒值 */

/* 记录unix时间, 准确性相对较低, 但适合用于确定时间点 */
int64_t easeds_get_unix_time_ns(void);

/* 转换 unix 时间戳为字符串格式: 2025-04-16 11:20:51.418 */
void easeds_get_unix_time_str(int64_t unix_time_ns, char *buffer, int32_t len);

/* 获取系统滴答计数, 用于绝对时间戳, 0通常是无效值 */
int64_t easeds_get_current_time_ns(void);
int64_t easeds_get_current_time_us(void);
int64_t easeds_get_current_time_ms(void);
int64_t easeds_get_current_time_sec(void);

/* 获取相对时间戳(起始不固定, 单调递增), 返回浮点数, 单位 sec */
double easeds_get_relative_time(void);

/* 获取一个随机数, uint32_t 类型 */
uint32_t easeds_get_random_uint32(void);

/**
 * 获取给定长度范围内的随机字符串.
 * @attention 内部函数(参数始终有效), 非线程安全.
 * @param buffer 缓冲区
 * @param min 最小长度
 * @param max 最大长度, 缓冲区至少为 max + 1 字节
 * @return 随机字符串
 */
const char *easeds_get_random_string(char *buffer, int32_t min, int32_t max);

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
const char *easeds_get_random_path_name(char *buffer, int32_t min, int32_t max);

#ifdef __cplusplus
}
#endif

#endif /* __EASEDS_UTILS_H__ */
