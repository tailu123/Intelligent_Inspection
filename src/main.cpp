#include "application/x30_inspection_system.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <optional>

namespace x30 {

// 命令处理结果
enum class CommandResult {
    SUCCESS,
    CONTINUE,
    EXIT
};

class InspectionApp {
public:
    // 初始化应用程序
    bool initialize(const std::string& host, uint16_t port) {
        setupCallbacks();
        setupEventHandlers();

        if (!system_.initialize(host, port)) {
            std::cout << "系统初始化失败\n";
            return false;
        }

        std::cout << "系统已初始化，等待连接...\n";
        waitForConnection();
        std::cout << "已连接到设备\n";

        return true;
    }

    // 运行应用程序
    void run() {
        printHelp();
        std::string command;

        while (true) {
            std::cout << "\n请输入命令: ";
            std::getline(std::cin, command);

            auto result = handleCommand(command);
            if (result == CommandResult::EXIT) {
                break;
            }
        }

        shutdown();
    }

private:
    // 设置回调函数
    void setupCallbacks() {
        application::InspectionCallback callback;
        callback.onStarted = []() {
            std::cout << "巡检任务已启动\n";
        };
        callback.onCompleted = []() {
            std::cout << "巡检任务已完成\n";
        };
        callback.onError = [](int code, const std::string& msg) {
            std::cout << "错误 [" << code << "]: " << msg << "\n";
        };
        callback.onStatusUpdate = [](const std::string& status) {
            std::cout << "状态更新: " << status << "\n";
        };
        system_.setCallback(callback);
    }

    // 设置事件处理器
    void setupEventHandlers() {
        // 订阅导航任务响应事件
        system_.subscribeEvent<application::MessageResponseEvent>(
            [](const std::shared_ptr<application::Event>& event) {
                auto respEvent = std::static_pointer_cast<application::MessageResponseEvent>(event);
                std::cout << "收到响应: " << respEvent->data <<
                    (respEvent->success ? " (成功)" : " (失败)") << std::endl;
            }
        );

        // 订阅连接状态事件
        system_.subscribeEvent<application::ConnectionStatusEvent>(
            [](const std::shared_ptr<application::Event>& event) {
                auto connEvent = std::static_pointer_cast<application::ConnectionStatusEvent>(event);
                std::cout << "连接状态: " <<
                    (connEvent->connected ? "已连接" : "已断开") <<
                    " - " << connEvent->message << std::endl;
            }
        );

        // 订阅导航状态事件
        system_.subscribeEvent<application::NavigationStatusEvent>(
            [](const std::shared_ptr<application::Event>& event) {
                auto navEvent = std::static_pointer_cast<application::NavigationStatusEvent>(event);
                std::cout << "导航状态: " << navEvent->status <<
                    (navEvent->completed ? " (已完成)" : "") <<
                    " - 当前点: " << navEvent->currentPoint << std::endl;
            }
        );

        // 订阅错误事件
        system_.subscribeEvent<application::ErrorEvent>(
            [](const std::shared_ptr<application::Event>& event) {
                auto errorEvent = std::static_pointer_cast<application::ErrorEvent>(event);
                std::cout << "错误事件: [" << errorEvent->code << "] " <<
                    errorEvent->message << std::endl;
            }
        );
    }

    // 等待连接建立
    void waitForConnection() {
        while (!system_.isConnected()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    // 处理用户命令
    CommandResult handleCommand(const std::string& command) {
        if (command == "start") {
            system_.handleCommand(std::make_unique<protocol::NavigationTaskRequest>());
            return CommandResult::CONTINUE;
        }
        else if (command == "cancel") {
            system_.handleCommand(std::make_unique<protocol::CancelTaskRequest>());
            return CommandResult::CONTINUE;
        }
        else if (command == "status") {
            system_.handleCommand(std::make_unique<protocol::QueryStatusRequest>());
            return CommandResult::CONTINUE;
        }
        else if (command == "quit") {
            return CommandResult::EXIT;
        }
        else if (command == "help") {
            printHelp();
            return CommandResult::CONTINUE;
        }
        else {
            std::cout << "未知命令，输入 'help' 查看可用命令\n";
            return CommandResult::CONTINUE;
        }
    }

    // 关闭应用程序
    void shutdown() {
        system_.shutdown();
        std::cout << "系统已关闭\n";
    }

    // 打印帮助信息
    static void printHelp() {
        std::cout << "可用命令：\n"
                  << "1. start - 开始巡检任务\n"
                  << "2. cancel - 取消巡检任务\n"
                  << "3. status - 查询状态\n"
                  << "4. help - 显示此帮助信息\n"
                  << "5. quit - 退出程序\n";
    }

private:
    application::X30InspectionSystem system_;
};

} // namespace x30

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cout << "用法: " << argv[0] << " <host> <port>\n";
            return 1;
        }

        x30::InspectionApp app;
        std::string host = argv[1];
        uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));

        if (!app.initialize(host, port)) {
            return 1;
        }

        app.run();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
        return 1;
    }
}