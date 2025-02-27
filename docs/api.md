# API 文档

本文档详细描述了机器狗导航控制系统的主要 API 接口。

## 目录

- [网络通信协议](#网络通信协议)
- [状态机接口](#状态机接口)
- [事件总线](#事件总线)
- [消息队列](#消息队列)

## 网络通信协议

### TCP 通信协议

#### 消息格式

所有消息都遵循以下格式：

```cpp
struct ProtocolHeader {
    uint32_t magic;      // 魔数：0x12345678
    uint32_t version;    // 协议版本
    uint32_t length;     // 消息体长度
    uint32_t type;       // 消息类型
    uint32_t sequence;   // 序列号
    uint32_t timestamp;  // 时间戳
};
```

#### 消息类型

```cpp
enum MessageType {
    HEARTBEAT = 0x01,        // 心跳包
    NAVIGATION = 0x02,       // 导航指令
    STATUS = 0x03,          // 状态信息
    ERROR = 0x04,           // 错误信息
    CONFIG = 0x05           // 配置信息
};
```

### 网络模型接口

```cpp
class BaseNetworkModel {
public:
    virtual bool connect(const std::string& ip, int port) = 0;
    virtual void disconnect() = 0;
    virtual bool send(const std::string& data) = 0;
    virtual bool receive(std::string& data) = 0;
};
```

## 状态机接口

### 导航状态机

```cpp
class NavigationMachine {
public:
    // 状态定义
    enum State {
        IDLE,           // 空闲状态
        INITIALIZING,   // 初始化
        NAVIGATING,     // 导航中
        PAUSED,         // 暂停
        ERROR          // 错误状态
    };

    // 事件定义
    struct Events {
        struct Start {};
        struct Stop {};
        struct Pause {};
        struct Resume {};
        struct Error {};
    };

    // 状态转换方法
    void start();
    void stop();
    void pause();
    void resume();
    void handleError();
};
```

## 事件总线

### 事件发布订阅

```cpp
class EventBus {
public:
    // 订阅事件
    template<typename Event>
    void subscribe(std::function<void(const Event&)> handler);

    // 发布事件
    template<typename Event>
    void publish(const Event& event);

    // 取消订阅
    template<typename Event>
    void unsubscribe();
};
```

### 事件类型

```cpp
// 导航事件
struct NavigationEvent {
    std::vector<Point> path;
    double speed;
    double direction;
};

// 状态更新事件
struct StatusUpdateEvent {
    State currentState;
    std::string message;
};
```

## 消息队列

### 消息队列接口

```cpp
template<typename T>
class MessageQueue {
public:
    // 发送消息
    void send(const T& message);

    // 接收消息
    T receive();

    // 非阻塞接收
    bool tryReceive(T& message);

    // 清空队列
    void clear();
};
```

## 使用示例

### 1. 初始化系统

```cpp
X30InspectionSystem system;
system.initialize("192.168.1.100", 8080);
```

### 2. 设置导航路径

```cpp
NavigationEvent event;
event.path = {
    {0.0, 0.0},
    {1.0, 1.0},
    {2.0, 0.0}
};
event.speed = 0.5;
EventBus::getInstance().publish(event);
```

### 3. 状态监控

```cpp
EventBus::getInstance().subscribe<StatusUpdateEvent>(
    [](const StatusUpdateEvent& event) {
        std::cout << "Current state: " << event.currentState << std::endl;
        std::cout << "Message: " << event.message << std::endl;
    }
);
```

## 错误处理

所有接口都使用异常处理机制：

```cpp
try {
    system.initialize("192.168.1.100", 8080);
} catch (const NetworkException& e) {
    std::cerr << "Network error: " << e.what() << std::endl;
} catch (const std::exception& e) {
    std::cerr << "General error: " << e.what() << std::endl;
}
```

## 注意事项

1. 所有网络操作都是异步的
2. 事件处理器在单独的线程中运行
3. 消息队列是线程安全的
4. 状态机转换是原子操作

## 版本历史

- v1.0.0: 初始版本
- v1.1.0: 添加异步网络支持
- v1.2.0: 改进状态机实现
- v1.3.0: 添加事件总线功能
