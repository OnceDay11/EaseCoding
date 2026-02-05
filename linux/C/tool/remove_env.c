/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 *
 * @FilePath: /linux/C/tool/remove_env.c
 * @Author: Once Day <once_day@qq.com>.
 * @Date: 2026-02-05 14:59
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  实现一个函数, 支持修改 LD_PRELOAD 环境变量, 剥离 libasan.so 库路径.
 *
 * @History:
 *  2026年2月5日, Once Day <once_day@qq.com>, 创建此文件.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 从 LD_PRELOAD 环境变量中移除 libasan.so 路径
void remove_libasan_from_ld_preload(void)
{
    const char *ld_env = getenv("LD_PRELOAD");
    if (ld_env == NULL) {
        // LD_PRELOAD 未设置, 直接返回
        return;
    }

    char *env_dup = strdup(ld_env);
    if (env_dup == NULL) {
        // 内存分配失败, 直接返回
        return;
    }

#define LIBASAN_PRELOAD_STR "/usr/lib/libasan.so"

    // 查找 libasan.so 路径
    char *pos = strstr(env_dup, LIBASAN_PRELOAD_STR);
    if (pos != NULL) {
        // 进行字符串拷贝
        char *copy = pos + sizeof(LIBASAN_PRELOAD_STR) - 1;
        if (*copy == ':') {
            copy++;    // 跳过冒号
        }

        while (*copy != '\0') {
            *pos++ = *copy++;
        }

        // 移除末尾的冒号, 结尾添加 '\0'
        if (pos != env_dup && *(pos - 1) == ':') {
            pos--;
        }
        *pos = '\0';

        // 更新环境变量
        setenv("LD_PRELOAD", env_dup, 1);
    }

    free(env_dup);
    return;
}

int main(void)
{
    // 测试数据
    const char *test_cases[] = {
        "/usr/lib/libasan.so:/usr/lib/libother.so",
        "/usr/lib/libother.so:/usr/lib/libasan.so",
        "/usr/lib/libother.so",
        "/usr/lib/libasan.so",
        "/usr/lib/libasan.so:/usr/lib/libother1.so:/usr/lib/libother2.so",
        "",
        NULL,
    };

    for (int i = 0; test_cases[i] != NULL; i++) {
        // 设置测试环境变量
        setenv("LD_PRELOAD", test_cases[i], 1);
        printf("Before: LD_PRELOAD=%s\n", getenv("LD_PRELOAD"));

        // 移除 libasan.so 路径
        remove_libasan_from_ld_preload();

        // 输出结果
        printf("After:  LD_PRELOAD=%s\n\n", getenv("LD_PRELOAD"));
    }

    return 0;
}
