/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 *
 * @FilePath: /linux/C/easeds/src/easeds-public.h
 * @Author: Once Day <once_day@qq.com>.
 * @Date: 2026-02-12 22:12
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  Ease Data Structure, 一个 C 语言实现的轻量级数据结构库.
 *  公共头文件, 定义了数据结构库的公共接口和类型定义.
 *
 * @History:
 *  2026年2月12日, Once Day <once_day@qq.com>, 创建此文件.
 *
 */

#ifndef __EASEDS_PUBLIC_H__
#define __EASEDS_PUBLIC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* easeds 消息统一状态标识定义 */
#define EASEDS_OK                 (0)
#define EASEDS_ERROR              (-1)
#define EASEDS_ERROR_NULL_POINTER (-2) /* 空指针错误 */

#ifdef __cplusplus
}
#endif

#endif /* __EASEDS_PUBLIC_H__ */
