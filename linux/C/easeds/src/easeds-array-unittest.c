/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 *
 * @FilePath: /linux/C/easeds/src/easeds-array-unittest.c
 * @Author: Once Day <once_day@qq.com>.
 * @Date: 2026-02-13 00:14
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  Easeds 动态数组单元测试实现文件, 包含了动态数组的基本功能测试.
 *
 * @History:
 *  2026年2月13日, Once Day <once_day@qq.com>, 创建此文件.
 *
 */

#include "easeds-unittest.h"

// 项目内部头文件
#include "easeds-array.h"

// 基本功能测试: 创建数组, 获取元素数量和容量, 销毁数组
static void test_easeds_array_basic(void **state)
{
    easeds_unused(state);

    struct easeds_array *array = easeds_array_create(sizeof(int), 16);
    assert_non_null(array);
    assert_int_equal(easeds_array_size(array), 0);
    assert_int_equal(easeds_array_capacity(array), 16);
    easeds_array_destroy(array);
}

// 基本功能测试: 插入元素, 修改元素, 删除元素, 获取元素
static void test_easeds_array_operations(void **state)
{
    easeds_unused(state);

    struct easeds_array *array = easeds_array_create(sizeof(int), 4);
    assert_non_null(array);

    // 插入元素
    for (int i = 0; i < 10; i++) {
        int value = i * 10;
        assert_int_equal(easeds_array_push_back(array, &value), EASEDS_OK);
        assert_int_equal(easeds_array_size(array), i + 1);
    }

    // 修改元素
    for (uint32_t i = 0; i < 10; i++) {
        int value = (int)(i + 1) * 10;
        assert_int_equal(easeds_array_set(array, i, (void *)&value), EASEDS_OK);
    }

    // 获取元素
    for (uint32_t i = 0; i < 10; i++) {
        int *pvalue;
        assert_int_equal(easeds_array_get(array, i, (void **)&pvalue), EASEDS_OK);
        assert_int_equal(*pvalue, (i + 1) * 10);
    }

    // 删除元素
    for (int i = 0; i < 10; i++) {
        assert_int_equal(easeds_array_pop_back(array), EASEDS_OK);
        assert_int_equal(easeds_array_size(array), 9 - i);
    }

    easeds_array_destroy(array);
}

// 注册单元测试用例
EASEDS_UNITTEST_REGISTER(easeds_unittest_array){
    cmocka_unit_test(test_easeds_array_basic),
    cmocka_unit_test(test_easeds_array_operations),
    easeds_unit_test_end,
};
