#!/bin/bash

# 开启调试
# set -x
# 关闭调试
# set +x

# 当前目录路径
export SOURCE_DIR=${SOURCE_DIR:-$(pwd)}

# 导入基本ANMK shell工具函数
source $SOURCE_DIR/utils.sh

# 导入版本信息
source $SOURCE_DIR/version.sh
INFO "EASEDS Version: $EASEDS_VERSION"

# 额外参数
if [ $# -gt 0 ]; then
    CMAKE_EX_ARGS="$*"
else
    # 默认编译选项, 只编译单元测试, 不编译共享库和MINI实例
    CMAKE_EX_ARGS="-DBUILD_EASEDS_UNITTEST=ON -DBUILD_EASEDS_SHARED=ON"
fi
INFO "CMAKE_EX_ARGS: [$CMAKE_EX_ARGS]"

# 指定默认编译器
CC=${CC:-gcc}
INFO "CC: [$CC]"

# 指定编译类型 - debug or release or coverage or clang-scan
BUILD_TYPE=${BUILD_TYPE:-debug}

# 检查编译类型是否正确
case $BUILD_TYPE in
debug | release | min-size | coverage | clang-scan | libasan | clang)
    INFO "BUILD_TYPE: [$BUILD_TYPE]"
    ;;
*)
    ERROR "Invalid BUILD_TYPE: $BUILD_TYPE"
    ERROR "Usage: BUILD_TYPE=debug|release|min-size|coverage|clang-scan|libasan|clang ./build.sh"
    exit 1
    ;;
esac

# 如果是 clang 编译, 则重定向CC为clang
if [ "$BUILD_TYPE" = "clang" ]; then
    CC=clang
    INFO "Change CC to [$CC]"
fi

# 从编译器 CC -v 获取目标架构信息 Target: aarch64-onceday-linux-gnu
COMPILE_VERBOSE=$($CC -v 2>&1)
INFO "COMPILE_VERBOSE: \n$COMPILE_VERBOSE"

# 提取目标架构信息 - aarch64-onceday-linux-gnu or x86_64-linux-gnu or ...
# 使用引号包裹命令, 保留换行符信息
TARGET_ARCH=$(echo "$COMPILE_VERBOSE" | grep "Target:" | awk '{print $2}')
INFO "Compiler Target: [$TARGET_ARCH]"

# 只需要架构信息 - x86_64 or aarch64
TARGET_ARCH=${TARGET_ARCH%%-*}

# 检查架构类型是否正确
case $TARGET_ARCH in
x86_64 | aarch64)
    INFO "TARGET_ARCH: [$TARGET_ARCH]"
    ;;
*)
    ERROR "Invalid TARGET_ARCH: $TARGET_ARCH"
    ERROR "Usage: TARGET_ARCH=x86_64|aarch64 ./build.sh"
    exit 1
    ;;
esac

# 指定默认编译后缀 - local or 具体产品型号
BUILD_SUFFIX=${BUILD_SUFFIX:-local}

# 指定编译标识符, 组成: BUILD_TYPE-TARGET_ARCH-BUILD_SUFFIX
BUILD_ID=${BUILD_TYPE}-${TARGET_ARCH}-${BUILD_SUFFIX}

# 指定默认编译目录: build/debug-x86_64
BUILD_DIR=${BUILD_DIR:-${SOURCE_DIR}/build/${BUILD_ID}}
mkdir -p $BUILD_DIR
INFO "BUILD_DIR: [$BUILD_DIR]"

# 指定默认输出目录: build/output/debug-x86_64
OUTPUT_DIR=${OUTPUT_DIR:-${SOURCE_DIR}/build/output/${BUILD_ID}}
mkdir -p $OUTPUT_DIR
INFO "OUTPUT_DIR: [$OUTPUT_DIR]"

# 指定默认安装目录: build/install/debug-x86_64
INSTALL_DIR=${INSTALL_DIR:-${OUTPUT_DIR}}
mkdir -p $INSTALL_DIR
INFO "INSTALL_DIR: [$INSTALL_DIR]"

# 创建编译命令描述文件的符号链接
COMPILE_INFO_SRC=$BUILD_DIR/compile_commands.json
COMPILE_INFO_DST=$SOURCE_DIR/compile-commands-${BUILD_ID}.json
ln -sf $COMPILE_INFO_SRC $COMPILE_INFO_DST
INFO "Create symbol link: $(basename $COMPILE_INFO_DST)"

# 创建编译链接描述文件的符号链接
LINK_INFO_SRC=$BUILD_DIR/CMakeFiles/easeds-shared.dir/link.txt
LINK_INFO_DST=$SOURCE_DIR/compile-link-${BUILD_ID}.txt
ln -sf $LINK_INFO_SRC $LINK_INFO_DST
INFO "Create symbol link: $(basename $LINK_INFO_DST)"

# 判断是否开启gcov编译
if [ "$BUILD_TYPE" = "coverage" ]; then
    GCOV=ON
    INFO "GCOV: [$GCOV]"
    # 修改编译类型,coverage编译只能是debug
    INFO "Change BUILD_TYPE from [$BUILD_TYPE] to [debug]"
    BUILD_TYPE=debug
    # COVERAGE_DIR默认为当前输出目录下的coverage
    COVERAGE_DIR=${COVERAGE_DIR:-$(OUTPUT_DIR)/coverage}
    INFO "COVERAGE_DIR: [$COVERAGE_DIR]"
    # 创建COVERAGE_DIR目录
    mkdir -p $COVERAGE_DIR
    # 清除COVERAGE_DIR目录
    rm $COVERAGE_DIR/* -rf
    INFO "Delete COVERAGE_DIR: [$COVERAGE_DIR]"
    # 清除编译目录
    rm $BUILD_DIR/* -rf
    INFO "Delete BUILD_DIR: [$BUILD_DIR]"
else
    GCOV=OFF
    INFO "GCOV: [$GCOV]"
fi

# 判断是否为libasan编译
if [ "$BUILD_TYPE" = "libasan" ]; then
    # libasan 为debug编译
    LIBASAN=ON
    INFO "LIBASAN: [$LIBASAN]"
    # 修改编译类型, libasan编译只能是debug
    INFO "Change BUILD_TYPE from [$BUILD_TYPE] to [debug]"
    BUILD_TYPE=debug
    # 添加编译选项, 保留栈指针, 允许快速栈回溯
    LIBASAN_FLAGS="-g -fsanitize=address -fno-omit-frame-pointer"
    # 保留堆栈信息, 不将C中全局变量视为公共变量, 允许ASAN检测
    LIBASAN_FLAGS="$LIBASAN_FLAGS -fno-optimize-sibling-calls -fno-common"
    # 允许发生错误时继续允许LIBASAN
    LIBASAN_FLAGS="$LIBASAN_FLAGS -fsanitize-recover=address"
    INFO "LIBASAN_FLAGS: [$LIBASAN_FLAGS]"
    # 添加链接选项, 允许libasan.a链接
    LIBASAN_LDFLAGS="-Wl,-Bstatic -lasan -Wl,-Bdynamic -lm -ldl"
    INFO "LIBASAN_LDFLAGS: [$LIBASAN_LDFLAGS]"
else
    LIBASAN=OFF
    INFO "LIBASAN: [$LIBASAN]"
fi

# 默认第三方库的路径是 library 目录
LIBRARY_DIR=${LIBRARY_DIR:-$SOURCE_DIR/library}
INFO "LIBRARY_DIR: [$LIBRARY_DIR]"

# 指定CMAKE编译选项
CMAKE_ARGS="-DCMAKE_C_COMPILER=$CC -DCMAKE_C_FLAGS='$CFLAGS'"
CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_EXE_LINKER_FLAGS='$LDFLAGS'"
CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_SHARED_LINKER_FLAGS='$LDFLAGS'"
CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_BUILD_TYPE=$BUILD_TYPE"
CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR"
CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
CMAKE_ARGS="$CMAKE_ARGS -DCOVERAGE=$GCOV"
CMAKE_ARGS="$CMAKE_ARGS -DTARGET_ARCH=$TARGET_ARCH"
CMAKE_ARGS="$CMAKE_ARGS -DBUILD_ID=$BUILD_ID"
CMAKE_ARGS="$CMAKE_ARGS -DLIBASAN=$LIBASAN"
CMAKE_ARGS="$CMAKE_ARGS -DLIBRARY_DIR=$LIBRARY_DIR"
CMAKE_ARGS="$CMAKE_ARGS -DEASEDS_MAJOR_VERSION=$EASEDS_MAJOR_VERSION"
CMAKE_ARGS="$CMAKE_ARGS -DEASEDS_MINOR_VERSION=$EASEDS_MINOR_VERSION"
CMAKE_ARGS="$CMAKE_ARGS -DEASEDS_PATCH_VERSION=$EASEDS_PATCH_VERSION"
CMAKE_ARGS="$CMAKE_ARGS -DBUILD_TIME=$BUILD_TIME"
CMAKE_ARGS="$CMAKE_ARGS $CMAKE_EX_ARGS"
INFO "CMAKE_ARGS: [$CMAKE_ARGS]"

CMAKE=${CMAKE:-cmake}

# 创建编译目录并且进入编译目录执行cmake
mkdir -p $BUILD_DIR && cd $BUILD_DIR &&
    eval cmake $CMAKE_ARGS "$SOURCE_DIR" &&
    make V=1 -j$(nproc) $TARGET && make install

# 保存make命令的返回值
return_code=$?

# 如果是root用户, 将 OUTPUT_DIR 目录权限设置为 0777
if [ $(id -u) -eq 0 ]; then
    INFO "Change OUTPUT_DIR permission to 0777"
    chmod -R 0777 $OUTPUT_DIR
fi

if [ $return_code -ne 0 ]; then
    ERROR "$BUILD_TYPE build failed, return code: $return_code"
    exit $return_code
fi

# 处理覆盖率测试的gcov文件
if [ "$GCOV" = "ON" ]; then
    # 拷贝所有的gcno到 COVERAGE_DIR 目录
    INFO "Copy all *.gcno to $COVERAGE_DIR"
    find $BUILD_DIR -name "*.gcno" -exec cp -v {} $COVERAGE_DIR \;
fi

# 对于动态库编译, 需要检查导出符号情况
if [ "$CHECK_SYMBOLS" = "ON" ]; then
    # 检查导出符号情况
    INFO "Check exported symbols for shared library"
    $SOURCE_DIR/check_api.py $SOURCE_DIR/libeaseds++.map $INSTALL_DIR/usr/lib/libeaseds++.so
    if [ $? -ne 0 ]; then
        ERROR "Check exported symbols failed!"
        exit 1
    else
        INFO "Check exported symbols passed!"
    fi
fi

# 如果开启了STRIP调试符号, 则删除调试符号
if [ "$STRIP_DEBUG_SYMBOLS" == "ON" ]; then
    # 删除调试符号
    INFO "Strip debug symbols in $INSTALL_DIR"
    STRIP=${STRIP:-strip}
    INFO "Use STRIP command: [$STRIP]"
    # find $INSTALL_DIR -type f -name "*.so*" -exec $STRIP -v --strip-debug {} \;
    find $INSTALL_DIR -type f -name "easeds-unittest" -exec $STRIP -v --strip-debug {} \;
    INFO "Strip debug symbols done!"
fi

# 对于产品编译, 需要RPM打包构建产物
if [ "$SUPPORT_RPM" == "ON" ]; then
    # 清除旧的RPM包
    INFO "Clean old RPM packages in $OUTPUT_DIR"
    rm -v -f $OUTPUT_DIR/*.rpm
    # 部分环境下只打包动态库, 其余文件全部清除
    if [ "$RPM_ONLY_LIBEASEDS" == "ON" ]; then
        INFO "RPM_ONLY_LIBEASEDS is ON, remove other files except libeaseds.so*"
        rm -f -r $OUTPUT_DIR/etc
        INFO "Remove $OUTPUT_DIR/etc"
        rm -f -r $OUTPUT_DIR/usr/bin
        INFO "Remove $OUTPUT_DIR/usr/bin"
    fi

    # 执行 rpm_build.sh 脚本, 打包构建产物
    INFO "Support RPM build, start to build RPM package"
    $SOURCE_DIR/rpm_build.sh easeds-$EASEDS_VERSION $TARGET_ARCH $OUTPUT_DIR
    # 如果开启了打包RPM, 则删除动态库文件, 避免上传文件过大
    rm -v -f $OUTPUT_DIR/usr/lib/*
    INFO "Delete dynamic library files in $OUTPUT_DIR/usr/lib/"
fi

# 执行完毕
INFO "$BUILD_ID build done!"
exit 0
