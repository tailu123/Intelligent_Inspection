# 公共组件模块

## 模块概述

公共组件模块提供系统所需的基础设施和工具类，包括日志管理、配置管理、工具类和公共接口等。该模块为其他模块提供基础支持服务。

## 核心组件

### 1. 日志管理 (Logger)

```cpp
class Logger {
public:
    static Logger& getInstance();

    void setLevel(LogLevel level);
    void setOutput(const std::string& path);

    void debug(const char* format, ...);
    void info(const char* format, ...);
    void warn(const char* format, ...);
    void error(const char* format, ...);
    void fatal(const char* format, ...);

private:
    LogLevel level_;
    std::ofstream output_;
    std::mutex mutex_;
};
```

### 2. 配置管理 (Config)

```cpp
class Config {
public:
    static Config& getInstance();

    bool load(const std::string& path);
    bool save(const std::string& path);

    template<typename T>
    T getValue(const std::string& key, const T& default_value = T());

    template<typename T>
    void setValue(const std::string& key, const T& value);

private:
    nlohmann::json config_data_;
    std::mutex mutex_;
};
```

### 3. 事件总线 (EventBus)

```cpp
class EventBus {
public:
    static EventBus& getInstance();

    template<typename EventType>
    void subscribe(const std::function<void(const EventType&)>& handler);

    template<typename EventType>
    void publish(const EventType& event);

    void unsubscribe(const std::string& event_type);

private:
    std::map<std::string, std::vector<std::function<void(const void*)>>> handlers_;
    std::mutex mutex_;
};
```

## 工具类

### 1. 时间工具 (TimeUtils)

```cpp
namespace TimeUtils {
    // 时间戳操作
    uint64_t getCurrentTimestamp();
    std::string formatTimestamp(uint64_t timestamp);
    uint64_t parseTimestamp(const std::string& time_str);

    // 时间计算
    uint64_t addSeconds(uint64_t timestamp, int seconds);
    uint64_t addMinutes(uint64_t timestamp, int minutes);
    uint64_t addHours(uint64_t timestamp, int hours);

    // 时间比较
    bool isExpired(uint64_t timestamp);
    int64_t getTimeDiff(uint64_t t1, uint64_t t2);
}
```

### 2. 字符串工具 (StringUtils)

```cpp
namespace StringUtils {
    // 字符串操作
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string trim(const std::string& str);
    std::string toUpper(const std::string& str);
    std::string toLower(const std::string& str);

    // 格式化
    std::string format(const char* fmt, ...);
    std::string formatBytes(size_t bytes);

    // 转换
    template<typename T>
    std::string toString(const T& value);

    template<typename T>
    T fromString(const std::string& str);
}
```

### 3. 文件工具 (FileUtils)

```cpp
namespace FileUtils {
    // 文件操作
    bool exists(const std::string& path);
    bool createDirectory(const std::string& path);
    bool removeFile(const std::string& path);

    // 文件读写
    std::string readFile(const std::string& path);
    bool writeFile(const std::string& path, const std::string& content);

    // 路径操作
    std::string getFileName(const std::string& path);
    std::string getFileExtension(const std::string& path);
    std::string getDirectory(const std::string& path);
}
```

## 内存管理

### 1. 内存池 (MemoryPool)

```cpp
template<typename T, size_t BlockSize = 4096>
class MemoryPool {
public:
    T* allocate();
    void deallocate(T* ptr);

    size_t getUsedBlocks() const;
    size_t getFreeBlocks() const;

private:
    std::vector<T*> free_blocks_;
    std::vector<std::unique_ptr<T[]>> allocated_blocks_;
    std::mutex mutex_;
};
```

### 2. 智能指针工具 (SmartPtrUtils)

```cpp
namespace SmartPtrUtils {
    template<typename T>
    using Ptr = std::shared_ptr<T>;

    template<typename T>
    using WeakPtr = std::weak_ptr<T>;

    template<typename T>
    using UniquePtr = std::unique_ptr<T>;

    template<typename T, typename... Args>
    Ptr<T> makeShared(Args&&... args);

    template<typename T, typename... Args>
    UniquePtr<T> makeUnique(Args&&... args);
}
```

## 线程管理

### 1. 线程池 (ThreadPool)

```cpp
class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads);

    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))>;

    void setThreadCount(size_t count);
    size_t getActiveThreadCount() const;

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable condition_;
    bool stop_;
};
```

### 2. 任务队列 (TaskQueue)

```cpp
template<typename T>
class TaskQueue {
public:
    void push(const T& task);
    bool pop(T& task);
    bool tryPop(T& task);

    size_t size() const;
    bool empty() const;

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
};
```

## 错误处理

### 1. 错误码定义

```cpp
enum class ErrorCode {
    SUCCESS = 0,
    INVALID_ARGUMENT,
    FILE_NOT_FOUND,
    PERMISSION_DENIED,
    TIMEOUT,
    NETWORK_ERROR,
    UNKNOWN_ERROR
};
```

### 2. 错误处理工具

```cpp
class Error {
public:
    Error(ErrorCode code, const std::string& message);

    ErrorCode getCode() const;
    std::string getMessage() const;
    std::string getStackTrace() const;

    static Error success();
    static Error fromException(const std::exception& e);

private:
    ErrorCode code_;
    std::string message_;
    std::string stack_trace_;
};
```

## 配置示例

### 1. 日志配置

```json
{
    "logger": {
        "level": "INFO",
        "output": "logs/app.log",
        "max_size": "100M",
        "max_files": 10,
        "format": "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"
    }
}
```

### 2. 线程池配置

```json
{
    "thread_pool": {
        "min_threads": 4,
        "max_threads": 16,
        "queue_size": 1000,
        "keep_alive_time": 60
    }
}
```

## 最佳实践

1. **内存管理**
   - 使用智能指针
   - 避免内存泄漏
   - 使用内存池
   - RAII原则

2. **错误处理**
   - 统一错误码
   - 异常处理
   - 日志记录
   - 错误追踪

3. **性能优化**
   - 资源复用
   - 延迟初始化
   - 批量处理
   - 并发控制
