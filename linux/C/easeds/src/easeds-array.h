/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 *
 * @FilePath: /linux/C/easeds/src/easeds-array.h
 * @Author: Once Day <once_day@qq.com>.
 * @Date: 2026-02-11 23:33
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  动态数组相关数据结构实现
 *
 * @History:
 *  2026年2月11日, Once Day <once_day@qq.com>, 创建此文件.
 *
 */

#ifndef __EASEDS_ARRAY_H__
#define __EASEDS_ARRAY_H__

/* C 标准库头文件 */
#include <stdbool.h>
#include <stdint.h>

/* 项目内部头文件 */
#include "easeds-public.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 实现一个常规的动态数组, 地址空间是连续的, 支持自动扩容和缩容, 以及基本的增删改查操作.
 *  (1) 数组包含一个指向元素的指针, 当前元素数量, 数组容量, 元素大小等元信息.
 *  (2) 数组支持自动扩容和缩容, 当元素数量达到容量时, 自动扩容为原来的2倍;
 *      当元素数量小于容量的1/4时, 自动缩容为原来的一半.
 *  (3) 数组支持基本的增删改查操作.
 *  (4) 数组支持清空操作, 可以一次性删除所有元素, 但不释放数组内存, 以便后续继续使用.
 *  (5) 数组支持销毁操作, 释放数组内存, 包括元素内存和数组结构体内存.
 *  (6) 数组非线程安全, 需要用户自行保证线程安全性.
 */
struct easeds_array {
    void    *elements;     /* 指向元素的指针 */
    uint32_t element_size; /* 元素大小 */
    uint32_t size;         /* 当前元素数量 */
    uint32_t capacity;     /* 数组容量 */
    uint32_t flags;        /* 数组标志位, 预留字段 */
};

// 动态数组默认初始容量
#define EASEDS_ARRAY_DEFAULT_INITIAL_CAPACITY 64

/**
 * 常见数组操作函数:
 *
 * 函数名                       功能描述
 * ------------------------     ------------------------------------------------------
 * easeds_array_create          创建一个动态数组, 返回数组指针, 失败返回NULL
 * easeds_array_destroy         销毁动态数组, 释放内存
 * easeds_array_clear           清空动态数组, 删除所有元素, 但不释放内存
 * easeds_array_push_back       在数组末尾添加一个元素, 成功返回0, 失败返回-1
 * easeds_array_pop_back        删除数组末尾的一个元素, 成功返回0, 失败返回-1
 * easeds_array_insert          在指定索引位置插入一个元素, 成功返回0, 失败返回-1
 * easeds_array_remove          删除指定索引位置的元素, 成功返回0, 失败返回-1
 * easeds_array_get             获取指定索引位置的元素指针, 成功返回元素指针, 失败返回NULL
 * easeds_array_set             设置指定索引位置的元素值, 成功返回0, 失败返回-1
 * easeds_array_size            获取数组当前元素数量
 * easeds_array_capacity        获取数组当前容量
 * easeds_array_is_empty        判断数组是否为空, 为空返回true, 否则返回false
 * easeds_array_foreach         遍历数组元素, 对每个元素执行指定的回调函数
 * easeds_array_find            查找数组中满足条件的元素, 返回元素指针, 未找到返回NULL
 */

// 创建一个动态数组, 返回数组指针, 失败返回NULL
struct easeds_array *easeds_array_create(uint32_t element_size, uint32_t initial_capacity);

// 销毁动态数组, 释放内存
void easeds_array_destroy(struct easeds_array *array);

// 清空动态数组, 删除所有元素, 但不释放内存
void easeds_array_clear(struct easeds_array *array);

// 在数组末尾添加一个元素, 成功返回0, 失败返回-1
int32_t easeds_array_push_back(struct easeds_array *array, const void *element);

// 删除数组末尾的一个元素, 成功返回0, 失败返回-1
int32_t easeds_array_pop_back(struct easeds_array *array);

// 在指定索引位置插入一个元素, 成功返回0, 失败返回-1
int32_t easeds_array_insert(struct easeds_array *array, uint32_t index, const void *element);

// 删除指定索引位置的元素, 成功返回0, 失败返回-1
int32_t easeds_array_remove(struct easeds_array *array, uint32_t index);

// 获取指定索引位置的元素指针, 成功返回元素指针, 失败返回NULL
int32_t easeds_array_get(struct easeds_array *array, uint32_t index, void **element);

// 设置指定索引位置的元素值, 成功返回0, 失败返回-1
int32_t easeds_array_set(struct easeds_array *array, uint32_t index, const void *element);

// 获取数组当前元素数量
uint32_t easeds_array_size(struct easeds_array *array);

// 获取数组当前容量
uint32_t easeds_array_capacity(struct easeds_array *array);

// 判断数组是否为空, 为空返回true, 否则返回false
bool easeds_array_is_empty(struct easeds_array *array);

// 遍历数组元素, 对每个元素执行指定的回调函数
void easeds_array_foreach(
    struct easeds_array *array, void (*callback)(void *element, void *user_data), void *user_data);

// 查找数组中满足条件的元素, 返回元素指针, 未找到返回NULL
void *easeds_array_find(
    struct easeds_array *array, bool (*predicate)(void *element, void *user_data), void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* __EASEDS_ARRAY_H__ */
