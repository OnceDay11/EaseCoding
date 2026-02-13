/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 *
 * @FilePath: /linux/C/easeds/src/easeds-array.c
 * @Author: Once Day <once_day@qq.com>.
 * @Date: 2026-02-11 23:47
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  动态数组常见操作实现
 *
 * @History:
 *  2026年2月11日, Once Day <once_day@qq.com>, 创建此文件.
 *
 */

#include "easeds-array.h"

// 标准库头文件
#include <stdlib.h>
#include <string.h>

// 项目内部头文件
#include "easeds-log.h"

/**
 * @description: 创建一个动态数组, 返回数组指针, 失败返回NULL.
 * @param name 数组名称, 预留字段, 可用于调试和日志输出
 * @param element_size 元素大小, 单位字节
 * @param initial_capacity 初始容量, 如果为0则使用默认初始容量
 * @return 成功返回数组指针, 失败返回NULL
 */
struct easeds_array *easeds_array_create(
    const char *name, uint32_t element_size, uint32_t initial_capacity)
{
    if (initial_capacity == 0) {
        initial_capacity = EASEDS_ARRAY_DEFAULT_INITIAL_CAPACITY;    // 默认初始容量
    }

    struct easeds_array *array =
        (struct easeds_array *)__easeds_malloc(sizeof(struct easeds_array));
    if (unlikely(array == NULL)) {
        EASEDS_ERR("[easeds_array_create]: Failed to allocate memory for array struct.");
        return NULL;
    }

    array->elements = __easeds_malloc(element_size * initial_capacity);
    if (unlikely(array->elements == NULL)) {
        EASEDS_ERR("[easeds_array_create]: Failed to allocate memory for array elements.");
        __easeds_free(array);
        return NULL;
    }

    // 初始化数组元信息
    array->element_size = element_size;
    array->size         = 0;
    array->capacity     = initial_capacity;
    array->flags        = 0; /* 预留字段, 可用于扩展 */

    PFL_DEBUG(
        "Created array: element_size=%u, initial_capacity=%u", element_size, initial_capacity);
    return array;
}

// 销毁动态数组, 释放内存
void easeds_array_destroy(struct easeds_array *array)
{
    if (unlikely(array == NULL)) {
        return;
    }

    __easeds_free(array->elements); /* 释放元素内存 */
    __easeds_free(array);           /* 释放数组结构体内存 */

    PFL_DEBUG("Destroyed array.");
}

// 清空动态数组, 删除所有元素, 但不释放内存
void easeds_array_clear(struct easeds_array *array)
{
    if (unlikely(array == NULL)) {
        return;
    }

    array->size = 0; /* 仅重置元素数量, 不释放内存 */

    PFL_DEBUG("Cleared array, size reset to 0, capacity remains %u.", array->capacity);
}

// 在数组末尾添加一个元素, 成功返回0, 失败返回-1
int32_t easeds_array_push_back(struct easeds_array *array, const void *element)
{
    if (unlikely(array == NULL || element == NULL)) {
        EASEDS_ERR("[easeds_array_push_back]: Invalid array or element pointer.");
        return -1;
    }

    /* 如果数组已满, 则需要扩容 */
    if (array->size >= array->capacity) {
        uint32_t new_capacity = array->capacity * 2; /* 扩容为原来的两倍 */
        void    *new_elements = realloc(array->elements, array->element_size * new_capacity);
        if (unlikely(new_elements == NULL)) {
            EASEDS_ERR(
                "[easeds_array_push_back]: Failed to reallocate memory for array expansion.");
            return -1;
        }
        array->elements = new_elements;
        array->capacity = new_capacity;
        PFL_DEBUG("Expanded array capacity to %u.", new_capacity);
    }

    /* 将元素添加到数组末尾 */
    void *dest = (char *)array->elements + array->size * array->element_size;
    memcpy(dest, element, array->element_size); /* 复制元素值 */

    /* 更新数组大小 */
    array->size++;

    PFL_DEBUG("Pushed element to back, new size is %u.", array->size);
    return 0;
}

// 删除数组末尾的一个元素, 成功返回0, 失败返回-1
int32_t easeds_array_pop_back(struct easeds_array *array)
{
    if (unlikely(array == NULL)) {
        EASEDS_ERR("[easeds_array_pop_back]: Invalid array pointer.");
        return -1;
    }

    if (array->size == 0) {
        EASEDS_ERR("[easeds_array_pop_back]: Cannot pop from an empty array.");
        return -1;
    }

    /* 更新数组大小, 实际上并不需要清除元素值, 只需减少大小即可 */
    array->size--;

    PFL_DEBUG("Popped element from back, new size is %u.", array->size);
    return 0;
}

// 在指定索引位置插入一个元素, 成功返回0, 失败返回-1
int32_t easeds_array_insert(struct easeds_array *array, uint32_t index, const void *element)
{
    if (unlikely(array == NULL || element == NULL)) {
        EASEDS_ERR("[easeds_array_insert]: Invalid array or element pointer.");
        return -1;
    }

    if (index > array->size) {
        EASEDS_ERR(
            "[easeds_array_insert]: Index %u out of bounds, size is %u.", index, array->size);
        return -1;
    }

    /* 如果数组已满, 则需要扩容 */
    if (array->size >= array->capacity) {
        uint32_t new_capacity = array->capacity * 2; /* 扩容为原来的两倍 */
        void    *new_elements = realloc(array->elements, array->element_size * new_capacity);
        if (unlikely(new_elements == NULL)) {
            EASEDS_ERR("[easeds_array_insert]: Failed to reallocate memory for array expansion.");
            return -1;
        }
        array->elements = new_elements;
        array->capacity = new_capacity;
        PFL_DEBUG("Expanded array capacity to %u.", new_capacity);
    }

    /* 将元素插入到指定索引位置 */
    uint8_t *dest = (uint8_t *)array->elements + index * array->element_size;
    memmove(dest, element, array->element_size); /* 使用 memmove 处理重叠情况 */

    /* 更新数组大小 */
    if (index == array->size) {
        array->size++; /* 插入到末尾, 直接增加大小 */
    } else {
        memmove(dest + array->element_size, dest, (array->size - index) * array->element_size);
        memcpy(dest, element, array->element_size); /* 插入元素 */
        array->size++;                              /* 增加大小 */
    }

    PFL_DEBUG("Inserted element at index %u, new size is %u.", index, array->size);
    return 0;
}

// 删除指定索引位置的元素, 成功返回0, 失败返回-1
int32_t easeds_array_remove(struct easeds_array *array, uint32_t index)
{
    if (unlikely(array == NULL)) {
        EASEDS_ERR("[easeds_array_remove]: Invalid array pointer.");
        return -1;
    }

    if (index >= array->size) {
        EASEDS_ERR(
            "[easeds_array_remove]: Index %u out of bounds, size is %u.", index, array->size);
        return -1;
    }

    /* 将元素从指定索引位置删除 */
    uint8_t *dest = (uint8_t *)array->elements + index * array->element_size;
    memmove(dest, dest + array->element_size, (array->size - index - 1) * array->element_size);

    /* 更新数组大小 */
    array->size--;

    PFL_DEBUG("Removed element at index %u, new size is %u.", index, array->size);
    return 0;
}

// 获取指定索引位置的元素指针, 成功返回元素指针, 失败返回NULL
int32_t easeds_array_get(struct easeds_array *array, uint32_t index, void **element)
{
    if (unlikely(array == NULL || element == NULL)) {
        EASEDS_ERR("[easeds_array_get]: Invalid array or element pointer.");
        return -1;
    }

    if (index >= array->size) {
        EASEDS_ERR("[easeds_array_get]: Index %u out of bounds, size is %u.", index, array->size);
        return -1;
    }

    /* 计算元素指针并返回 */
    *element = (char *)array->elements + index * array->element_size;

    PFL_DEBUG("Got element at index %u.", index);
    return 0;
}

// 设置指定索引位置的元素值, 成功返回0, 失败返回-1
int32_t easeds_array_set(struct easeds_array *array, uint32_t index, const void *element)
{
    if (unlikely(array == NULL || element == NULL)) {
        EASEDS_ERR("[easeds_array_set]: Invalid array or element pointer.");
        return -1;
    }

    if (index >= array->size) {
        EASEDS_ERR("[easeds_array_set]: Index %u out of bounds, size is %u.", index, array->size);
        return -1;
    }

    /* 计算元素指针并设置值 */
    uint8_t *dest = (uint8_t *)array->elements + index * array->element_size;
    memcpy(dest, element, array->element_size);

    PFL_DEBUG("Set element at index %u.", index);
    return 0;
}

// 获取数组当前元素数量
uint32_t easeds_array_size(struct easeds_array *array)
{
    if (unlikely(array == NULL)) {
        EASEDS_ERR("[easeds_array_size]: Invalid array pointer.");
        return 0;
    }

    return array->size;
}

// 获取数组当前容量
uint32_t easeds_array_capacity(struct easeds_array *array)
{
    if (unlikely(array == NULL)) {
        EASEDS_ERR("[easeds_array_capacity]: Invalid array pointer.");
        return 0;
    }

    return array->capacity;
}

// 遍历数组元素, 对每个元素执行指定的回调函数
void easeds_array_foreach(
    struct easeds_array *array, void (*callback)(void *element, void *user_data), void *user_data)
{
    if (unlikely(array == NULL || callback == NULL)) {
        EASEDS_ERR("[easeds_array_foreach]: Invalid array pointer or callback function.");
        return;
    }

    for (uint32_t i = 0; i < array->size; i++) {
        void *element = (char *)array->elements + i * array->element_size;
        callback(element, user_data);
    }
}
