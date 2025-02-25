# 机器狗导航控制系统

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/std/the-standard)
[![Boost](https://img.shields.io/badge/Boost-1.65%2B-orange.svg)](https://www.boost.org/)
[![CMake](https://img.shields.io/badge/CMake-3.15%2B-brightgreen.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

## 项目简介

机器狗导航控制系统是一个专为四足机器人设计的自主导航控制程序，使用C++17实现，基于状态机架构。系统通过TCP网络与机器人硬件进行通信，实现路径规划和导航控制。

### 主要特性

- **高效状态管理**：基于Boost.MSM实现的状态机，可靠的状态转换和事件处理
- **多线程架构**：分离的UI线程、处理线程和网络线程，确保实时响应
- **灵活网络层**：支持Boost.Asio和原生Epoll两种网络模型
- **优雅的消息处理**：基于事件总线和消息队列的解耦设计

## 快速开始

### 环境要求

- C++17兼容编译器（GCC 7+/Clang 5+/MSVC 19.14+）
- CMake 3.15 或更高版本
- Boost库 1.65 或更高版本

### 构建与运行

```bash
# 克隆仓库
git clone https://github.com/yourusername/robot-dog-navigation.git
cd robot-dog-navigation

# 构建项目
mkdir build && cd build
cmake ..
make -j$(nproc)

# 运行程序
./bin/robot_nav
```

## 文档

更详细的文档请参考 [docs/](docs/) 目录：

- [架构设计](docs/architecture.md)
- [用户手册](docs/user_manual.md)
- [开发指南](docs/developer_guide.md)
- [API文档](docs/api.md)

## 开发状态

当前版本：v0.1.0 (MVP)

- [x] 基础框架搭建
- [x] 状态机实现
- [x] 网络层框架
- [ ] 完整的单元测试
- [ ] 性能测试和优化

## 贡献代码

欢迎提交Issue和Pull Request。详细的贡献指南请参考[贡献指南](docs/contributing.md)。

## 许可证

本项目采用MIT许可证。详情请参阅[LICENSE](LICENSE)文件。
