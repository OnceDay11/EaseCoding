/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 *
 * @FilePath: /linux/C/easeds/src/easeds-environment.h
 * @Author: Once Day <once_day@qq.com>.
 * @Date: 2026-02-12 22:14
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  Easeds Data Structure, 一个 C 语言实现的轻量级数据结构库.
 *  运行环境相关的头文件, 包括编译器和平台相关的环境适配和配置.
 *
 * @History:
 *  2026年2月12日, Once Day <once_day@qq.com>, 创建此文件.
 *
 */

#ifndef __EASEDS_ENVIRONMENT_H__
#define __EASEDS_ENVIRONMENT_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 全内存屏障（Full Memory Barrier）:
 * __sync_synchronize() 是 GCC 提供的一种内建函数（built-in function），
 * 用于在多线程环境中实现全内存屏障（Full Memory Barrier）。
 * 它确保在函数调用之前的所有内存访问操作（读和写）在函数调用之后的所有内存访问操作之前完成。
 * 换句话说，它阻止了处理器重新排序跨越该屏障的内存操作。
 */
#define easeds_rmb() __sync_synchronize()
#define easeds_wmb() __sync_synchronize()

/* 分支预测宏 */
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif
#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif

/**
 * 定义空类型, 其在复合数据结构体里面不占"空间", H[0]是gnu扩展语法, 这里不能使用.
 * Intentional empty struct definition.
 * 虽然C11标准允许空结构体,但在实践中,空结构体常常是由于漏写结构体成员列表而导致的错误。
 * 所以,为了帮助发现这类错误,-Werror=pedantic 选项会把空结构体当成一个错误来报告。
 * 这里使用预处理指令屏蔽报错.
 */
#pragma GCC diagnostic ignored "-Wpedantic" /* 忽略所有pedantic错误 */

typedef struct easeds_empty {
    /* 如果非要有一个元素, 那么会造成一些麻烦, 编译期会检查该结构体是否为零字节 */
} easeds_empty_t;

#pragma GCC diagnostic error "-Wpedantic" /* 恢复pedantic错误 */

/* 定义变量未使用, 避免编译报错, 声明函数参数可能使用属性 */
#define __easeds_unused  __attribute__((unused))
#define easeds_unused(x) (void)(x)

/* 定义变量需要为非常量, 用于显式表明需要函数需要为非常量 */
#define easeds_nonconst(x) (void)(x)

/* 运行环境层抽象定义 */
/* ## 运算符, 这是 GCC 扩展的语法. ISO C标准中应该使用 __VA_OPT__(,) 代替 */
#define __easeds_malloc malloc
#define __easeds_free   free

/* 类型转化和定义 */
#define EASEDS_TYPE(x)         __typeof__(x)
#define EASEDS_CAST(x, y)      ((__typeof__(x))(y))
/* 精度丢失的强制转化 */
#define EASEDS_CAST_LOST(x, y) ((__typeof__(x))(y))

/* 线程变量定义和声明 */
#define EASEDS_THREAD_LOCAL              __thread
#define EASEDS_THREAD_VAR(var)           g_per_thread_##var
#define EASEDS_THREAD_DEFINE(type, var)  EASEDS_THREAD_LOCAL EASEDS_TYPE(type) EASEDS_THREAD_VAR(var)
#define EASEDS_THREAD_DECLARE(type, var) EASEDS_THREAD_DEFINE(type, var)

/* 变量自动清理函数注册, GCC扩展支持能力, 自动生命周期管理 */
#define EASEDS_CLEAN_UP(clean_func) __attribute__((__cleanup__(clean_func)))

/* 变量命名辅助宏, 自动根据行号生成独一无二的名字 */
#define EASEDS_CONCAT_IMPL(x, y) x##y
#define EASEDS_CONCAT(x, y)      EASEDS_CONCAT_IMPL(x, y)
#define EASEDS_UNIQUE_NAME(base) EASEDS_CONCAT(base, __LINE__)

/**
 * 编译期检查静态数组是否大小合适。
 * 条件失败编译期会报错. 由于严格的C标准不允许0 size, 因此需要改成-1.
 */
#define EASEDS_ASSERT(cond) ((void)(sizeof(int[(-(!(cond) ? 1 : -1))])))

#ifdef __cplusplus
}
#endif

#include <assert.h>

/* 符合 C 标准的编译器断言检测函数 */
#define easeds_assert(cond)       assert(cond)
#define easeds_assert_perror(str) assert_perror(str)

/* ease主动abort宏, 在一些极端情况, 直接abort, 保留现场, 利于后续定位 */
#define easeds_abort(str) abort()

#endif /* __EASEDS_ENVIRONMENT_H__ */
