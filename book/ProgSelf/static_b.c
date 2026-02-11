/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 *
 * @FilePath: /book/ProgSelf/static_b.c
 * @Author: Once Day <once_day@qq.com>.
 * @Date: 2026-02-11 11:45
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  程序员自我修养 - 第 4 章节测试用例源码 B
 *
 * @History:
 *  2026年2月11日, Once Day <once_day@qq.com>, 创建此文件.
 *
 */

// 不使用头文件, 直接声明函数和变量, 避免引入不必要的符号
int shared = 1;

void swap(int *a, int *b)
{
    *a ^= *b = *a ^= *b;
}
