#pragma once

#include <string>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// 声明 extractFileName 函数
std::string extractFileName(const std::string& path);

// 定义日志类，使用懒汉式单例模式（C++11 及以后自动线程安全）
class Logger {
private:
    // 控制台日志记录器
    std::shared_ptr<spdlog::logger> consoleLogger;

    // 私有构造函数，防止外部实例化
    Logger()
    {
        // 创建控制台日志记录器
        consoleLogger = spdlog::stdout_color_mt("console");
    }

    // 删除拷贝构造函数和赋值运算符，防止复制
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

public:
    // 获取唯一实例的静态方法
    static Logger& getInstance()
    {
        static Logger instance;
        return instance;
    }

    // 封装 info 级别的日志记录函数，支持格式化
    template<typename... Args>
    void info(const char* file, int line, const char* fmt, Args&&... args) {
        if (consoleLogger) {
            std::string fileName = extractFileName(file);
            consoleLogger->info("[{}:{}] " + std::string(fmt), fileName, line, std::forward<Args>(args)...);
        }
    }

    // 实现 warn 级别的日志记录函数，支持格式化
    template<typename... Args>
    void warn(const char* file, int line, const char* fmt, Args&&... args) {
        if (consoleLogger) {
            std::string fileName = extractFileName(file);
            consoleLogger->warn("[{}:{}] " + std::string(fmt), fileName, line, std::forward<Args>(args)...);
        }
    }

    // 实现 error 级别的日志记录函数，支持格式化
    template<typename... Args>
    void error(const char* file, int line, const char* fmt, Args&&... args) {
        if (consoleLogger) {
            std::string fileName = extractFileName(file);
            consoleLogger->error("[{}:{}] " + std::string(fmt), fileName, line, std::forward<Args>(args)...);
        }
    }
};

// 定义宏，自动传递文件名和行号
#define LOG_INFO(...) Logger::getInstance().info(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) Logger::getInstance().warn(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) Logger::getInstance().error(__FILE__, __LINE__, __VA_ARGS__)
