/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 *
 * @FilePath: /linux/C/easeds/src/easeds-unittest.c
 * @Author: Once Day <once_day@qq.com>.
 * @Date: 2026-02-12 23:34
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  Easeds 单元测试实现文件, 使用 CMocka 作为单元测试框架, 包含了数据结构库的基本功能测试.
 *
 * @History:
 *  2026年2月12日, Once Day <once_day@qq.com>, 创建此文件.
 *
 */

#include "easeds-unittest.h"

/* 系统库头文件 */
#include <unistd.h>

/* C标准库头文件 */
#include <getopt.h>

#ifndef _EASEDS_UNITTEST
#error "Easeds Client unittest must be compiled with -D_EASEDS_UNITTEST"
#endif /* _EASEDS_UNITTEST */

/* 定义单元测试节点尾链表 */
STAILQ_HEAD(easeds_unittest_head, easeds_unittest_node);

/* 单元测试链表 */
static struct easeds_unittest_head g_easeds_unittest_head =
    STAILQ_HEAD_INITIALIZER(g_easeds_unittest_head);

/* 单元测试数量 */
static uint32_t g_easeds_unittest_num = 0;

/* 单元测试输出流 */
static FILE *g_easeds_output_stream  = NULL;
static FILE *g_easeds_invalid_stream = NULL;

/* 日志控制辅助函数 */
static void easeds_unittest_log_control(bool debug, bool verbose, bool measure)
{
    /* 设置调试模式 */
    easeds_log_debug_state_switch(debug);
    /* 设置详细输出模式 */
    easeds_log_verbose_state_switch(verbose);
    /* 设置性能测量模式 */
    easeds_log_measure_state_switch(measure);
}

/* 获取无效输出流 */
FILE *easeds_unittest_invalid_stream(void)
{
    if (unlikely(g_easeds_invalid_stream == NULL)) {
        g_easeds_invalid_stream = fopen("/dev/null", "w");
        if (g_easeds_invalid_stream == NULL) {
            EASEDS_ERR("Failed to open /dev/null for invalid stream.");
            // 如果无法打开 /dev/null, 则使用 stderr 作为无效输出流
            return stderr;
        }
    }

    return g_easeds_invalid_stream;
}

/* 单元测试输出流函数: 输出到 /dev/null 或者 stderr(debug使能) */
FILE *easeds_unittest_output_stream(void)
{
    // 如果 Debug 状态开启
    if (easeds_log_debug_state()) {
        return stderr;    // 返回标准错误流作为无效输出流
    }

    return g_easeds_output_stream ? g_easeds_output_stream : stderr;
}

/* 单元测试设置输出流 */
static void easeds_unittest_set_output_stream(FILE *stream)
{
    if (stream == NULL) {
        EASEDS_ERR("Invalid output stream, using default stderr.");
        g_easeds_output_stream = stderr;
    } else {
        g_easeds_output_stream = stream;
    }
}

/* 单元测试输出流资源回收 */
static void easeds_unittest_invalid_stream_release(void)
{
    if (g_easeds_invalid_stream != NULL && g_easeds_invalid_stream != stderr) {
        fclose(g_easeds_invalid_stream);
        g_easeds_invalid_stream = NULL;
    }
}

/**
 * 注册单元测试到链表中, 通过全局构造函数来实现单元测试用例自动注册.
 * @attention 内部函数(参数始终有效), 线程不安全, 不可重入.
 * @param test_node: 单元测试节点
 * @note test_node->num == 0 时, 测试用例数目需要遍历获取.
 * @return: 无返回值.
 */
void easeds_unittest_register(struct easeds_unittest_node *test_node)
{
    /* 如果 test_node->test_num == 0, 通过编译测试用例数组获取节点数 */
    if (test_node->test_num == 0) {
        for (const struct CMUnitTest *test = test_node->tests; test->name != NULL; test++) {
            test_node->test_num++;
        }
    }

    PFL_DEBUG("Register unit test: %s(%p) x %u.", test_node->name, (void *)test_node,
        test_node->test_num);
    STAILQ_INSERT_TAIL(&g_easeds_unittest_head, test_node, next);
    g_easeds_unittest_num += test_node->test_num;
}

/* Sizeof Dump 辅助函数, 这样总是对齐打印 */
void easeds_unittest_sizeof_dump(const char *name, uint64_t size)
{
    EASEDS_INFO("%s => %lu bytes.", name, size);
}

/* 打印所有的单元测试用例 */
static void easeds_unittest_dump(void)
{
    struct easeds_unittest_node *test_node;

    STAILQ_FOREACH(test_node, &g_easeds_unittest_head, next)
    {
        easeds_printf(
            "Test: %s(%p) x %u.\n", test_node->name, (void *)test_node, test_node->test_num);
        for (uint32_t i = 0; i < test_node->test_num; i++) {
            easeds_printf("  %s\n", test_node->tests[i].name);
        }
    }
}

/* 定义单元测试命令行参数 */
static struct option long_options[] = {
    {"help",        no_argument,       0, 'h'}, /* 帮助信息 */
    {"group",       required_argument, 0, 'g'}, /* 测试组名称 */
    {"filter",      required_argument, 0, 'f'}, /* 过滤条件 */
    {"skip",        required_argument, 0, 's'}, /* 跳过条件 */
    {"list",        no_argument,       0, 'l'}, /* 列出单元测试 */
    {"xml",         no_argument,       0, 'x'}, /* 输出 XML 格式 */
    {"performance", no_argument,       0, 'p'}, /* 性能环境 */
    {"verbose",     no_argument,       0, 'v'}, /* 输出详细信息 */
    {"debug",       no_argument,       0, 'd'}, /* 打印调试信息 */
    {0,             0,                 0, 0  },
};

static void easeds_unittest_help(__easeds_unused int32_t argc, char *const *argv)
{
    easeds_printf("Usage: %s [options]\n", argv[0]);
    easeds_printf("Options:\n");
    easeds_printf("  -h, --help            : print this help message.\n");
    easeds_printf("  -g, --group name      : run the test group with the given name.\n");
    easeds_printf("  -f, --filter pattern  : only run the ones matching the pattern.\n");
    easeds_printf("  -s, --skip pattern    : skip the ones matching the pattern.\n");
    easeds_printf("  -l, --list            : list all available tests.\n");
    easeds_printf("  -x, --xml             : output in XML format.\n");
    easeds_printf("  -p, --performance     : performance environment.\n");
    easeds_printf("  -v, --verbose         : print verbose information.\n");
    easeds_printf("  -d, --debug           : print debug information.\n");
}

/**
 * @brief 解析命令行参数
 * @param argc: 命令行参数的数量, 通常来自 main 函数的参数.
 * @param argv: 命令行参数的数组, 通常来自 main 函数的参数.
 * @param filter: 过滤条件, 用于指定需要执行的测试用例.
 * @param group: 运行指定的测试组, 如果为 NULL 则不指定.
 * @param debug: 是否开启调试模式, 如果为 NULL 则不修改调试状态.
 * @return: 返回值为 EASEDS_UNITTEST_CONTINUE 表示继续执行，返回值为 EASEDS_UNITTEST_EXIT 表示退出。
 */
static int32_t easeds_unittest_parse_cmdline(
    int32_t argc, char *const *argv, const char **group, bool *debug)
{
    int32_t opt;

    /**
     * @brief 解析命令行参数
     * @param argc: 命令行参数的数量, 通常来自 main 函数的参数.
     * @param argv: 命令行参数的数组, 通常来自 main 函数的参数.
     * @param short_options: 定义短选项的字符串, 每个选项字符后可跟一个冒号 : 表示需要参数.
     * @param long_options: 指向长选项数组的指针, 每个元素描述一个长选项.
     * @param long_index: 指向一个整数, getopt_long 会在其中存储被解析的长选项在 __longopts
     * 数组中的索引. 如果不需要这个信息, 可以传 NULL.
     * @return: 返回值为 EASEDS_UNITTEST_CONTINUE 表示继续执行，返回值为 EASEDS_UNITTEST_EXIT
     * 表示退出。
     */
    while ((opt = getopt_long(argc, argv, "hg:f:s:lxpvd", long_options, NULL)) != -1) {
        switch (opt) {
        case 'h':
            easeds_unittest_help(argc, argv);
            return EASEDS_UNITTEST_EXIT;
        case 'g':
            /* 运行指定的测试组 */
            EASEDS_INFO("Run test group: %s.", optarg);
            if (group != NULL) {
                *group = optarg;
            }
            break;
        case 'f':
            /* 存在 filter 参数时, 把 Debug 宏给打开, 输出更多信息, 方便调试 */
            EASEDS_INFO("Set filter to %s.", optarg);
            easeds_log_debug_state_switch(true);
            cmocka_set_test_filter(optarg);
            break;
        case 's':
            cmocka_set_skip_filter(optarg);
            break;
        case 'l':
            easeds_unittest_dump();
            return EASEDS_UNITTEST_EXIT;
        case 'x':
            /* 设置输出格式为 XML */
            EASEDS_INFO("Set output format to XML.");
            cmocka_set_message_output(CM_OUTPUT_XML);
            break;
        case 'p':
            /* 性能环境, 关闭 Debug 宏 */
            EASEDS_INFO("Set performance environment, debug on => false.");
            easeds_log_measure_state_switch(true);
            break;
        case 'v':
            easeds_log_verbose_state_switch(true);
            break;
        case 'd':
            /* 打开调试信息 */
            EASEDS_INFO("Debug mode enabled.");
            easeds_log_debug_state_switch(true);
            if (debug != NULL) {
                *debug = true;
            }
            break;
        default:
            easeds_unittest_help(argc, argv);
            return EASEDS_UNITTEST_ERROR;
        }
    }

    return EASEDS_UNITTEST_CONTINUE;
}

/* 单元测试执行主函数 */
int32_t easeds_unittest_main(int32_t argc, char **argv, int32_t (*global_init)(void))
{
    struct easeds_unittest_node *test_node  = NULL;  /* 单元测试节点 */
    const char                  *group      = NULL;  /* 过滤条件 */
    int32_t                      ret        = 0;     /* 返回值 */
    bool                         debug      = false; /* 是否开启调试模式 */
    bool                         debug_on   = false; /* 是否开启调试模式 */
    bool                         verbose_on = false; /* 是否开启详细输出模式 */
    bool                         measure_on = false; /* 是否开启性能测量模式 */

    /* 如果不存在参数, 直接跳到后续处理 */
    if (argc > 1 &&
        easeds_unittest_parse_cmdline(argc, argv, &group, &debug) == EASEDS_UNITTEST_EXIT) {
        goto end;
    }

    // 如果 debug 为 false, 则在 global_init 阶段关闭debug和verbose日志
    // 当性能模式开启之后, debug 和 verbose 模式会被关闭
    measure_on = easeds_log_measure_state();
    debug_on   = easeds_log_debug_state() && !measure_on;
    verbose_on = easeds_log_verbose_state() && !measure_on;
    if (!debug || measure_on) {
        EASEDS_INFO("Debug mode is disabled, switch debug and verbose to false when global init.");
        easeds_unittest_log_control(false, false, measure_on);
    }

    // 如果 verbose_on 开启, 则开启 debug_on 模式
    debug_on = debug_on || verbose_on;
    // 如果 debug_on 开启, 则开启 measure_on 模式
    measure_on = measure_on || debug_on;

    // 如果存在全局初始化函数, 则调用它
    if (global_init != NULL) {
        ret = global_init();
        if (ret != 0) {
            EASEDS_ERR("Global initialization failed, exit.");
            goto end;
        }
        EASEDS_INFO("Global initialization succeeded.");
    }

    easeds_unittest_log_control(debug_on, verbose_on, measure_on);

    /* 单元测试用例数不能为零, 至少包含测试框架自身用例 */
    easeds_assert(g_easeds_unittest_num != 0);

    /* 申请内存创建 Test 数组 */
    EASEDS_INFO("Total unit test number: %u.", g_easeds_unittest_num);

    /* 单元测试开始, 切换默认输出到无效流 */
    easeds_unittest_set_output_stream(easeds_unittest_invalid_stream());

    /* 遍历链表, 逐个执行单元测试用例组 */
    STAILQ_FOREACH(test_node, &g_easeds_unittest_head, next)
    {
        /* 过滤执行组, 包含组关键字即可 */
        if (group != NULL && strstr(test_node->name, group) == NULL) {
            continue;
        }

        ret |= _cmocka_run_group_tests(test_node->name, test_node->tests, test_node->test_num,
            test_node->setup, test_node->teardown);
    }

end:
    /* 释放单元测试输出流资源 */
    easeds_unittest_invalid_stream_release();

    return ret;
}

/* 单元测试可执行文件入口函数 */
int32_t main(int32_t argc, char **argv)
{
    return easeds_unittest_main(argc, argv, NULL);
}
