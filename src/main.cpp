#include "application/x30_inspection_system.hpp"
#include <iostream>
#include <ostream>
#include <thread>
#include <chrono>
#include "common/event_bus.hpp"
#include "protocol/x30_protocol.hpp"
#include <fmt/core.h>
#include "common/utils.hpp"
namespace x30 {

// 命令处理结果
enum class CommandResult {
    SUCCESS,
    CONTINUE,
    EXIT
};

std::unordered_map<int, protocol::NavigationPoint> point_map_ = common::loadNavigationPointsMap();

class InspectionApp {
public:

    explicit InspectionApp() : system_(std::make_unique<application::X30InspectionSystem>()) {}
    ~InspectionApp() = default;

    // 初始化应用程序
    bool initialize(const std::string& host, uint16_t port) {
        setupEventHandlers();

        if (!system_->initialize(host, port)) {
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
            std::cout << "\n请输入命令:\n";
            std::getline(std::cin, command);

            auto result = handleCommand(command);
            if (result == CommandResult::EXIT) {
                break;
            }
        }
    }

private:
    // 设置事件处理器
    void setupEventHandlers() {
        // 订阅网络错误事件
        common::EventBus::getInstance().subscribe<common::NetworkErrorEvent>([](const std::shared_ptr<common::Event>& event) {
            auto errorEvent = std::static_pointer_cast<common::NetworkErrorEvent>(event);
            std::cout << fmt::format("[{}]: 网络错误: {}, 请检查网络连接, 程序需要重新启动", common::getCurrentTimestamp(), errorEvent->message) << std::endl;
        });

        // 订阅导航任务事件
        common::EventBus::getInstance().subscribe<common::NavigationTaskEvent>(
            [](const std::shared_ptr<common::Event>& event) {
                auto taskEvent = std::static_pointer_cast<common::NavigationTaskEvent>(event);
                std::cout << fmt::format("[{}]: 导航任务执行状态: {}", common::getCurrentTimestamp(), taskEvent->status) << std::endl;
            }
        );

        // 订阅状态查询消息响应事件
        common::EventBus::getInstance().subscribe<common::QueryStatusEvent>(
            [](const std::shared_ptr<common::Event>& event) {
                auto queryEvent = std::static_pointer_cast<common::QueryStatusEvent>(event);
                if (queryEvent->status == protocol::NavigationStatus::EXECUTING) {
                    static int lastValue = 0;
                    if (lastValue != queryEvent->value) {
                        lastValue = queryEvent->value;
                        auto&& point = point_map_[queryEvent->value];
                        std::cout << fmt::format("[{}]: 正在前往点位 {}， 目标点类型: {}，点位坐标: [{}, {}, {}]...",
                        queryEvent->timestamp, queryEvent->value, point.pointInfo, point.posX, point.posY, point.posZ) << std::endl;
                    }
                }
            }
        );

        // 订阅实时状态消息响应事件
        common::EventBus::getInstance().subscribe<common::GetRealTimeStatusEvent>(
            [](const std::shared_ptr<common::Event>& event) {
                auto realTimeEvent = std::static_pointer_cast<common::GetRealTimeStatusEvent>(event);
                std::cout << fmt::format("[{}]: 当前位置坐标: [{}, {}, {}]，累计里程数：{}, 机器人定位状态: {}",
                realTimeEvent->timestamp, realTimeEvent->posX, realTimeEvent->posY, realTimeEvent->posZ, realTimeEvent->sumOdom, realTimeEvent->location) << std::endl;
            }
        );

        // 订阅错误事件
        common::EventBus::getInstance().subscribe<common::ErrorEvent>(
            [](const std::shared_ptr<common::Event>& event) {
                auto errorEvent = std::static_pointer_cast<common::ErrorEvent>(event);
                std::cout << fmt::format("[{}]: 错误 [{}]: {}", common::getCurrentTimestamp(), errorEvent->code, errorEvent->message) << std::endl;
            }
        );
    }

    // 等待连接建立
    void waitForConnection() {
        while (!system_->isConnected()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    // 处理用户命令
    CommandResult handleCommand(const std::string& command) {
        if (command == "start" || command == "cancel" || command == "status") {
            system_->handleCommand(command);
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
    std::unique_ptr<application::X30InspectionSystem> system_;
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
