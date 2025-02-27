#pragma once
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <iostream>

namespace common {

class Logger {
public:
    static void init(const std::string& log_path = "logs/x30_inspection.log") {
        try {
            // 创建控制台sink
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::info);

            // 创建文件sink（滚动日志）
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                log_path, 1024 * 1024 * 10, 3  // 10MB文件大小，保留3个文件
            );
            file_sink->set_level(spdlog::level::debug);

            // 创建logger
            std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
            auto logger = std::make_shared<spdlog::logger>("x30", sinks.begin(), sinks.end());

            // 设置格式
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
            logger->set_level(spdlog::level::debug);

            // 设置为默认logger
            spdlog::set_default_logger(logger);

            spdlog::info("Logger initialized");
        }
        catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        }
    }

    static void shutdown() {
        spdlog::shutdown();
    }
};

}  // namespace common