#!/bin/bash

# 开启shell调试
# set -x

# 当前目录路径
export SOURCE_DIR=${SOURCE_DIR:-$(pwd)}

# 导入基本shell工具函数
source $SOURCE_DIR/utils.sh

# 参数 rpm_build.sh <软件名> <构建架构> <构建产物目录>
# 例如: rpm_build.sh valgrind-3.21.0 X86_64 build/output/
if [ $# -lt 3 ]; then
    ERROR "Usage: $0 <software-name> <build-arch> <target-dir>"
    exit 1
fi

# 第一个参数是软件信息
if [ -n "$1" ]; then
    SOFTNAME=$1
    # 取出版本号
    SOURCE_VERSION=${SOFTNAME##*-}
    # 取出软件包名称
    SOURCE_NAME=${SOFTNAME%-*}
else
    ERROR "No sortware name specified, please specify it as the first argument."
    exit 1
fi
INFO "SOURCE_DIR: [$SOURCE_DIR]"
INFO "SOURCE_NAME: [$SOURCE_NAME]"
INFO "SOURCE_VERSION: [$SOURCE_VERSION]"

# 第二个参数是构建架构
if [ -n "$2" ]; then
    BUILD_ARCH=$2
else
    ERROR "No build architecture specified, please specify it as the second argument."
    exit 1
fi
INFO "BUILD_ARCH: [$BUILD_ARCH]"

# 第三个参数是构建产物目录
if [ -n "$3" ]; then
    TARGET_DIR=$3
    # 使用绝对路径
    TARGET_DIR=$(realpath $TARGET_DIR)
else
    ERROR "No build target directory specified, please specify it as the first argument."
    exit 1
fi
INFO "TARGET_DIR: [$TARGET_DIR]"

# 创建RPM包目录
RPM_DIR=${SOURCE_DIR}/build/rpmbuild

# 删除旧的RPM包目录
if [ -d "$RPM_DIR" ]; then
    rm -rf $RPM_DIR
    INFO "Delete Old RPM_DIR: [$RPM_DIR]"
fi

# 创建新的RPM包目录
if [ ! -d "$RPM_DIR" ]; then
    mkdir -p $RPM_DIR
fi
INFO "RPM_DIR: [$RPM_DIR]"

# 创建RPM包目录结构
RPM_BUILD_DIR=${RPM_DIR}/BUILD
if [ ! -d "$RPM_BUILD_DIR" ]; then
    mkdir -p $RPM_BUILD_DIR
fi
INFO "RPM_BUILD_DIR: [$RPM_BUILD_DIR]"

RPM_RPMS_DIR=${RPM_DIR}/RPMS
if [ ! -d "$RPM_RPMS_DIR" ]; then
    mkdir -p $RPM_RPMS_DIR
fi
INFO "RPM_RPMS_DIR: [$RPM_RPMS_DIR]"

RPM_SRCS_DIR=${RPM_DIR}/SOURCES
if [ ! -d "$RPM_SRCS_DIR" ]; then
    mkdir -p $RPM_SRCS_DIR
fi
INFO "RPM_SRCS_DIR: [$RPM_SRCS_DIR]"

RPM_SPECS_DIR=${RPM_DIR}/SPECS
if [ ! -d "$RPM_SPECS_DIR" ]; then
    mkdir -p $RPM_SPECS_DIR
fi
INFO "RPM_SPECS_DIR: [$RPM_SPECS_DIR]"

RPM_SRCS_DIR=${RPM_DIR}/SOURCES
if [ ! -d "$RPM_SRCS_DIR" ]; then
    mkdir -p $RPM_SRCS_DIR
fi
INFO "RPM_SRCS_DIR: [$RPM_SRCS_DIR]"

# 编写RPM spec文件
SPEC_FILE=${RPM_SPECS_DIR}/${SOURCE_NAME}.spec
rm -f $SPEC_FILE
INFO "Delete Old SPEC_FILE: [$SPEC_FILE]"

# 创建RPM spec文件
if [ ! -f "$SPEC_FILE" ]; then
    cat <<EOF >$SPEC_FILE
Name: $SOURCE_NAME
Version: $SOURCE_VERSION
Release: 1%{?dist}
Summary: $SOURCE_NAME RPM package
Group: Applications/System

%define __brp_strip_debug :
%define __strip /bin/true

License: Ruijie Network CSBU Private
URL: http://10.51.134.12:8080/ease/easeds/-/blob/libeaseds-2.5.0/LICENSE

%description
This package contains a pre-built binary of the $SOURCE_NAME program.

%build
# No build needed

%install
# Copy all extracted files into buildroot
cp -a $TARGET_DIR/* %{buildroot}/

%files
%defattr(-,root,root,-)
%dir /
/*

%changelog
* $(date +"%a %b %d %Y") Once Day <once_day@qq.com> - $SOURCE_VERSION-1
- Initial RPM package

EOF
fi

INFO "SPEC_FILE: [$SPEC_FILE]"

# 创建RPM源文件
rpmbuild --define "_topdir $RPM_DIR" --define "_binary_payload w0T8.xzdio" \
    -bb ${SPEC_FILE} --target $BUILD_ARCH

# 输出在 RPM_RPMS_DIR/BUILD_ARCH 目录下
INFO "RPM package created in: $RPM_RPMS_DIR/$BUILD_ARCH"

# 拷贝到 $TARGET_DIR 目录下
if [ -d "$RPM_RPMS_DIR/$BUILD_ARCH" ]; then
    cp -a $RPM_RPMS_DIR/$BUILD_ARCH/*.rpm $TARGET_DIR/
    INFO "Copy RPM package to target directory: $TARGET_DIR"
else
    ERROR "No RPM package found in: $RPM_RPMS_DIR/$BUILD_ARCH"
    exit 1
fi

INFO "RPM build completed successfully!"
exit 0
