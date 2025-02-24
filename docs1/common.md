# 通用组件模块设计文档

## 1. 模块概述

通用组件模块提供了系统所需的基础设施，包括事件总线、消息队列和工具类等核心功能组件。这些组件被其他模块广泛使用，是系统的基础支撑。

## 2. 核心组件

### 2.1 事件总线 (EventBus)

```cpp
class EventBus {
public:
    // 事件订阅
    template<typename T>
    std::string subscribe(EventHandler handler);

    // 事件发布
    void publish(const std::shared_ptr<Event>& event);

    // 取消订阅
    void unsubscribe(const std::string& type, const std::string& id);
};
```

特性：
- 类型安全的事件处理
- 异步事件分发
- 支持优先级
- 自动事件清理

### 2.2 消息队列 (MessageQueue)

```cpp
class MessageQueue {
public:
    void push(const Message& msg);
    bool try_pop(Message& msg);
    void process_messages();

private:
    std::queue<Message> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
```

特性：
- 线程安全
- 支持阻塞和非阻塞操作
- 自动容量管理
- 优先级队列支持

### 2.3 工具类 (Utils)

1. **时间工具**
   ```cpp
   namespace utils {
       std::string getCurrentTimestamp();
       uint64_t getMilliseconds();
       void sleep(uint32_t ms);
   }
   ```

2. **字符串工具**
   ```cpp
   namespace utils {
       std::string format(const char* fmt, ...);
       std::vector<std::string> split(const std::string& str, char delim);
       std::string trim(const std::string& str);
   }
   ```

## 3. 事件系统

### 3.1 事件类型
```cpp
class Event {
public:
    virtual std::string getType() const = 0;
    virtual ~Event() = default;
};

// 具体事件示例
class MessageResponseEvent : public Event {
    std::string getType() const override { return "MessageResponse"; }
    // 事件数据...
};
```

### 3.2 事件处理
```cpp
// 事件处理器
using EventHandler = std::function<void(const std::shared_ptr<Event>&)>;

// 事件订阅示例
event_bus.subscribe<MessageResponseEvent>([](const auto& event) {
    // 处理事件...
});
```

## 4. 消息处理

### 4.1 消息类型
```cpp
struct Message {
    uint16_t type;
    std::vector<uint8_t> data;
    uint64_t timestamp;
};
```

### 4.2 消息处理流程
1. 消息入队
2. 消息分发
3. 处理器调用
4. 结果反馈

## 5. 线程安全

### 5.1 同步原语
- 互斥锁
- 条件变量
- 原子操作
- 读写锁

### 5.2 并发控制
- 队列访问保护
- 事件处理同步
- 资源访问控制

## 6. 性能优化

1. **内存管理**
   - 对象池
   - 内存对齐
   - 缓存友好

2. **并发优化**
   - 无锁算法
   - 细粒度锁
   - 批处理

## 7. 使用示例

### 7.1 事件总线
```cpp
// 订阅事件
auto id = event_bus.subscribe<NavEvent>([](const auto& event) {
    std::cout << "Received nav event\n";
});

// 发布事件
auto event = std::make_shared<NavEvent>();
event_bus.publish(event);
```

### 7.2 消息队列
```cpp
// 发送消息
Message msg{1001, data, timestamp};
message_queue.push(msg);

// 处理消息
Message received;
while (message_queue.try_pop(received)) {
    process_message(received);
}
```

## 8. 错误处理

1. **异常处理**
   - 类型错误
   - 资源耗尽
   - 并发冲突

2. **错误恢复**
   - 自动重试
   - 降级处理
   - 资源清理

## 9. 扩展性

1. **新事件类型**
   - 自定义事件
   - 事件过滤
   - 优先级控制

2. **新工具组件**
   - 配置管理
   - 日志系统
   - 监控工具

## 10. 注意事项

1. **资源管理**
   - 内存泄漏防护
   - 资源及时释放
   - 循环引用检测

2. **性能考虑**
   - 锁竞争
   - 缓存利用
   - 内存分配

3. **可维护性**
   - 接口简洁
   - 文档完善
   - 测试覆盖
