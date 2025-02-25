# 机器狗导航控制系统 - 开发者指南

## 1. 开发环境设置

本文档提供机器狗导航控制系统开发所需的环境配置、代码规范和开发流程指南。

### 1.1 开发环境要求

- **操作系统**：
  - Linux (首选): Ubuntu 20.04 LTS 或更高版本
  - MacOS: 10.15 或更高版本
  - Windows: 仅通过WSL2支持

- **编译器**：
  - GCC 8.0+ (Linux)
  - Clang 10.0+ (MacOS/Linux)
  - MSVC 19.20+ (Windows)

- **构建工具**：
  - CMake 3.15+
  - Ninja (可选但推荐)

- **依赖工具**：
  - Git 2.20+
  - Python 3.6+ (用于脚本)
  - Doxygen 1.8+ (文档生成)

### 1.2 环境设置步骤

#### Linux (Ubuntu)

```bash
# 安装必要工具
sudo apt update
sudo apt install -y build-essential git cmake ninja-build
sudo apt install -y doxygen python3 python3-pip
sudo apt install -y libboost-all-dev libssl-dev

# 克隆代码库
git clone https://github.com/yourusername/robot-dog-navigation.git
cd robot-dog-navigation

# 设置开发环境
python3 scripts/setup_env.py
```

#### MacOS

```bash
# 安装Homebrew (如果尚未安装)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装必要工具
brew install cmake ninja boost openssl doxygen
brew install python

# 克隆代码库
git clone https://github.com/yourusername/robot-dog-navigation.git
cd robot-dog-navigation

# 设置开发环境
python3 scripts/setup_env.py
```

## 2. 项目结构说明

```
robot-dog-navigation/
├── include/                    # 头文件
│   ├── app/                   # 应用层头文件
│   │   ├── cli/               # 命令行界面
│   │   ├── config/            # 配置管理
│   │   └── log/               # 日志系统
│   ├── procedure/            # 业务层头文件
│   │   ├── state_machine/     # 状态机实现
│   │   ├── nav/               # 导航逻辑
│   │   ├── event/             # 事件系统
│   │   └── message/           # 消息定义
│   └── network/              # 网络层头文件
│       ├── tcp/               # TCP通信
│       ├── protocol/          # 协议实现
│       └── serialization/     # 序列化
├── src/                       # 源文件
│   ├── app/                   # 应用层实现
│   ├── procedure/            # 业务层实现
│   └── network/              # 网络层实现
├── test/                      # 测试文件
│   ├── unit/                  # 单元测试
│   ├── integration/           # 集成测试
│   └── mock/                  # 模拟对象
├── docs/                      # 文档
│   ├── api/                   # API文档
│   ├── architecture/          # 架构文档
│   └── diagrams/              # 图表和图示
├── scripts/                   # 辅助脚本
│   ├── build/                 # 构建脚本
│   ├── setup/                 # 环境设置脚本
│   └── tools/                 # 开发工具
├── third_party/              # 第三方库
├── examples/                 # 示例代码
│   ├── simple_nav/            # 简单导航示例
│   └── custom_path/           # 自定义路径示例
├── .github/                  # GitHub配置
│   └── workflows/             # CI/CD工作流
├── CMakeLists.txt            # 主CMake文件
├── README.md                 # 项目概述
└── LICENSE                   # 许可证文件
```

## 3. 构建与测试

### 3.1 构建流程

```bash
# 进入项目目录
cd robot-dog-navigation

# 创建构建目录
mkdir -p build && cd build

# 配置项目 (Debug模式)
cmake -DCMAKE_BUILD_TYPE=Debug -GNinja ..

# 构建项目
ninja

# 或者使用make (如果不使用Ninja)
# cmake -DCMAKE_BUILD_TYPE=Debug ..
# make -j$(nproc)
```

### 3.2 构建选项

| 选项 | 说明 | 默认值 |
|-----|------|-------|
| `CMAKE_BUILD_TYPE` | 构建类型(Debug/Release/RelWithDebInfo) | `Debug` |
| `ENABLE_TESTING` | 启用测试构建 | `ON` |
| `ENABLE_COVERAGE` | 启用代码覆盖率工具 | `OFF` |
| `USE_ASAN` | 启用地址消毒器(Address Sanitizer) | `OFF` |
| `USE_TSAN` | 启用线程消毒器(Thread Sanitizer) | `OFF` |
| `USE_STATIC_BOOST` | 使用Boost静态库 | `OFF` |
| `NETWORK_IMPL` | 网络实现选择(asio/epoll) | `asio` |

### 3.3 运行测试

```bash
# 在构建目录下
cd build

# 运行所有测试
ctest

# 运行特定测试
ctest -R "NavTest"

# 带详细输出
ctest -V
```

### 3.4 代码覆盖率

```bash
# 配置覆盖率构建
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
ninja

# 运行测试并生成覆盖率报告
ninja coverage

# 查看报告
xdg-open coverage/index.html
```

## 4. 代码规范与风格

### 4.1 C++编码规范

本项目遵循Google C++编码风格，主要规范包括：

- 使用4空格缩进
- 类名使用PascalCase (例如: `NavController`)
- 方法名和变量名使用camelCase (例如: `processMessage()`)
- 常量和枚举使用UPPER_SNAKE_CASE (例如: `MAX_SPEED`)
- 私有成员变量以下划线结尾 (例如: `counter_`)
- 头文件保护使用全大写的项目前缀 (例如: `ROBOT_NAV_CONTROLLER_H_`)

### 4.2 命名约定

| 类型 | 规范 | 示例 |
|-----|------|------|
| 类名 | PascalCase | `MessageQueue`, `NavController` |
| 函数/方法 | camelCase | `sendMessage()`, `processEvent()` |
| 变量 | camelCase | `currentSpeed`, `pathPoints` |
| 常量 | UPPER_SNAKE_CASE | `MAX_RETRY_COUNT`, `DEFAULT_TIMEOUT` |
| 枚举值 | UPPER_SNAKE_CASE | `STATE_INIT`, `ERROR_CONNECTION_TIMEOUT` |
| 文件名 | snake_case | `message_queue.cpp`, `nav_controller.hpp` |

### 4.3 代码格式化

项目使用clang-format进行代码格式化。配置文件位于项目根目录的`.clang-format`。

```bash
# 格式化单个文件
clang-format -i src/app/cli/command_processor.cpp

# 格式化所有源文件
find src include -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

### 4.4 静态分析

项目使用clang-tidy进行静态代码分析。

```bash
# 在构建目录下
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

# 运行静态分析
clang-tidy -p . ../src/app/cli/command_processor.cpp
```

## 5. 架构与扩展指南

### 5.1 核心架构概览

机器狗导航控制系统采用分层架构，确保各组件职责明确、耦合度低：

1. **App层**：负责用户交互和配置管理
2. **Procedure层**：负责业务逻辑和状态管理
3. **Network层**：负责网络通信和协议处理

组件通过以下机制通信：

- **Message Queue**：组件间异步通信
- **Event Bus**：基于发布/订阅模式的事件通知
- **Direct Interface**：层内组件的直接调用

### 5.2 添加新功能

添加新功能时，遵循以下步骤：

1. **确定功能所属层**：确定新功能属于哪一层
2. **接口设计**：设计公共接口和数据结构
3. **实现功能**：实现具体功能
4. **单元测试**：编写单元测试验证功能
5. **集成测试**：编写集成测试验证与其他组件的交互
6. **文档**：更新相关文档

#### 示例：添加新命令

```cpp
// 1. 在include/app/cli/commands/下创建header
// path_export_command.hpp
#pragma once
#include "app/cli/command.hpp"

namespace robot_nav {
namespace app {
namespace cli {

class PathExportCommand : public Command {
public:
    PathExportCommand();
    ~PathExportCommand() override;

    // 命令名称
    std::string getName() const override { return "path:export"; }

    // 命令帮助
    std::string getHelp() const override;

    // 执行命令
    void execute(const CommandArgs& args, CommandResult& result) override;

private:
    // 实现细节
};

} // namespace cli
} // namespace app
} // namespace robot_nav

// 2. 在src/app/cli/commands/下实现
// path_export_command.cpp
#include "app/cli/commands/path_export_command.hpp"
// 实现代码...

// 3. 在CommandFactory中注册
// command_factory.cpp
void CommandFactory::registerCommands() {
    // 已有命令...
    registerCommand(std::make_shared<PathExportCommand>());
}
```

### 5.3 状态机扩展

系统使用Boost.MSM实现状态机。扩展状态机需要：

1. 定义新状态
2. 定义新事件
3. 定义状态转换
4. 实现进入/退出动作

```cpp
// 添加新状态示例
struct Recovering : public msm::front::state<> {
    // 进入状态的动作
    template <class Event, class FSM>
    void on_entry(Event const& evt, FSM& fsm) {
        LOG_INFO("进入恢复状态");
        // 实现...
    }

    // 退出状态的动作
    template <class Event, class FSM>
    void on_exit(Event const& evt, FSM& fsm) {
        LOG_INFO("退出恢复状态");
        // 实现...
    }
};

// 添加新事件
struct RecoveryEvent {
    RecoveryEvent(const std::string& reason) : reason(reason) {}
    std::string reason;
};

// 在状态机中定义状态转换
// ...
struct transition_table : mpl::vector<
    // 已有转换...
    _row < Nav, RecoveryEvent, Recovering, &NavStateMachine_::startRecovery >,
    _row < Recovering, CompleteEvent, Nav, &NavStateMachine_::resumeNavigation >
> {};
```

### 5.4 网络层扩展

系统提供两种网络实现（Boost.Asio和Epoll）。添加新的通信协议需要：

1. 实现`ProtocolHandler`接口
2. 实现消息序列化/反序列化
3. 注册协议处理器

```cpp
// 添加新协议示例
class CustomProtocolHandler : public ProtocolHandler {
public:
    CustomProtocolHandler();
    ~CustomProtocolHandler() override;

    // 处理接收到的消息
    void handleMessage(const RawMessage& message) override;

    // 创建要发送的消息
    RawMessage createMessage(const Message& message) override;

private:
    // 实现细节
};

// 在NetworkManager中注册
// network_manager.cpp
void NetworkManager::initializeProtocolHandlers() {
    // 已有处理器...
    registerProtocolHandler("custom", std::make_shared<CustomProtocolHandler>());
}
```

## 6. 消息队列与事件系统

### 6.1 消息队列

消息队列是系统各组件间异步通信的核心机制。

```cpp
// 生产消息
auto message = std::make_shared<NavRequestMessage>(target);
MessageQueue::getInstance().enqueue(message);

// 消费消息
while (auto message = MessageQueue::getInstance().dequeue()) {
    MessageDispatcher::dispatch(message);
}
```

### 6.2 事件总线

事件总线基于发布/订阅模式，实现组件间的松耦合通信。

```cpp
// 定义事件处理器
class NavStatusListener : public EventListener {
public:
    void onEvent(const Event& event) override {
        if (event.type == EventType::NAV_STATUS_CHANGED) {
            const auto& statusEvent = static_cast<const NavStatusEvent&>(event);
            // 处理导航状态变化...
        }
    }
};

// 订阅事件
auto listener = std::make_shared<NavStatusListener>();
EventBus::getInstance().subscribe(EventType::NAV_STATUS_CHANGED, listener);

// 发布事件
auto event = std::make_shared<NavStatusEvent>(NavStatus::ACTIVE);
EventBus::getInstance().publish(event);
```

## 7. 测试指南

### 7.1 单元测试

系统使用Google Test框架进行单元测试。

```cpp
// message_queue_test.cpp
#include <gtest/gtest.h>
#include "procedure/message/message_queue.hpp"

using namespace robot_nav::procedure;

TEST(MessageQueueTest, EnqueueDequeue) {
    MessageQueue queue;

    // 准备测试数据
    auto message = std::make_shared<TextMessage>("test");

    // 执行操作
    queue.enqueue(message);
    auto retrieved = queue.dequeue();

    // 验证结果
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->getType(), MessageType::TEXT);

    auto textMessage = std::dynamic_pointer_cast<TextMessage>(retrieved);
    EXPECT_EQ(textMessage->content, "test");
}
```

### 7.2 模拟对象

使用Google Mock创建模拟对象进行测试。

```cpp
// network_client_mock.hpp
#include <gmock/gmock.h>
#include "network/tcp/tcp_client.hpp"

namespace robot_nav {
namespace test {

class MockTcpClient : public network::TcpClient {
public:
    MOCK_METHOD(bool, connect, (const std::string&, int), (override));
    MOCK_METHOD(void, disconnect, (), (override));
    MOCK_METHOD(bool, send, (const std::vector<uint8_t>&), (override));
    MOCK_METHOD(bool, isConnected, (), (const, override));
};

} // namespace test
} // namespace robot_nav

// 使用模拟对象
TEST(NetworkTest, SendMessage) {
    // 创建模拟对象
    MockTcpClient mockClient;

    // 设置期望
    EXPECT_CALL(mockClient, isConnected())
        .WillOnce(Return(true));

    EXPECT_CALL(mockClient, send(testing::_))
        .WillOnce(Return(true));

    // 使用模拟对象测试
    NetworkManager manager(&mockClient);
    bool result = manager.sendMessage("test message");

    // 验证结果
    EXPECT_TRUE(result);
}
```

### 7.3 集成测试

集成测试验证多个组件的交互。

```cpp
// navigation_integration_test.cpp
#include <gtest/gtest.h>
#include "app/cli/command_processor.hpp"
#include "procedure/state_machine/nav_state_machine.hpp"
#include "test/mock/network_client_mock.hpp"

using namespace robot_nav;
using ::testing::Return;
using ::testing::_;

class NavigationIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试环境
        mockClient = std::make_shared<test::MockTcpClient>();
        networkManager = std::make_shared<network::NetworkManager>(mockClient);
        stateMachine = std::make_shared<procedure::NavStateMachine>(networkManager);
        commandProcessor = std::make_shared<app::cli::CommandProcessor>(stateMachine);

        // 设置模拟对象行为
        EXPECT_CALL(*mockClient, connect(_, _))
            .WillRepeatedly(Return(true));
        EXPECT_CALL(*mockClient, isConnected())
            .WillRepeatedly(Return(true));
        EXPECT_CALL(*mockClient, send(_))
            .WillRepeatedly(Return(true));
    }

    std::shared_ptr<test::MockTcpClient> mockClient;
    std::shared_ptr<network::NetworkManager> networkManager;
    std::shared_ptr<procedure::NavStateMachine> stateMachine;
    std::shared_ptr<app::cli::CommandProcessor> commandProcessor;
};

TEST_F(NavigationIntegrationTest, StartNavigation) {
    // 执行命令
    app::cli::CommandResult result;
    app::cli::CommandArgs args;
    args["target"] = "kitchen";

    commandProcessor->executeCommand("nav:start", args, result);

    // 验证结果
    EXPECT_TRUE(result.success);
    EXPECT_EQ(stateMachine->getCurrentState(), procedure::NavState::NAV);
}
```

## 8. 版本控制与贡献流程

### 8.1 分支策略

项目使用以下分支策略：

- `main`: 稳定版本分支，只接受合并请求
- `develop`: 开发分支，新功能合并到此分支
- `feature/*`: 功能分支，用于开发新功能
- `bugfix/*`: 修复分支，用于修复bug
- `release/*`: 发布分支，用于准备新版本发布

### 8.2 提交规范

提交消息遵循以下格式：

```
<类型>(<范围>): <简短描述>

<详细描述>

<关闭的Issues>
```

类型包括：
- `feat`: 新功能
- `fix`: Bug修复
- `docs`: 文档修改
- `style`: 代码风格修改
- `refactor`: 代码重构
- `test`: 测试用例
- `chore`: 构建过程或辅助工具变动

例如：
```
feat(nav): 添加路径导出功能

添加了将当前导航路径导出为JSON文件的功能。
支持两种格式：详细格式和简化格式。

Closes #123
```

### 8.3 代码审查清单

提交代码前，确保：

1. 代码符合项目编码规范
2. 通过所有单元测试
3. 不引入新的警告或错误
4. 添加适当的文档
5. 合理的日志记录
6. 异常处理完善
7. 适当的注释

### 8.4 发布流程

1. 从`develop`分支创建`release/X.Y.Z`分支
2. 在发布分支上修复bug和完善文档
3. 更新版本号和CHANGELOG
4. 合并到`main`分支并打标签
5. 同时合并回`develop`分支

## 9. 文档生成

项目使用Doxygen生成API文档。

### 9.1 文档注释规范

```cpp
/**
 * @brief 处理导航命令
 *
 * 解析并执行导航相关的命令，如开始导航、停止导航等
 *
 * @param command 命令名称
 * @param args 命令参数
 * @return true 命令执行成功
 * @return false 命令执行失败
 *
 * @note 此函数线程安全
 * @see NavController
 */
bool handleNavCommand(const std::string& command, const CommandArgs& args);
```

### 9.2 生成文档

```bash
# 在项目根目录下
doxygen docs/Doxyfile

# 查看生成的文档
xdg-open docs/html/index.html
```

## 10. 性能分析与调优

### 10.1 性能分析工具

- **CPU性能**：使用perf进行CPU分析
- **内存分析**：使用Valgrind进行内存分析
- **线程分析**：使用helgrind检测线程问题

### 10.2 性能分析示例

```bash
# CPU性能分析
perf record -g ./build/bin/robot_nav
perf report

# 内存分析
valgrind --tool=memcheck --leak-check=full ./build/bin/robot_nav

# 线程竞争分析
valgrind --tool=helgrind ./build/bin/robot_nav
```

### 10.3 常见性能优化

- 使用高效的数据结构
- 减少不必要的内存分配
- 避免过度使用虚函数
- 使用移动语义减少拷贝
- 优化锁的粒度和持有时间

## 11. 调试技巧

### 11.1 日志调试

系统内置了分级日志系统：

```cpp
LOG_DEBUG("准备连接到服务器: %s:%d", host.c_str(), port);
LOG_INFO("连接成功，等待数据...");
LOG_WARNING("接收队列接近上限: %d/%d", queueSize, maxQueueSize);
LOG_ERROR("连接失败: %s", errorMsg.c_str());
```

### 11.2 使用GDB调试

```bash
# 在构建目录下
gdb ./bin/robot_nav

# 设置断点
(gdb) b NavController.cpp:125
(gdb) run

# 检查变量
(gdb) print pathPoints
(gdb) print currentState
```

### 11.3 内存调试

```bash
# 运行Address Sanitizer
./build/bin/robot_nav_asan

# 使用Valgrind
valgrind --tool=memcheck --leak-check=full ./build/bin/robot_nav
```

## 12. 常见问题与解决方案

### 12.1 构建问题

**问题**：找不到Boost库
**解决方案**：
```bash
# Ubuntu
sudo apt install libboost-all-dev

# MacOS
brew install boost

# 在CMake中指定Boost路径
cmake -DBOOST_ROOT=/path/to/boost ..
```

**问题**：编译错误 "缺少xyz.hpp"
**解决方案**：检查include路径和文件名大小写

### 12.2 运行问题

**问题**：连接错误 "无法连接到机器狗设备"
**解决方案**：
- 检查网络连接和防火墙设置
- 确保设备IP和端口正确
- 验证设备已开启并运行相应服务

**问题**：崩溃于 "段错误"
**解决方案**：
- 使用gdb获取堆栈跟踪
- 检查空指针和越界访问
- 使用Address Sanitizer定位内存问题

### 12.3 其他常见问题

**问题**：状态机卡在特定状态
**解决方案**：
- 查看日志确定卡住原因
- 检查事件条件是否满足
- 确认状态转换是否定义正确

**问题**：消息队列溢出
**解决方案**：
- 增加队列容量
- 优化消息处理速度
- 减少不必要的消息生成
