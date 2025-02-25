# 代码规范

## 基本原则

1. **清晰性**
   - 代码应该清晰易读
   - 避免晦涩的写法
   - 适当的注释说明
   - 有意义的命名

2. **一致性**
   - 遵循统一的代码风格
   - 保持命名风格一致
   - 使用一致的设计模式
   - 保持文件组织结构一致

3. **可维护性**
   - 模块化设计
   - 单一职责原则
   - 避免代码重复
   - 合理的抽象层次

## 命名规范

### 1. 文件命名

```cpp
// 头文件
network_model.hpp
tcp_server.hpp
robot_controller.hpp

// 源文件
network_model.cpp
tcp_server.cpp
robot_controller.cpp

// 测试文件
network_model_test.cpp
tcp_server_test.cpp
robot_controller_test.cpp
```

### 2. 类命名

```cpp
class NetworkModel;        // 类名使用大驼峰命名法
class TcpServer;          // 每个单词首字母大写
class RobotController;    // 使用名词或名词短语
```

### 3. 函数命名

```cpp
void connectToServer();   // 函数名使用小驼峰命名法
bool isConnected();       // 判断函数使用is/has/can等前缀
void handleMessage();     // 动词开头，表示动作
void onMessageReceived(); // 回调函数使用on前缀
```

### 4. 变量命名

```cpp
int message_count_;      // 成员变量使用下划线结尾
static int max_retries;  // 静态变量不使用下划线
const int MAX_BUFFER_SIZE = 1024;  // 常量全大写，下划线分隔
```

### 5. 命名空间

```cpp
namespace network {      // 命名空间使用小写
namespace utils {        // 可以嵌套
namespace detail {       // 内部实现使用detail命名空间
```

## 格式规范

### 1. 缩进和空格

```cpp
class Example {
public:                          // 访问说明符顶格写
    void function() {           // 函数体缩进4个空格
        if (condition) {        // 控制语句缩进4个空格
            doSomething();      // 语句块缩进4个空格
        }                       // 右花括号独占一行
    }                          // 右花括号独占一行

    int value() const {        // const关键字跟在函数后面
        return value_;         // 返回语句缩进4个空格
    }

private:
    int value_;               // 成员变量声明缩进4个空格
};
```

### 2. 括号和空格

```cpp
if (condition) {              // if后面有空格，条件两边有空格
    statement;
} else if (condition) {       // else if连在一起
    statement;
} else {                     // else单独一行
    statement;
}

while (condition) {          // while后面有空格
    statement;
}

for (int i = 0; i < n; ++i) {  // for循环分号后面有空格
    statement;
}

function(arg1, arg2);        // 函数调用参数用逗号加空格分隔
```

### 3. 换行和对齐

```cpp
// 长表达式换行
int result = very_long_variable_name + another_long_variable_name +
             yet_another_long_variable_name;

// 函数参数过多时的换行
void longFunction(int parameter1, int parameter2,
                 int parameter3, int parameter4,
                 int parameter5);

// 初始化列表换行
MyClass::MyClass()
    : first_member_(1),
      second_member_(2),
      third_member_(3) {
}
```

## 注释规范

### 1. 文件头注释

```cpp
/**
 * @file network_model.hpp
 * @brief 网络模型的基类定义
 * @author 作者名
 * @date 2024-02-25
 *
 * 提供网络通信的基础抽象接口，支持多种网络模型实现。
 */
```

### 2. 类注释

```cpp
/**
 * @class NetworkModel
 * @brief 网络模型基类
 *
 * 定义网络通信的基本接口，包括连接管理、数据收发等功能。
 * 派生类需要实现具体的网络模型。
 */
class NetworkModel {
    // ...
};
```

### 3. 函数注释

```cpp
/**
 * @brief 连接到指定服务器
 * @param host 服务器地址
 * @param port 服务器端口
 * @return 连接是否成功
 * @throw NetworkException 当网络错误发生时
 */
bool connect(const std::string& host, uint16_t port);
```

### 4. 变量注释

```cpp
private:
    int retry_count_;     ///< 重试次数
    bool is_connected_;   ///< 连接状态

    /// 最大缓冲区大小（字节）
    static const int MAX_BUFFER_SIZE = 1024;
```

## 编码实践

### 1. 头文件组织

```cpp
// 1. 版权声明和文件说明
// 2. 防止头文件重复包含
#ifndef NETWORK_MODEL_HPP
#define NETWORK_MODEL_HPP

// 3. C++标准库头文件
#include <string>
#include <vector>

// 4. 第三方库头文件
#include <boost/asio.hpp>

// 5. 项目头文件
#include "common/types.hpp"
#include "network/network_types.hpp"

// 6. 代码实现
namespace network {
    // ...
}

#endif // NETWORK_MODEL_HPP
```

### 2. 源文件组织

```cpp
// 1. 对应的头文件
#include "network/network_model.hpp"

// 2. C++标准库头文件
#include <iostream>

// 3. 第三方库头文件
#include <boost/format.hpp>

// 4. 项目其他头文件
#include "common/logger.hpp"

// 5. 匿名命名空间（如果需要）
namespace {
    const int DEFAULT_TIMEOUT = 5000;
}

// 6. 代码实现
namespace network {
    // ...
}
```

### 3. 类的组织

```cpp
class NetworkModel {
public:
    // 1. 构造和析构函数
    NetworkModel();
    virtual ~NetworkModel();

    // 2. 公共接口
    bool connect();
    void disconnect();

    // 3. 访问器和修改器
    bool isConnected() const;
    void setTimeout(int timeout);

protected:
    // 4. 保护成员
    void onConnect();
    void onDisconnect();

private:
    // 5. 私有辅助函数
    void initializeConnection();
    void cleanupConnection();

    // 6. 私有成员变量
    bool is_connected_;
    int timeout_;
};
```

## 最佳实践

### 1. RAII原则

```cpp
class ResourceManager {
public:
    ResourceManager() {
        // 在构造函数中获取资源
        resource_ = acquireResource();
    }

    ~ResourceManager() {
        // 在析构函数中释放资源
        releaseResource(resource_);
    }

private:
    Resource* resource_;
};
```

### 2. 智能指针使用

```cpp
// 优先使用智能指针而不是裸指针
std::shared_ptr<NetworkModel> model =
    std::make_shared<NetworkModel>();

// unique_ptr用于独占所有权
std::unique_ptr<Buffer> buffer =
    std::make_unique<Buffer>(1024);

// weak_ptr用于打破循环引用
std::weak_ptr<Connection> weak_conn = conn;
```

### 3. 异常处理

```cpp
try {
    // 可能抛出异常的代码
    connection->send(data);
} catch (const NetworkException& e) {
    // 处理网络异常
    LOG_ERROR("Network error: {}", e.what());
} catch (const std::exception& e) {
    // 处理标准异常
    LOG_ERROR("Standard error: {}", e.what());
} catch (...) {
    // 处理未知异常
    LOG_ERROR("Unknown error occurred");
}
```

## 代码审查清单

1. **可读性**
   - 命名是否清晰直观
   - 代码结构是否合理
   - 注释是否充分准确
   - 是否遵循项目规范

2. **正确性**
   - 逻辑是否正确
   - 边界条件是否处理
   - 错误处理是否完善
   - 内存管理是否安全

3. **性能**
   - 算法是否高效
   - 资源使用是否合理
   - 是否有性能瓶颈
   - 是否考虑并发

4. **可维护性**
   - 代码是否模块化
   - 依赖关系是否合理
   - 是否易于扩展
   - 是否易于测试
