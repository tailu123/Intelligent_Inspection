# API文档

## 概述

智能巡检系统提供了一组完整的API接口，用于系统控制、任务管理和状态监控。本文档详细说明了这些接口的使用方法和注意事项。

## 网络接口

### 1. 基础网络接口 (BaseNetworkModel)

```cpp
class BaseNetworkModel {
public:
    /**
     * @brief 初始化网络模型
     * @return 初始化是否成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 连接到指定服务器
     * @param host 服务器地址
     * @param port 服务器端口
     * @return 连接是否成功
     */
    virtual bool connect(const std::string& host, uint16_t port) = 0;

    /**
     * @brief 断开连接
     */
    virtual void disconnect() = 0;

    /**
     * @brief 发送消息
     * @param message 要发送的消息
     * @return 发送是否成功
     */
    virtual bool send(const Message& message) = 0;

    /**
     * @brief 接收消息
     * @param message 接收到的消息
     * @return 接收是否成功
     */
    virtual bool receive(Message& message) = 0;
};
```

### 2. 消息定义

```cpp
struct Message {
    MessageHeader header;      // 消息头
    std::vector<uint8_t> payload;  // 消息体
    uint32_t checksum;        // 校验和

    /**
     * @brief 验证消息的有效性
     * @return 消息是否有效
     */
    bool isValid() const;

    /**
     * @brief 获取消息的总大小
     * @return 消息大小（字节）
     */
    size_t size() const;
};

struct MessageHeader {
    uint8_t  sync;           // 同步字节
    uint8_t  version;        // 协议版本
    uint16_t length;         // 消息长度
    uint16_t command_id;     // 命令ID
    uint16_t sequence_id;    // 序列号
    uint32_t timestamp;      // 时间戳
    uint8_t  flags;          // 标志位
};
```

## 任务接口

### 1. 任务管理器 (TaskManager)

```cpp
class TaskManager {
public:
    /**
     * @brief 创建新任务
     * @param config 任务配置
     * @return 任务ID
     */
    TaskId createTask(const TaskConfig& config);

    /**
     * @brief 执行指定任务
     * @param id 任务ID
     * @return 执行是否成功
     */
    bool executeTask(TaskId id);

    /**
     * @brief 暂停任务
     * @param id 任务ID
     * @return 暂停是否成功
     */
    bool pauseTask(TaskId id);

    /**
     * @brief 恢复任务
     * @param id 任务ID
     * @return 恢复是否成功
     */
    bool resumeTask(TaskId id);

    /**
     * @brief 取消任务
     * @param id 任务ID
     * @return 取消是否成功
     */
    bool cancelTask(TaskId id);

    /**
     * @brief 获取任务状态
     * @param id 任务ID
     * @return 任务状态
     */
    TaskStatus getTaskStatus(TaskId id);
};
```

### 2. 任务配置

```cpp
struct TaskConfig {
    std::string name;              // 任务名称
    TaskType type;                 // 任务类型
    int priority;                  // 优先级
    std::vector<Point> points;     // 巡检点位
    std::map<std::string, std::string> params;  // 附加参数
};

struct Point {
    double x;                      // X坐标
    double y;                      // Y坐标
    double z;                      // Z坐标
    PointType type;               // 点位类型
    std::string description;       // 点位描述
};
```

## 状态接口

### 1. 状态管理器 (StateManager)

```cpp
class StateManager {
public:
    /**
     * @brief 获取当前状态
     * @return 系统状态
     */
    SystemState getCurrentState() const;

    /**
     * @brief 更新状态
     * @param new_state 新状态
     * @return 更新是否成功
     */
    bool updateState(const SystemState& new_state);

    /**
     * @brief 注册状态观察者
     * @param observer 观察者对象
     */
    void registerObserver(StateObserver* observer);

    /**
     * @brief 移除状态观察者
     * @param observer 观察者对象
     */
    void removeObserver(StateObserver* observer);
};
```

### 2. 状态定义

```cpp
enum class SystemState {
    INITIALIZING,    // 初始化中
    READY,          // 就绪
    RUNNING,        // 运行中
    PAUSED,         // 已暂停
    ERROR,          // 错误
    SHUTDOWN        // 已关闭
};

struct StateInfo {
    SystemState state;            // 系统状态
    std::string description;      // 状态描述
    uint32_t timestamp;           // 时间戳
    std::map<std::string, std::string> details;  // 详细信息
};
```

## 事件接口

### 1. 事件总线 (EventBus)

```cpp
class EventBus {
public:
    /**
     * @brief 订阅事件
     * @param handler 事件处理函数
     */
    template<typename EventType>
    void subscribe(const std::function<void(const EventType&)>& handler);

    /**
     * @brief 发布事件
     * @param event 事件对象
     */
    template<typename EventType>
    void publish(const EventType& event);

    /**
     * @brief 取消订阅
     * @param event_type 事件类型
     */
    void unsubscribe(const std::string& event_type);
};
```

### 2. 事件定义

```cpp
struct Event {
    std::string type;             // 事件类型
    uint32_t timestamp;           // 时间戳
    std::string source;           // 事件源
    nlohmann::json data;          // 事件数据
};

// 系统事件
struct SystemEvent : Event {
    SystemState old_state;        // 旧状态
    SystemState new_state;        // 新状态
    std::string reason;           // 状态变更原因
};

// 任务事件
struct TaskEvent : Event {
    TaskId task_id;               // 任务ID
    TaskStatus status;            // 任务状态
    float progress;               // 任务进度
};
```

## 错误处理

### 1. 异常定义

```cpp
class SystemException : public std::exception {
public:
    SystemException(ErrorCode code, const std::string& message);

    ErrorCode getCode() const;
    const char* what() const noexcept override;

private:
    ErrorCode code_;
    std::string message_;
};

enum class ErrorCode {
    SUCCESS = 0,
    INVALID_ARGUMENT,
    NETWORK_ERROR,
    TASK_ERROR,
    STATE_ERROR,
    UNKNOWN_ERROR
};
```

### 2. 错误回调

```cpp
struct ErrorCallback {
    /**
     * @brief 错误处理回调
     * @param error 错误信息
     */
    virtual void onError(const Error& error) = 0;

    /**
     * @brief 警告处理回调
     * @param warning 警告信息
     */
    virtual void onWarning(const Warning& warning) = 0;
};
```

## 使用示例

### 1. 基本使用流程

```cpp
// 创建并初始化系统
auto system = std::make_unique<InspectionSystem>();
system->initialize();

// 创建任务
TaskConfig config;
config.name = "巡检任务1";
config.type = TaskType::INSPECTION;
config.priority = 1;

TaskId task_id = system->createTask(config);

// 执行任务
system->executeTask(task_id);

// 监控任务状态
while (true) {
    TaskStatus status = system->getTaskStatus(task_id);
    if (status == TaskStatus::COMPLETED) {
        break;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
```

### 2. 事件处理示例

```cpp
// 注册事件处理器
EventBus::getInstance().subscribe<TaskEvent>([](const TaskEvent& event) {
    std::cout << "Task " << event.task_id
              << " status: " << toString(event.status)
              << " progress: " << event.progress << std::endl;
});

// 注册错误处理器
class ErrorHandler : public ErrorCallback {
public:
    void onError(const Error& error) override {
        LOG_ERROR("Error occurred: {}", error.getMessage());
    }

    void onWarning(const Warning& warning) override {
        LOG_WARN("Warning: {}", warning.getMessage());
    }
};

system->setErrorCallback(std::make_shared<ErrorHandler>());
```

## 注意事项

1. **线程安全**
   - 所有接口都是线程安全的
   - 注意避免死锁
   - 合理使用同步机制

2. **错误处理**
   - 总是检查返回值
   - 使用try-catch捕获异常
   - 正确处理错误状态

3. **性能考虑**
   - 避免频繁创建销毁对象
   - 合理使用异步操作
   - 注意资源释放

4. **配置管理**
   - 使用配置文件管理参数
   - 运行时可修改配置
   - 保存配置历史
