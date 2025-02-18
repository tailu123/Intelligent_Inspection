# X30机器狗巡检系统

## 项目概述
本项目是基于X30机器狗平台开发的智能巡检系统，通过TCP通信实现对机器狗的导航控制和状态监控。

## 系统架构
系统采用分层架构设计，主要包含以下几层：

1. **应用层 (Application Layer)**
   - 业务逻辑处理
   - 巡检任务管理
   - 状态监控和报警

2. **状态管理层 (State Management Layer)**
   - 使用Boost::FSM实现状态机
   - 管理机器狗运行状态
   - 处理状态转换和事件

3. **通信层 (Communication Layer)**
   - 基于Boost::Asio的异步通信
   - XML消息的序列化和反序列化
   - TCP连接管理

4. **协议层 (Protocol Layer)**
   - XML协议解析
   - 消息封装和解析
   - 错误码处理

## 关键特性
- 异步通信：使用Boost::Asio实现高效的异步通信
- 状态管理：使用Boost::FSM实现可靠的状态转换
- 可扩展性：模块化设计，便于扩展新功能
- 错误处理：完整的错误码体系和异常处理机制

## 主要功能
1. 导航任务下发（Type: 1003）
2. 导航任务取消（Type: 1004）
3. 设备状态查询（Type: 1007）

## 技术栈
- 编程语言：C++17
- 网络库：Boost::Asio
- 状态管理：Boost::FSM
- XML解析：RapidXML
- 构建工具：CMake

## 目录结构
```
├── src/
│   ├── application/    # 应用层代码
│   ├── state/         # 状态管理相关代码
│   ├── communication/ # 通信相关代码
│   ├── protocol/      # 协议相关代码
│   └── utils/         # 工具类
├── include/           # 头文件
├── test/             # 单元测试
├── docs/             # 文档
└── examples/         # 示例代码
```