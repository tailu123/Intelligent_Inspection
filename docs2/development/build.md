# 编译构建指南

## 环境要求

### 1. 操作系统

- Ubuntu 20.04 LTS 或更高版本（推荐）
- CentOS 7/8
- Debian 10 或更高版本

### 2. 编译工具

- GCC 9+ 或 Clang 10+
- CMake 3.15+
- Make 或 Ninja
- Git 2.25+

### 3. 系统资源

- CPU：4核或更多
- 内存：8GB或更多
- 磁盘空间：10GB可用空间

## 依赖安装

### 1. 基础开发工具

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    git \
    ninja-build \
    pkg-config

# CentOS
sudo yum groupinstall "Development Tools"
sudo yum install -y \
    cmake3 \
    git \
    ninja-build \
    pkg-config
```

### 2. 必要依赖库

```bash
# Ubuntu/Debian
sudo apt install -y \
    libboost-all-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libjsoncpp-dev \
    libspdlog-dev \
    libfmt-dev \
    libssl-dev

# CentOS
sudo yum install -y \
    boost-devel \
    protobuf-devel \
    protobuf-compiler \
    jsoncpp-devel \
    spdlog-devel \
    fmt-devel \
    openssl-devel
```

### 3. 可选依赖

```bash
# Ubuntu/Debian
sudo apt install -y \
    doxygen \
    graphviz \
    ccache \
    clang-format \
    clang-tidy

# CentOS
sudo yum install -y \
    doxygen \
    graphviz \
    ccache \
    clang-tools-extra
```

## 获取源码

```bash
# 克隆仓库
git clone https://github.com/your-org/intelligent-inspection.git
cd intelligent-inspection

# 更新子模块
git submodule update --init --recursive
```

## 编译步骤

### 1. 基本编译

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
make -j$(nproc)
```

### 2. 自定义编译

```bash
# Debug版本
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release版本
cmake -DCMAKE_BUILD_TYPE=Release ..

# 使用Ninja构建系统
cmake -GNinja ..
ninja

# 指定安装路径
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
```

### 3. 编译选项

```bash
# 启用测试
cmake -DBUILD_TESTING=ON ..

# 启用文档生成
cmake -DBUILD_DOCUMENTATION=ON ..

# 启用代码覆盖率
cmake -DENABLE_COVERAGE=ON ..

# 使用clang编译器
CC=clang CXX=clang++ cmake ..
```

## 安装

```bash
# 安装到系统
sudo make install

# 指定安装路径
make DESTDIR=/path/to/install install
```

## 运行测试

```bash
# 运行所有测试
ctest --output-on-failure

# 运行特定测试
ctest -R test_name

# 并行运行测试
ctest -j$(nproc)

# 生成测试覆盖率报告
make coverage
```

## 生成文档

```bash
# 生成API文档
make doc

# 生成PDF文档
make doc-pdf
```

## 清理构建

```bash
# 清理构建文件
make clean

# 完全清理
rm -rf build/
```

## CMake选项说明

| 选项 | 说明 | 默认值 |
|------|------|--------|
| BUILD_TESTING | 启用单元测试 | OFF |
| BUILD_DOCUMENTATION | 生成文档 | OFF |
| ENABLE_COVERAGE | 启用代码覆盖率 | OFF |
| USE_SYSTEM_BOOST | 使用系统Boost库 | ON |
| ENABLE_SANITIZERS | 启用地址检查 | OFF |

## 常见问题

### 1. 编译错误

问题：找不到依赖库
```
CMake Error: Could not find Boost
```

解决：
```bash
sudo apt install libboost-all-dev
# 或
sudo yum install boost-devel
```

### 2. 链接错误

问题：符号未定义
```
undefined reference to `boost::system::system_category()'
```

解决：
```bash
# 检查CMakeLists.txt中的链接设置
target_link_libraries(${PROJECT_NAME}
    PRIVATE Boost::system
)
```

### 3. 性能问题

问题：编译速度慢

解决：
```bash
# 使用ccache
sudo apt install ccache
export CC="ccache gcc"
export CXX="ccache g++"

# 使用并行编译
make -j$(nproc)

# 使用Ninja构建系统
cmake -GNinja ..
ninja
```

## 构建脚本

### 1. 快速构建脚本

```bash
#!/bin/bash
set -e

# 创建构建目录
mkdir -p build && cd build

# 配置项目
cmake -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_TESTING=ON \
      -DENABLE_COVERAGE=OFF \
      ..

# 编译
make -j$(nproc)

# 运行测试
ctest --output-on-failure
```

### 2. 完整构建脚本

```bash
#!/bin/bash
set -e

# 清理旧的构建
rm -rf build
mkdir build && cd build

# 配置项目
cmake -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_TESTING=ON \
      -DBUILD_DOCUMENTATION=ON \
      -DENABLE_COVERAGE=ON \
      -DUSE_SYSTEM_BOOST=ON \
      ..

# 编译
make -j$(nproc)

# 运行测试
ctest --output-on-failure

# 生成文档
make doc

# 生成覆盖率报告
make coverage

# 安装
sudo make install
```

## 持续集成

### 1. Jenkins配置

```groovy
pipeline {
    agent any
    stages {
        stage('Build') {
            steps {
                sh '''
                    mkdir -p build
                    cd build
                    cmake ..
                    make -j$(nproc)
                '''
            }
        }
        stage('Test') {
            steps {
                sh '''
                    cd build
                    ctest --output-on-failure
                '''
            }
        }
    }
}
```

### 2. GitHub Actions配置

```yaml
name: CI

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    - name: Configure
      run: cmake -B build
    - name: Build
      run: cmake --build build -j$(nproc)
    - name: Test
      run: cd build && ctest --output-on-failure
```
