#!/bin/bash

# 检测系统类型
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
    VER=$VERSION_ID
else
    echo "无法检测操作系统类型"
    exit 1
fi

# 安装依赖
install_ubuntu() {
    echo "正在安装 Ubuntu 依赖..."
    sudo apt-get update
    sudo apt-get install -y \
        libboost-system-dev \
        libboost-filesystem-dev \
        libboost-thread-dev \
        libspdlog-dev \
        nlohmann-json3-dev

    # RapidXML 可能需要手动安装
    if ! dpkg -l | grep -q rapidxml; then
        echo "正在安装 RapidXML..."
        TEMP_DIR=$(mktemp -d)
        cd $TEMP_DIR
        wget https://downloads.sourceforge.net/project/rapidxml/rapidxml/rapidxml%201.13/rapidxml-1.13.zip
        unzip rapidxml-1.13.zip
        sudo mkdir -p /usr/include/rapidxml
        sudo cp rapidxml-1.13/*.hpp /usr/include/rapidxml/
        cd - > /dev/null
        rm -rf $TEMP_DIR
    fi
}

install_centos() {
    echo "正在安装 CentOS 依赖..."
    sudo yum install -y epel-release
    sudo yum install -y \
        boost-devel \
        spdlog-devel \
        nlohmann-json-devel

    # RapidXML 可能需要手动安装
    if ! rpm -q rapidxml > /dev/null 2>&1; then
        echo "正在安装 RapidXML..."
        TEMP_DIR=$(mktemp -d)
        cd $TEMP_DIR
        wget https://downloads.sourceforge.net/project/rapidxml/rapidxml/rapidxml%201.13/rapidxml-1.13.zip
        unzip rapidxml-1.13.zip
        sudo mkdir -p /usr/include/rapidxml
        sudo cp rapidxml-1.13/*.hpp /usr/include/rapidxml/
        cd - > /dev/null
        rm -rf $TEMP_DIR
    fi
}

# 根据系统类型安装依赖
case "$OS" in
    "Ubuntu")
        install_ubuntu
        ;;
    "CentOS Linux")
        install_centos
        ;;
    *)
        echo "不支持的操作系统: $OS"
        exit 1
        ;;
esac

echo "依赖安装完成！"
echo "您可以通过运行 'mkdir build && cd build && cmake ..' 来验证安装"
