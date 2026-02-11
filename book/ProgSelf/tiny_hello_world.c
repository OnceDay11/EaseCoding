/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 *
 * @FilePath: /book/ProgSelf/tiny_hello_world.c
 * @Author: Once Day <once_day@qq.com>.
 * @Date: 2026-02-11 17:26
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  程序员自我修养 - 第 4 章节测试用例源码 C
 *
 * @History:
 *  2026年2月11日, Once Day <once_day@qq.com>, 创建此文件.
 *
 */

// 不使用头文件, 直接声明函数和变量, 避免引入不必要的符号

void my_write(int fd, const void *buf, unsigned long count)
{
    __asm__(
        "movq %0, %%rdi\n"
        "movq %1, %%rsi\n"
        "movq %2, %%rdx\n"
        "movq $1, %%rax\n"
        "syscall\n"
        :
        : "r"((unsigned long)fd), "r"(buf), "r"(count)
        : "%rdi", "%rsi", "%rdx", "%rax");
}

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

int no_main(void)
{
    my_write(1, "Hello, World!\n", 14);
    my_exit(0);
    return 0;
}
