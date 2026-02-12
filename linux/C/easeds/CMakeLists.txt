###
 # SPDX-License-Identifier: BSD-3-Clause
 #
 # Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 #
 # @FilePath: /linux/C/easeds/CMakeLists.txt
 # @Author: Once Day <once_day@qq.com>.
 # @Date: 2026-02-12 22:22
 # @info: Encoder=utf-8, TabSize=4, Eol=\n.
 #
###

# cmake版本号要求
cmake_minimum_required(VERSION 3.10)

# 确认 EASEDS_MAJOR_VERSION/EASEDS_MINOR_VERSION/EASEDS_PATCH_VERSION 是否定义, 如果未定义, 则使用默认值
if (NOT DEFINED EASEDS_MAJOR_VERSION)
    set(EASEDS_MAJOR_VERSION 1)
endif()
if (NOT DEFINED EASEDS_MINOR_VERSION)
    set(EASEDS_MINOR_VERSION 0)
endif()
if (NOT DEFINED EASEDS_PATCH_VERSION)
    set(EASEDS_PATCH_VERSION 0)
endif()

# 定义 EASEDS_VERSION, 如果未定义, 则使用默认值
set(EASEDS_VERSION "${EASEDS_MAJOR_VERSION}.${EASEDS_MINOR_VERSION}.${EASEDS_PATCH_VERSION}")
set(EASEDS_SO_VERSION "${EASEDS_MAJOR_VERSION}")

# 项目名称/版本号
project(EASEDS VERSION ${EASEDS_VERSION})

# 使能编译语言 C ASM
enable_language(C ASM)

# 设置编译器, 使用 -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g 指定
if (NOT CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER "gcc")
endif()

# 记录顶层目录, 子目录可以使用
set(TOPDIR ${CMAKE_CURRENT_SOURCE_DIR})

# 设置编译类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Releaseds")
endif()

# 转换为统一大写模式
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

# 将 CMAKE_BUILD_TYPE 转成固定值 Releaseds/Debug/RelWithDebInfo/MinSizeRel
if (BUILD_TYPE STREQUAL "DEBUG")
    set(CMAKE_BUILD_TYPE "Debug")
elseif (BUILD_TYPE STREQUAL "RELEASEDS")
    set(CMAKE_BUILD_TYPE "Releaseds")
elseif (BUILD_TYPE STREQUAL "MIN-SIZE")
    set(CMAKE_BUILD_TYPE "MinSizeRel")
endif()

# 设置输出二进制类型为位置独立代码
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(C_FLAGS
    # -DVALGRIND
    -D_GNU_SOURCE=1         # GNU扩展
    -DCHECK_PTHREAD_RETURN_VALUE
    -D_FILE_OFFSET_BITS=64
    -Wall                   # 打开所有警告
    -Wextra                 # 打开额外警告
    -Werror                 # 严格的错误处理策略
    -Wshadow                # 检查变量声明遮蔽
    -Wundef                 # 未定义的宏
    -Wcast-qual             # 类型转换时丢失限定符
    -Wcast-align            # 类型转换时对齐
    -Wstrict-prototypes     # 函数原型声明
    -Wmissing-prototypes    # 缺少函数原型声明
    -Wmissing-declarations  # 缺少声明
    -Wnested-externs        # 嵌套的extern声明
    -Wunreachable-code      # 不可达的代码
    -Wuninitialized         # 未初始化的变量
    -Winline                # 内联函数
    -Wfloat-equal           # 浮点数比较
    -Wdouble-promotion      # 浮点数提升
    -Wswitch                # 检查switch语句中的枚举值, switch case集合必须和枚举定义集合一致
    -Wswitch-default        # 检查switch语句中的default, 没有default将报错
    -Wbad-function-cast     # 检查函数指针强制转换
    -Waggregate-return      # 不允许返回结构体或联合体
    -Wpacked                # 检查结构体或联合体的字节对齐, 如果对齐不合理, 将报错
    -Wpadded                # 检查结构体或联合体的字节对齐, 如果额外填充了字节, 将报错
    -Wvariadic-macros       # 检查宏定义
    -Wvla                   # 变长数组
    -Wconversion            # 检查隐式类型转换
    -Wsign-conversion       # 检查隐式类型转换
    -Wpointer-arith         # 指针运算
    -Wwrite-strings         # 字符串常量
    -Woverlength-strings    # 字符串常量
    -Wpedantic              # 严格遵循ISO C/C标准，检查编译器扩展语法
    -Wformat=2              # 格式化字符串
    -Wnull-dereference      # 空指针解引用
    -Wno-unused-parameter   # 未使用的参数
    -Wno-unused-variable    # 未使用的变量
    -Wno-unused-function    # 未使用的函数
    -Wmisleading-indentation   # 误导性缩进
    -Wsizeof-pointer-memaccess # 访问指针大小
    # -MMD
    # -march=native           # 优化
    # -fstack-protector-all   # 栈保护, 根据编译类型确定
    -std=c11                  # C11标准
    # -rdynamic               # 生成位置无关代码
    -fno-omit-frame-pointer   # 不省略栈帧指针
    # -flto                   # 启用链接时优化 (Link Time Optimization)
)

# 根据编译器类型设置编译器选项
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    # GCC编译器特有选项
    set(GUN_C_FLAGS
        -Wjump-misses-init      # 跳转到未初始化的变量
        -Wformat-overflow=2     # 格式化字符串溢出
        -Wformat-truncation=2   # 格式化字符串截断
        -Walloc-zero            # 分配内存时检查是否为0
        -Wduplicated-cond       # 重复的条件
        -Wduplicated-branches   # 重复的分支
        -Wlogical-op            # 逻辑操作符
    )
    set(GUN_CXX_FLAGS
        -Wformat-overflow=2     # 格式化字符串溢出
        -Wformat-truncation=2   # 格式化字符串截断
        -Walloc-zero            # 分配内存时检查是否为0
        -Wduplicated-cond       # 重复的条件
        -Wduplicated-branches   # 重复的分支
        -Wlogical-op            # 逻辑操作符
    )

    # 添加到 C_FLAGS 和 CXX_FLAGS
    list(APPEND C_FLAGS ${GUN_C_FLAGS})
    list(APPEND CXX_FLAGS ${GUN_CXX_FLAGS})
    message(STATUS "GCC: Using GNU compiler specific flags")
elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    # Clang编译器特有选项
    message(STATUS "Clang: Using Clang compiler specific flags")
endif()

# 启用覆盖率测试
option(COVERAGE "Enable coverage reporting" OFF)
if(COVERAGE)
  list(APPEND C_FLAGS "--coverage -D_EASEDS_COVERAGE")
  list(APPEND C_FLAGS "-fprofile-dir=${CMAKE_CURRENT_SOURCE_DIR}/build/output/coverage")
endif()

# 设置内部编译选项
string(REPLACE ";" " " CMAKE_C_FLAGS_INNER "${C_FLAGS}")

# 添加内部编译选项到全局编译选项
string(APPEND CMAKE_C_FLAGS " ${CMAKE_C_FLAGS_INNER}")

# 设置链接选项 LDFLAGS
set(LD_FLAGS
    -Wl,--no-undefined              # 未定义的符号
    -Wl,--no-allow-shlib-undefined  # 不允许符号覆盖
    -Wl,--warn-common               # 重名的公共符号
    -Wl,--gc-sections               # 垃圾回收, 删除未使用的符号
    # -flto                           # 启用链接时优化 (Link Time Optimization)
    # -Wl,--verbose                 # 显示链接器的详细信息
)

# 编译libasan版本 - 静态库版本
option(BUILD_ASAN "Build with AddressSanitizer" OFF)
if (LIBASAN)
    set(LIBASAN_FLAGS
        -fsanitize=address              # 启用地址检查
        -fno-omit-frame-pointer         # 不省略栈帧指针
        -fno-optimize-sibling-calls     # 禁止优化兄弟调用
        -fno-common                     # 禁止全局变量的重复定义
        -fsanitize-recover=address)     # 允许地址检查错误恢复
    set(LIBASAN_STATIC_LIB "asan.a")    # 单元测试使用静态库
    set(LIBASAN_LINK_OPTION "-fsanitize=address") # 动态库和守护进程使用动态库
    message(STATUS "Libasan: Build with AddressSanitizer")
endif()

# 设置内部链接器选项
string(REPLACE ";" " " CMAKE_EXE_LINKER_FLAGS_INNER "${LD_FLAGS}")
string(REPLACE ";" " " CMAKE_SHARED_LINKER_FLAGS_INNER "${LD_FLAGS}")

# 添加内部链接器选项到全局链接器选项
string(APPEND CMAKE_EXE_LINKER_FLAGS " ${CMAKE_EXE_LINKER_FLAGS_INNER}")
string(APPEND CMAKE_SHARED_LINKER_FLAGS " ${CMAKE_SHARED_LINKER_FLAGS_INNER}")

# 设置debug版本编译选项, 开启调试信息, 不优化, 开启栈保护
set(DEBUG_FLAGS
    -O0
    -g                          # 携带调试信息
    -fstack-protector-all       # 栈保护
)
string(REPLACE ";" " " CMAKE_C_FLAGS_DEBUG "${DEBUG_FLAGS}")

# 设置releaseds版本编译选项
set(RELEASEDS_FLAGS
    -O2                         # 优化等级2
    -g                          # 生成调试信息
    -DNDEBUG                    # 禁用断言
    -fstack-protector-all       # 栈保护
)
string(REPLACE ";" " " CMAKE_C_FLAGS_RELEASEDS "${RELEASEDS_FLAGS}")

# 设置min-size版本编译选项
set(MIN_SIZE_FLAGS
    -Os                         # 优化代码尺寸
    -g                          # 生成调试信息
    -DNDEBUG                    # 禁用断言
    # -fstack-protector-all       # 栈保护
    # -fno-rtti                   # 禁用RTTI(运行时类型信息)
)
string(REPLACE ";" " " CMAKE_C_FLAGS_MINSIZEREL "${MIN_SIZE_FLAGS}")

# 添加头文件搜索路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 选择编译类型对应的编译标识
message(STATUS "C_FLAGS = " ${CMAKE_C_FLAGS} " " ${CMAKE_C_FLAGS_${BUILD_TYPE}})
message(STATUS "CMAKE_EXE_LINKER_FLAGS = " ${CMAKE_EXE_LINKER_FLAGS})
message(STATUS "CMAKE_SHARED_LINKER_FLAGS = " ${CMAKE_SHARED_LINKER_FLAGS})
message(STATUS "TARGET_ARCH = " ${TARGET_ARCH})
message(STATUS "BUILD_ID = " ${BUILD_ID})

# 只有文件更新时才需要打印消息
set(CMAKE_INSTALL_MESSAGE LAZY)

# 安装 EASEDS 辅助文件
set(EASEDS_EXEC_INSTALL_DIR usr/bin)
set(EASEDS_LIB_INSTALL_DIR usr/lib)
set(EASEDS_HEADER_INSTALL_DIR usr/include)
set(EASEDS_DATA_INSTALL_DIR etc/easeds)
set(EASEDS_UNITTEST_INSTALL_DIR ${EASEDS_DATA_INSTALL_DIR}/unittest)
set(EASEDS_INSTALL_DIR usr/bin/easeds)

# 添加源文件列表
add_subdirectory(src)
