/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 *
 * @FilePath: /linux/C/easeds/src/easeds-unittest.h
 * @Author: Once Day <once_day@qq.com>.
 * @Date: 2026-02-12 23:30
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  Easeds 单元测试相关的头文件, 定义了单元测试的相关宏和函数声明.
 *
 * @History:
 *  2026年2月12日, Once Day <once_day@qq.com>, 创建此文件.
 *
 */

#ifndef __EASEDS_UNITTEST_H__
#define __EASEDS_UNITTEST_H__

/* C标准库头文件 */
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 第三方库文件 */
/* CMocka头文件里需要定义调用约定为C */
#include <cmocka.h>

#ifdef __cplusplus
}
#endif

/* 项目内部头文件 */
#include "easeds-environment.h"
#include "easeds-log.h"
#include "easeds-queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 定义单元测试信息打印函数, 部分场景下输出到日志或者通过IPC输出到终端 */
#ifndef easeds_printf
#define easeds_printf(...) printf(__VA_ARGS__)
#endif

/* 单元测试 Mock 函数/变量的声明和定义 */
#define MOCK_REAL(func)     __real_##func                // 原始函数
#define MOCK_WRAP(func)     __wrap_##func                // 包装函数
#define MOCK_STATE(func)    __mock_enabled_##func        // 是否启用包装函数
#define MOCK_DEFINE(func)   volatile bool MOCK_STATE(func) = false
#define MOCK_ENABLED(func)  MOCK_STATE(func) = true;     // 启用 Mock 功能
#define MOCK_DISABLED(func) MOCK_STATE(func) = false;    // 禁用 Mock 功能

/* 声明 Mock 函数, 包括包装函数和原始函数, volatile 防止编译器优化 */
#define MOCK_DECLARE(func)                    \
    extern EASEDS_TYPE(func) MOCK_WRAP(func); \
    extern EASEDS_TYPE(func) MOCK_REAL(func); \
    extern volatile bool MOCK_STATE(func)

/* 单元测试 Mock 辅助使用宏, 自动生成包装函数名称 */
#define mock_expect_string(function, parameter, string) \
    expect_string(__wrap_##function, parameter, string)
#define mock_expect_memory(function, parameter, memory, size) \
    expect_memory(__wrap_##function, parameter, memory, size)
#define mock_will_return(function, value) will_return(__wrap_##function, value)

/* 打印 Mock 函数信息, 专用日志函数, 清除 errno 避免干扰日志信息 */
#define MOCK_INFO(func, ...)                                        \
    do {                                                            \
        if (MOCK_STATE(func)) {                                     \
            EASEDS_INFO("[mock][" #func "][enable]: " __VA_ARGS__); \
        } else {                                                    \
            PFL_DEBUG("[mock][" #func "][disable]: " __VA_ARGS__);  \
        }                                                           \
        errno = 0;                                                  \
    } while (0)

/* Sizeof Dump辅助函数: 使用宏很难对齐打印 */
void easeds_unittest_sizeof_dump(const char *name, size_t size);

/* Sizeof测试用例, 用于打印感兴趣数据结构的大小, 辅助宏 */
#define EASEDS_SIZEOF_DUMP(expr) easeds_unittest_sizeof_dump(#expr, expr);

/* 单元测试执行状态 */
#define EASEDS_UNITTEST_CONTINUE 0
#define EASEDS_UNITTEST_EXIT     (-1)
#define EASEDS_UNITTEST_ERROR    (-2)

/* 定义单元测试节点 */
struct easeds_unittest_node {
    uint32_t                 test_num;       /* 测试用例数量 */
    uint32_t                 pad;            /* 填充 */
    const char              *name;           /* 测试用例名称 */
    const struct CMUnitTest *tests;          /* 测试用例 */
    CMFixtureFunction        setup;          /* 测试用例初始化函数 */
    CMFixtureFunction        teardown;       /* 测试用例清理函数 */
    STAILQ_ENTRY(easeds_unittest_node) next; /* 单元测试节点链表 */
};

/* 注册单元测试到链表中, 最终通过单元测试主入口函数进行执行 */
extern void easeds_unittest_register(struct easeds_unittest_node *test_node);

/* 避免VScode解析异常 */
#ifdef VSCODE_INTELLISENSE
#define constructor(...) constructor
#define destructor(...)  destructor
#endif

/* 定义单元测试构造函数 */
#define EASEDS_UNITTEST_INIT(func) static void __attribute__((constructor(6666), used)) func(void)

/* 测试节点包装宏 */
#define EASEDS_UNITTEST_REGISTER(tests)                     \
    extern const struct CMUnitTest tests[128];              \
    EASEDS_UNITTEST_INIT(tests##_init)                      \
    {                                                       \
        static struct easeds_unittest_node tests##_node = { \
            0, 0, #tests, (tests), NULL, NULL, {NULL}};     \
        easeds_unittest_register(&(tests##_node));          \
    }                                                       \
    const struct CMUnitTest tests[128] =

/* 简化测试节点注册宏 */
#define EASEDS_UNITTEST_REGISTER_EX(tests, setup, teardown)  \
    extern const struct CMUnitTest tests[128];               \
    EASEDS_UNITTEST_INIT(tests##_init)                       \
    {                                                        \
        static struct easeds_unittest_node tests##_node = {  \
            0, 0, #tests, (tests), setup, teardown, {NULL}}; \
        easeds_unittest_register(&(tests##_node));           \
    }                                                        \
    const struct CMUnitTest tests[128] =

/* 用于标识最后一个测试节点 */
#define easeds_unit_test_end {NULL, NULL, NULL, NULL, NULL}

/* 单元测试主入口函数 */
extern int32_t easeds_unittest_main(int32_t argc, char **argv, int32_t (*global_init)(void));

/* 单元测试输出流函数: 输出到 /dev/null 或者 stderr(debug使能) */
extern FILE *easeds_unittest_output_stream(void);

/* 获取无效输出流 */
extern FILE *easeds_unittest_invalid_stream(void);

/* 重定义单元测试信息打印函数, 未开启debug时, 无需打印到控制台 */
#undef easeds_printf
#define easeds_printf(...) fprintf(easeds_unittest_output_stream(), __VA_ARGS__)

/**
 * 单元测试主要测试目标函数在给定输入参数时的输出结果.
 * 一个函数通常有如下几种输入输出:
 *  (1) 显式输入参数: 函数的输入参数, 可能是指针或者值.
 *  (2) 隐式输入参数: 通过内部函数调用获取的输入参数.
 *   (2-1) 静态隐式输入参数: 内部函数输入和输出映射固定, 不会变化, 例如工具和辅助功能函数.
 *   (2-2) 动态隐式输入参数: 内部函数输入和输出映射不固定, 可能变化, 例如系统函数调用.
 *  (3) 显式输出结果: 函数的输出结果, 可能是指针或者值.
 *  (4) 隐式输出结果: 通过内部函数调用输出的结果.
 *   (4-1) 静态隐式输出结果: 内部函数输入和输出映射固定, 不会变化, 例如工具和辅助功能函数.
 *   (4-2) 动态隐式输出结果: 内部函数输入和输出映射不固定, 可能变化, 例如系统函数调用.
 * 通常一个主要逻辑函数需要测试下面几种情况:
 *  (1) normal-功能测试, 假设显式和隐式输入都正常, 只验证显式输出结果.
 *  (2) boundary-边界测试, 假设显式输入参数位于边界值附近, 只验证显式输出结果.
 *  (3) error-失效测试, 假设显式输入参数是异常值, 验证错误处理情况, 包括显式和隐式输出结果.
 *  (4) mock-模拟测试, 假设隐式输入参数和隐式输出结果是异常值, 验证显式输出参数.
 *  (5) perf-性能测试, 假设显式和隐式输入参数是正常值, 验证显式输出参数和性能情况.
 *  (6) parallel-并发测试, 假设显式和隐式输入参数是正常值, 验证显式和隐式输出参数和性能情况.
 * 函数根据使用范围, 分为内部函数和外部函数, 这并不等价于函数链接作用域.
 *  (1) 内部函数: 仅在组件内部使用, 不对外部组件提供服务, 例如辅助函数和内部工具函数.
 *  (2) 外部函数: 对外部组件提供服务, 例如对外接口函数和公共函数.
 * 内部功能函数至少要求功能测试, 外部函数至少要求功能测试, 边界测试和失效测试.
 * 核心函数(包括工具类)额外要求(系统函数)模拟测试.
 * 高频调用或者热点路径的函数额外要求性能测试.
 * 支持多线程的函数额外要求并发测试.
 */

#ifdef __cplusplus
}
#endif

#endif /* __EASEDS_UNITTEST_H__ */
