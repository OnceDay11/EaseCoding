/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 *
 * @FilePath: /book/ProgSelf/static_a.c
 * @Author: Once Day <once_day@qq.com>.
 * @Date: 2026-02-11 11:44
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  程序员自我修养 - 第 4 章节测试用例源码 A
 *
 * @History:
 *  2026年2月11日, Once Day <once_day@qq.com>, 创建此文件.
 *
 */

// 不使用头文件, 直接声明函数和变量, 避免引入不必要的符号

extern int shared;

void my_exit(int status)
{
    __asm__(
        "movq %0, %%rdi\n"
        "movq $60, %%rax\n"
        "syscall\n"
        :
        : "r"((unsigned long)status)
        : "%rdi", "%rax");
}

int main(void)
{
    int a = 100;

    swap(&a, &shared);

    my_exit(0);
    return 0;
}
