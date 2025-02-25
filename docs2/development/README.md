# 开发指南

## 概述

本文档提供智能巡检系统的开发指南，包括环境搭建、编译构建、代码规范和测试规范等内容。开发人员应该仔细阅读并遵循这些指南，以确保代码质量和项目可维护性。

## 目录

1. [编译构建](build.md)
   - 环境要求
   - 依赖安装
   - 编译步骤
   - 常见问题

2. [代码规范](coding_standard.md)
   - 命名规范
   - 格式规范
   - 注释规范
   - 最佳实践

3. [测试规范](testing.md)
   - 单元测试
   - 集成测试
   - 性能测试
   - 测试覆盖率

## 开发环境

### 1. 系统要求

- 操作系统：Ubuntu 20.04 或更高版本
- 编译器：GCC 9+ 或 Clang 10+
- CMake：3.15 或更高版本
- Git：2.25 或更高版本

### 2. 依赖工具

```bash
# 基础开发工具
sudo apt update
sudo apt install -y build-essential cmake git

# 开发库
sudo apt install -y \
    libboost-all-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libjsoncpp-dev \
    libspdlog-dev

# 代码质量工具
sudo apt install -y \
    clang-format \
    clang-tidy \
    cppcheck \
    valgrind
```

### 3. IDE 配置

推荐使用 Visual Studio Code，需要安装以下插件：

- C/C++
- CMake
- CMake Tools
- clang-format
- Code Runner
- Git Graph
- GitLens

## 开发流程

### 1. 代码管理

```bash
# 克隆仓库
git clone https://github.com/your-org/intelligent-inspection.git
cd intelligent-inspection

# 创建功能分支
git checkout -b feature/your-feature

# 提交代码
git add .
git commit -m "feat: add your feature"
git push origin feature/your-feature
```

### 2. 构建步骤

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译项目
make -j$(nproc)

# 运行测试
ctest --output-on-failure
```

### 3. 代码审查

1. 提交前自查：
   - 代码格式化
   - 静态检查
   - 单元测试
   - 文档更新

2. 提交合并请求：
   - 清晰的标题和描述
   - 完整的测试用例
   - 相关文档更新
   - 符合代码规范

## 版本管理

### 1. 分支策略

- `main`: 主分支，保持稳定
- `develop`: 开发分支
- `feature/*`: 功能分支
- `bugfix/*`: 修复分支
- `release/*`: 发布分支

### 2. 版本号规范

采用语义化版本号：`主版本号.次版本号.修订号`

- 主版本号：不兼容的API修改
- 次版本号：向下兼容的功能性新增
- 修订号：向下兼容的问题修正

### 3. 提交信息规范

```
<type>(<scope>): <subject>

<body>

<footer>
```

类型（type）：
- feat：新功能
- fix：修复
- docs：文档
- style：格式
- refactor：重构
- test：测试
- chore：构建

## 调试技巧

### 1. 日志调试

```cpp
// 设置日志级别
Logger::getInstance().setLevel(LogLevel::DEBUG);

// 添加日志
LOG_DEBUG("Variable value: {}", value);
LOG_INFO("Processing task: {}", task_id);
LOG_ERROR("Failed to connect: {}", error_msg);
```

### 2. GDB调试

```bash
# 编译带调试信息的版本
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# 启动调试
gdb ./bin/inspection_system

# 常用GDB命令
(gdb) break main
(gdb) run
(gdb) next
(gdb) step
(gdb) print variable
(gdb) backtrace
```

### 3. 性能分析

```bash
# 使用 Valgrind 进行内存检查
valgrind --leak-check=full ./bin/inspection_system

# 使用 perf 进行性能分析
perf record ./bin/inspection_system
perf report
```

## 常见问题

### 1. 编译问题

- 检查依赖是否完整
- 确认CMake版本
- 查看编译错误日志
- 清理构建目录重试

### 2. 运行问题

- 检查配置文件
- 查看系统日志
- 确认权限设置
- 检查网络连接

### 3. 性能问题

- 使用性能分析工具
- 检查内存泄漏
- 优化算法实现
- 调整配置参数

## 参考资源

1. 官方文档
   - [C++ 参考手册](https://en.cppreference.com/)
   - [CMake 文档](https://cmake.org/documentation/)
   - [Boost 文档](https://www.boost.org/doc/)

2. 开发工具
   - [GDB 文档](https://sourceware.org/gdb/documentation/)
   - [Valgrind 手册](https://valgrind.org/docs/)
   - [Perf 教程](https://perf.wiki.kernel.org/)

3. 编码规范
   - [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
   - [Modern C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
