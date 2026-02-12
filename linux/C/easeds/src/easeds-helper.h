/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 *
 * @FilePath: /linux/C/easeds/src/easeds-helper.h
 * @Author: Once Day <once_day@qq.com>.
 * @Date: 2026-02-12 23:42
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  Easeds Helper, 定义了一些辅助宏和函数, 用于简化代码编写和提高代码可读性.
 *
 * @History:
 *  2026年2月12日, Once Day <once_day@qq.com>, 创建此文件.
 *
 */

#ifndef _EASEDS_HELPER_H_
#define _EASEDS_HELPER_H_

#include "easeds-environment.h"

// 项目内部头文件
#include "easeds-log.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 检查宏, 条件不成立时, 输出错误信息 */
#define EASEDS_CHECK(cond, ...)                                                  \
    do {                                                                         \
        if (unlikely(!(cond))) {                                                 \
            EASEDS_ERR("[easeds check failed]: cond [" #cond "]. " __VA_ARGS__); \
            easeds_abort("check failed");                                        \
        }                                                                        \
    } while (0)

/* 支持返回值 */
#define EASEDS_CHECK_RETURN(cond, return_value_when_false, ...)                  \
    do {                                                                         \
        if (unlikely(!(cond))) {                                                 \
            EASEDS_ERR("[easeds check failed]: cond [" #cond "]. " __VA_ARGS__); \
            return return_value_when_false;                                      \
        }                                                                        \
    } while (0)

// 辅助宏: 检查数据是否为 nullptr, 如果是则打印错误信息并返回错误码
#define EASEDS_CHECK_NULL_RETURN(pointer, return_value_when_false)               \
    do {                                                                         \
        if (unlikely((pointer) == NULL)) {                                       \
            EASEDS_ERR("[easeds check failed]: Arg [" #pointer "] is nullptr."); \
            return return_value_when_false;                                      \
        }                                                                        \
    } while (0)

/* 空指针检测直接返回 EASEDS_ERROR_NULL_POINTER */
#define EASEDS_CHECK_NULL_POINTER(pointer)                                      \
    do {                                                                        \
        if (unlikely((pointer) == NULL)) {                                      \
            EASEDS_ERR("[easeds check failed]: " #pointer " is NULL pointer."); \
            return EASEDS_ERROR_NULL_POINTER;                                   \
        }                                                                       \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif    // _EASEDS_HELPER_H_
