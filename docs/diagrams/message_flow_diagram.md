# 消息流程图表

## 1. 系统消息流转概览

本文档描述机器狗导航控制系统中消息的流转路径、类型和处理机制。

### 1.1 基本消息流转图

```mermaid
sequenceDiagram
    participant CLI as 命令行界面
    participant MQ as 消息队列
    participant PT as 处理线程
    participant SM as 状态机
    participant EB as 事件总线
    participant NC as 网络客户端
    participant RD as 机器狗设备

    CLI->>MQ: 1. 放入命令消息
    MQ->>PT: 2. 消费消息
    PT->>SM: 3. 处理命令/更新状态
    SM->>NC: 4. 发送网络请求
    NC->>RD: 5. 发送TCP数据
    RD->>NC: 6. 返回响应数据
    NC->>MQ: 7. 放入响应消息
    MQ->>PT: 8. 消费响应
    PT->>SM: 9. 处理响应/更新状态
    SM->>EB: 10. 发布状态变更事件
    EB->>CLI: 11. 通知UI更新
```

### 1.2 异步消息处理流程

```mermaid
sequenceDiagram
    participant Producer as 消息生产者
    participant MQ as 消息队列
    participant PT as 处理线程
    participant Handler as 消息处理器
    participant EB as 事件总线
    participant Subscribers as 事件订阅者

    Producer->>MQ: 1. enqueue(message)
    Note over MQ: 消息入队

    loop 消息处理循环
        PT->>MQ: 2. dequeue()
        MQ-->>PT: 3. 返回消息
        PT->>Handler: 4. dispatch(message)
        Handler->>Handler: 5. 处理消息
        Handler->>EB: 6. publish(event)
        EB->>Subscribers: 7. notify(event)
    end
```

### 1.3 错误处理流程

```mermaid
sequenceDiagram
    participant Source as 错误源
    participant MQ as 消息队列
    participant EH as 错误处理器
    participant SM as 状态机
    participant EB as 事件总线
    participant Logger as 日志系统

    Source->>MQ: 1. 放入错误消息
    MQ->>EH: 2. 消费错误消息
    EH->>EH: 3. 分析错误

    alt 可恢复错误
        EH->>SM: 4a. 触发恢复流程
        SM->>EB: 5a. 发布恢复事件
    else 不可恢复错误
        EH->>SM: 4b. 触发重置流程
        SM->>EB: 5b. 发布重置事件
    end

    EH->>Logger: 6. 记录错误信息
```

## 2. 消息类型定义

系统中存在多种类型的消息，每种类型有不同的处理流程。

### 2.1 命令消息 (CommandMessage)

用户通过CLI输入的命令转换为命令消息进入系统处理流程。

```cpp
struct CommandMessage : public Message {
    std::string command;      // 命令名称
    CommandArgs arguments;    // 命令参数
    MessageId correlationId;  // 关联ID
};
```

### 2.2 导航请求消息 (NavRequestMessage)

从处理线程发往网络层的导航请求。

```cpp
struct NavRequestMessage : public Message {
    NavRequestType type;      // 请求类型(START/STOP/PAUSE等)
    std::string targetId;     // 目标位置
    PathPlanOptions options;  // 路径规划选项
};
```

### 2.3 导航响应消息 (NavResponseMessage)

从网络层返回的导航操作结果。

```cpp
struct NavResponseMessage : public Message {
    NavResponseType type;     // 响应类型
    NavStatus status;         // 导航状态
    ErrorCode errorCode;      // 错误代码(如果有)
    Position currentPosition; // 当前位置
};
```

### 2.4 状态更新消息 (StatusUpdateMessage)

用于通知系统状态变化的消息。

```cpp
struct StatusUpdateMessage : public Message {
    NavState previousState;   // 之前状态
    NavState currentState;    // 当前状态
    std::string reason;       // 状态变化原因
    Timestamp timestamp;      // 时间戳
};
```

### 2.5 错误消息 (ErrorMessage)

系统中发生错误时产生的消息。

```cpp
struct ErrorMessage : public Message {
    ErrorCode code;           // 错误代码
    ErrorSeverity severity;   // 错误严重程度
    std::string description;  // 错误描述
    std::string source;       // 错误来源
};
```

## 3. 消息队列实现

系统使用线程安全的消息队列实现异步消息处理。

### 3.1 消息队列类图

```mermaid
classDiagram
    class Message {
        +MessageType type
        +MessageId id
        +Timestamp createdAt
        +MessagePriority priority
    }

    class MessageQueue {
        -Queue<Message> queue
        -Mutex mutex
        -ConditionVariable condition
        +enqueue(Message)
        +dequeue() Message
        +tryDequeue() Message
        +size() int
        +isEmpty() bool
    }

    class PriorityMessageQueue {
        +enqueue(Message, Priority)
        +dequeueHighestPriority() Message
    }

    Message <|-- CommandMessage
    Message <|-- NavRequestMessage
    Message <|-- NavResponseMessage
    Message <|-- StatusUpdateMessage
    Message <|-- ErrorMessage

    MessageQueue <|-- PriorityMessageQueue
```

### 3.2 消息分发器实现

```mermaid
classDiagram
    class MessageDispatcher {
        -Map<MessageType, MessageHandler> handlers
        +registerHandler(MessageType, MessageHandler)
        +unregisterHandler(MessageType)
        +dispatch(Message)
    }

    class MessageHandler {
        <<interface>>
        +handleMessage(Message)
    }

    class CommandMessageHandler {
        +handleMessage(CommandMessage)
    }

    class NavRequestMessageHandler {
        +handleMessage(NavRequestMessage)
    }

    class NavResponseMessageHandler {
        +handleMessage(NavResponseMessage)
    }

    class StatusUpdateMessageHandler {
        +handleMessage(StatusUpdateMessage)
    }

    class ErrorMessageHandler {
        +handleMessage(ErrorMessage)
    }

    MessageHandler <|.. CommandMessageHandler
    MessageHandler <|.. NavRequestMessageHandler
    MessageHandler <|.. NavResponseMessageHandler
    MessageHandler <|.. StatusUpdateMessageHandler
    MessageHandler <|.. ErrorMessageHandler

    MessageDispatcher --> MessageHandler
```

## 4. 事件总线机制

事件总线负责将消息处理结果以事件形式通知给相关组件。

### 4.1 事件总线类图

```mermaid
classDiagram
    class Event {
        +EventType type
        +EventId id
        +Timestamp timestamp
    }

    class EventBus {
        -Map<EventType, List<EventListener>> subscribers
        -Mutex mutex
        +subscribe(EventType, EventListener)
        +unsubscribe(EventType, EventListener)
        +publish(Event)
    }

    class EventListener {
        <<interface>>
        +onEvent(Event)
    }

    class NavStateChangeListener {
        +onEvent(NavStateChangeEvent)
    }

    class ErrorEventListener {
        +onEvent(ErrorEvent)
    }

    class CommandResultListener {
        +onEvent(CommandResultEvent)
    }

    Event <|-- NavStateChangeEvent
    Event <|-- ErrorEvent
    Event <|-- CommandResultEvent

    EventListener <|.. NavStateChangeListener
    EventListener <|.. ErrorEventListener
    EventListener <|.. CommandResultListener

    EventBus --> EventListener
```

### 4.2 事件发布-订阅流程

```mermaid
sequenceDiagram
    participant Publisher as 事件发布者
    participant EB as 事件总线
    participant Subscriber1 as 订阅者1
    participant Subscriber2 as 订阅者2

    Note over Publisher,EB: 订阅阶段
    Subscriber1->>EB: subscribe(EVENT_TYPE_A)
    Subscriber2->>EB: subscribe(EVENT_TYPE_A)
    Subscriber2->>EB: subscribe(EVENT_TYPE_B)

    Note over Publisher,EB: 发布阶段
    Publisher->>EB: publish(EventA)
    EB->>Subscriber1: onEvent(EventA)
    EB->>Subscriber2: onEvent(EventA)

    Publisher->>EB: publish(EventB)
    EB->>Subscriber2: onEvent(EventB)
```

## 5. 网络通信模型

系统提供两种网络实现模型，以下展示了消息在网络层的处理流程。

### 5.1 Boost.Asio模型消息流程

```mermaid
sequenceDiagram
    participant PT as 处理线程
    participant TM as 传输管理器
    participant TC as TCP客户端
    participant IO as IO服务
    participant CH as 回调处理器
    participant MQ as 消息队列

    PT->>TM: 发送消息
    TM->>TC: 序列化并发送
    TC->>IO: async_write()

    Note over IO: 异步IO操作

    IO->>CH: 写入完成回调

    IO->>CH: 读取完成回调
    CH->>TM: 处理接收的数据
    TM->>MQ: 放入响应消息
```

### 5.2 Epoll模型消息流程

```mermaid
sequenceDiagram
    participant PT as 处理线程
    participant TM as 传输管理器
    participant ES as Epoll服务器
    participant EL as 事件循环
    participant ES as Epoll套接字
    participant MH as 消息处理器
    participant MQ as 消息队列

    PT->>TM: 发送消息
    TM->>ES: 序列化并发送

    loop 事件循环
        EL->>ES: epoll_wait()
        ES-->>EL: 就绪事件

        alt 可读事件
            EL->>MH: 处理接收数据
            MH->>MQ: 放入响应消息
        else 可写事件
            EL->>ES: 发送数据
        end
    end
```

## 6. 消息数据流向总览

下图展示了系统中消息从输入到输出的完整流转路径：

```mermaid
flowchart TD
    subgraph Input
        CMD[用户命令] --> CLI
        NET[网络响应] --> NC
    end

    subgraph Processing
        CLI[命令行界面] --> MQ
        NC[网络客户端] --> MQ
        MQ[消息队列] --> PT
        PT[处理线程] --> MD[消息分发器]
        MD --> CH[命令处理器]
        MD --> NH[导航处理器]
        MD --> EH[错误处理器]

        CH --> SM
        NH --> SM
        EH --> SM

        SM[状态机] --> AM[动作管理器]

        AM --> NM[导航管理器]
        AM --> CM[配置管理器]
        AM --> LM[日志管理器]
    end

    subgraph Output
        NM --> NC
        NM --> EB
        CM --> EB
        LM --> LOG

        EB[事件总线] --> UI[用户界面]
        LOG[日志系统] --> FS[文件系统]
    end

    NC --> TD[TCP数据]
    TD --> RD[机器狗设备]
```

## 7. 消息处理最佳实践

为确保消息处理的高效和可靠，系统遵循以下最佳实践：

1. **消息优先级**：关键消息（如错误消息）具有更高的处理优先级
2. **消息过期机制**：超过一定时间未处理的消息会被标记为过期
3. **消息去重**：检测并合并短时间内的重复消息
4. **消息追踪**：每个消息有唯一ID，便于跟踪和调试
5. **批量处理**：同类型消息可进行批量处理以提高效率
6. **背压机制**：当消息积压时，采取措施减缓生产速度
7. **消息持久化**：关键消息会被持久化，系统崩溃后可恢复
8. **消息验证**：所有消息在入队前进行验证，确保格式正确
