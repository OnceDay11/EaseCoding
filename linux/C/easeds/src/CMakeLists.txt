# 子目录辅助 CmakeLists.txt 文件, 定义了 Easeds 模块的构建规则和依赖关系.
# 添加源文件
set(easeds_SRCS
    easeds-array.c
    easeds-log.c
    easeds-utils.c
  )

# 添加单元测试源文件
set(easeds_unittest_SRCS
    easeds-unittest.c
    easeds-array-unittest.c
    )

# 添加链接库
set(easeds_LIBS
  )

# 添加单元测试链接库
set(easeds_unittest_LIBS
    m
    cmocka
  )

# 设置链接参数
set(easeds_LDFLAGS
    -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libeaseds.map
    -Wl,--build-id
  )

# 设置链接参数
set(easeds_unittest_LDFLAGS
    -Wl,--build-id
  )

message(STATUS "easeds_SRCS: ${easeds_SRCS}")
message(STATUS "easeds_unittest_SRCS: ${easeds_unittest_SRCS}")
message(STATUS "easeds_LIBS: ${easeds_LIBS}")
message(STATUS "easeds_unittest_LIBS: ${easeds_unittest_LIBS}")
message(STATUS "easeds_LDFLAGS: ${easeds_LDFLAGS}")
message(STATUS "easeds_unittest_LDFLAGS: ${easeds_unittest_LDFLAGS}")

# 添加选项, 编译单元测试可执行文件
option(BUILD_EASEDS_UNITTEST "Build EASEDS unit tests" OFF)

# 如果开启了单元测试编译选项
if(BUILD_EASEDS_UNITTEST)
    # 编译可执行文件 - 单元测试执行用例
    add_executable(easeds-unittest ${easeds_SRCS} ${easeds_unittest_SRCS})

    # 添加单元测试宏定义
    target_compile_options(easeds-unittest PRIVATE
        -D_EASEDS_UNITTEST # 定义单元测试宏
        -D_EASEDS_SHARED # 总是按照动态库编译, 以便于测试
        ${LIBASAN_FLAGS} # 如果使用了 AddressSanitizer
    )

    # 单元测试添加链接库 cmocka
    target_link_libraries(easeds-unittest PRIVATE
        ${LIBASAN_STATIC_LIB} # 如果使用了 AddressSanitizer
        ${easeds_LIBS}
        ${easeds_unittest_LIBS}
    )

    # 设置链接参数
    target_link_options(easeds-unittest PRIVATE
        ${easeds_LDFLAGS}
        ${easeds_unittest_LDFLAGS}
    )

    # 安装到可执行目录, BUILD_ID 在路径信息里, 名字不再包含 BUILD_ID
    set_target_properties(easeds-unittest PROPERTIES OUTPUT_NAME "easeds-unittest")
    install(TARGETS easeds-unittest DESTINATION ${EASEDS_EXEC_INSTALL_DIR})
endif() # BUILD_EASEDS_UNITTEST

# 添加选项, 编译静态库文件
option(BUILD_EASEDS_STATIC "Build EASEDS static library" ON)

# 如果开启了静态库编译选项, 或者动态库编译选项
if (BUILD_EASEDS_STATIC)
    # 编译静态链接库 - libeaseds.a
    add_library(easeds-static STATIC ${easeds_SRCS})

    # 添加编译选项
    target_compile_options(easeds-static PRIVATE
        -D_EASEDS_SHARED
        ${LIBASAN_FLAGS} # 如果使用了 AddressSanitizer
    )

    # 安装到静态链接库目录, BUILD_ID 在路径信息里, 名字不再包含 BUILD_ID
    set_target_properties(easeds-static PROPERTIES OUTPUT_NAME "easeds")
    install(TARGETS easeds-static DESTINATION ${EASEDS_LIB_INSTALL_DIR})
endif() # BUILD_EASEDS_STATIC

# 添加选项, 编译动态库文件
option(BUILD_EASEDS_SHARED "Build EASEDS shared library" ON)

# 如果开启了动态库编译选项
if (BUILD_EASEDS_SHARED)
    # 编译动态链接库 - libeaseds.so
    add_library(easeds-shared SHARED  ${easeds_SRCS})

    # 添加编译选项
    target_compile_options(easeds-shared PRIVATE
        -D_EASEDS_SHARED
        ${LIBASAN_FLAGS} # 如果使用了 AddressSanitizer
    )

    # 添加链接库
    target_link_libraries(easeds-shared PRIVATE
        ${easeds_LIBS}
    )

    # 设置链接参数
    target_link_options(easeds-shared PRIVATE
        ${easeds_LDFLAGS}
        ${LIBASAN_LINK_OPTION} # 如果使用了 AddressSanitizer
    )

    # 如果未定义EASEDS_VERSION, 默认为 0.0.0
    if(NOT DEFINED EASEDS_VERSION)
        set(EASEDS_VERSION 0.0.0)
    endif()

    # 如果未定义EASEDS_SO_VERSION, 默认为 0
    if(NOT DEFINED EASEDS_SO_VERSION)
        set(EASEDS_SO_VERSION 0)
    endif()
    Message(STATUS "EASEDS_VERSION = ${EASEDS_VERSION}")
    Message(STATUS "EASEDS_SO_VERSION = ${EASEDS_SO_VERSION}")

    # 安装到动态链接库目录, BUILD_ID 在路径信息里, 名字不再包含 BUILD_ID
    set_target_properties(easeds-shared PROPERTIES OUTPUT_NAME "easeds")
    set_target_properties(easeds-shared PROPERTIES VERSION ${EASEDS_VERSION} SOVERSION ${EASEDS_SO_VERSION})

    install(TARGETS easeds-shared DESTINATION ${EASEDS_LIB_INSTALL_DIR})
    install(DIRECTORY ${TOPDIR}/include/ DESTINATION ${EASEDS_HEADER_INSTALL_DIR})
endif() # BUILD_EASEDS_SHARED
