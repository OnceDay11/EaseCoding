/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 *
 * @FilePath: /book/ProgSelf/simple_section.c
 * @Author: Once Day <once_day@qq.com>.
 * @Date: 2026-02-09 17:23
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  程序员自我修养 - 3.1 章节测试用例源码
 *
 * @History:
 *  2026年2月9日, Once Day <once_day@qq.com>, 创建此文件.
 *
 */

// 不使用头文件, 直接声明函数和变量, 避免引入不必要的符号

int printf(const char *format, ...);

int global_init_var = 84;    // 全局初始化变量
int global_uninit_var;       // 全局未初始化变量

void func1(int i)
{
    printf("%d\n", i);
}

int main(void)
{
    static int static_var = 85;    // 静态变量
    static int static_var2;        // 静态未初始化变量

    int a = 1;
    int b;

    func1(static_var + static_var2 + a + b);

    return a;
}
