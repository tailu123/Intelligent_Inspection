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

        if (!system_.initialize(host, port)) {
            std::cout << "系统初始化失败\n";
            return false;
        }

        std::cout << "系统已初始化，等待连接...\n";
        waitForConnection();
        std::cout << "已连接到设备\n";

        loadNavigationPoints();
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

    // 等待连接建立
    void waitForConnection() {
        while (!system_.isConnected()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    // 加载导航点
    void loadNavigationPoints() {
        std::filesystem::path exePath = std::filesystem::canonical("/proc/self/exe");
        std::filesystem::path projectRoot = exePath.parent_path().parent_path();
        std::filesystem::path configPath = projectRoot / "config" / "default_params.json";

        points_ = protocol::loadDefaultNavigationPoints(configPath.string());
        if (points_.empty()) {
            std::cout << "警告: 未能加载默认导航点，将使用示例导航点\n";
            points_ = {
                {0, 1, -4.2181582, 3.4758759, -0.056337897, -3.044234, 0, 0, 1, 0, 0, 0, 0, 0},
                {0, 2, -9.1335344, 2.9462891, 0.093159825, -1.4948614, 0, 0, 1, 0, 0, 0, 0, 0}
            };
        }
    }

    // 处理用户命令
    CommandResult handleCommand(const std::string& command) {
        if (command == "start") {
            if (system_.startInspection(points_)) {
                std::cout << "正在启动巡检任务...\n";
            }
            return CommandResult::CONTINUE;
        }
        else if (command == "cancel") {
            if (system_.cancelInspection()) {
                std::cout << "正在取消巡检任务...\n";
            }
            return CommandResult::CONTINUE;
        }
        else if (command == "status") {
            if (system_.queryStatus()) {
                std::cout << "正在查询状态...\n";
            }
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
                  << "4. quit - 退出程序\n";
    }

private:
    application::X30InspectionSystem system_;
    std::vector<protocol::NavigationPoint> points_;
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