# X30巡检系统

## 项目简介

X30巡检系统是一个基于C++17开发的智能巡检控制系统，采用分层架构设计，提供可靠的导航控制、任务管理和状态监控功能。系统通过TCP协议与巡检设备进行通信，支持实时状态反馈和异步事件处理。

### 核心特性
- 🚀 高性能异步通信
- 🛡️ 可靠的状态管理
- 🔌 灵活的插件系统
- 📦 模块化设计

## 功能特点

### 1. 导航控制
- 支持多点导航任务规划
- 实时状态反馈和监控
- 可配置的导航参数
  - 速度控制
  - 避障模式
  - 导航模式
  - 地形适应

### 2. 任务管理
- 任务控制
  - 启动巡检任务
  - 取消当前任务
  - 暂停/恢复功能
- 状态查询
  - 实时位置信息
  - 任务执行状态
  - 设备工作状态
- 异常处理
  - 自动错误恢复
  - 故障报警机制
  - 应急处理流程

### 3. 系统配置
- JSON格式配置文件
- 动态参数加载
- 配置项验证机制
- 默认值支持

### 4. 通信协议
- 基于TCP的可靠通信
- XML格式消息协议
- 支持的消息类型：
  - 导航任务消息 (Type: 1003)
  - 取消任务消息 (Type: 1004)
  - 状态查询消息 (Type: 1007)

## 系统架构

系统采用四层架构设计：
1. 应用层：用户交互和任务控制
2. 业务逻辑层：核心业务处理
3. 通信层：网络通信实现
4. 数据层：数据管理和存储

详细架构说明请参考 [架构文档](docs/architecture.md)

## 开发环境

### 依赖项
- C++17或更高版本
- CMake 3.10+
- Boost库
  - asio
  - system
  - filesystem
  - thread
- nlohmann/json
- RapidXML

### 支持的平台
- Linux (Ubuntu 18.04+)
- macOS (10.15+)
- Windows (计划中)

## 快速开始

### 编译安装

```bash
# 克隆仓库
git clone https://github.com/your-username/x30_inspection_system.git
cd x30_inspection_system

# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
make -j$(nproc)

# 运行测试（可选）
make test
```

### 运行程序

```bash
# 运行主程序
./x30_inspection_system <host> <port>

# 运行测试服务器（用于开发测试）
./tcp_server <port>
```

## 配置说明

### 导航点配置
配置文件位置：`config/default_params.json`

```json
{
    "MapID": 0,
    "Value": 1,
    "PosX": 0.0,
    "PosY": 0.0,
    "PosZ": 0.0,
    "AngleYaw": 0.0,
    "Speed": 1,
    "NavMode": 0
}
```

### 命令列表
- `start`: 开始巡检任务
- `cancel`: 取消当前任务
- `status`: 查询系统状态
- `help`: 显示帮助信息
- `quit`: 退出程序

## 项目结构

```
Intelligent_Inspection/
├── include/          # 头文件
│   ├── application/  # 应用层
│   ├── communication/# 通信层
│   ├── protocol/     # 协议层
│   └── state/        # 状态管理
├── src/             # 源文件
├── config/          # 配置文件
├── docs/            # 文档
│   └── architecture.md
├── tests/           # 测试文件
├── examples/        # 示例代码
└── CMakeLists.txt
```

## 开发指南

### 代码规范
- 遵循C++17标准
- 使用Google C++代码风格
- 必要的注释和文档
- 单元测试覆盖

### 贡献指南
1. Fork 项目
2. 创建特性分支
3. 提交变更
4. 推送到分支
5. 创建Pull Request

## 常见问题

### 问题排查
1. 连接问题
   - 检查网络连接
   - 验证IP和端口
   - 查看防火墙设置

2. 配置问题
   - 确认配置文件格式
   - 检查参数有效性
   - 查看日志输出

## 版本历史

- v1.0.0 (2024-01)
  - 初始版本发布
  - 基本功能实现
  - 支持Linux平台

## 开发团队

- 开发者：[开发团队]
- 邮箱：[联系邮箱]
- 项目主页：[项目地址]

## 许可证

本项目采用 [许可证类型] 许可证 - 详见 [LICENSE](LICENSE) 文件

## 致谢

感谢所有贡献者对项目的支持！