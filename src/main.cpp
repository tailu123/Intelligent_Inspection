#include "application/x30_inspection_system.hpp"
#include <iostream>
#include <ostream>
#include <thread>
#include <chrono>
#include "common/event_bus.hpp"
#include "protocol/x30_protocol.hpp"
// #include <fmt/core.h>
#include "common/utils.hpp"
#include "common/logger.hpp"
// #include "common/Logger.hpp"
namespace x30 {

std::atomic<bool> program_running_(true);
// 命令处理结果
enum class CommandResult {
    SUCCESS,
    CONTINUE,
    EXIT
};

// std::unordered_map<int, protocol::NavigationPoint> point_map_ = common::loadNavigationPointsMap();

class InspectionApp {
public:

    explicit InspectionApp() : system_(std::make_unique<application::X30InspectionSystem>()) {}
    ~InspectionApp() = default;

    // 初始化应用程序
    bool initialize(const std::string& host, uint16_t port) {
        setupEventHandlers();

        if (!system_->initialize(host, port)) {
            spdlog::error("[{}]: [InspectionApp:ERR]: 系统初始化失败", common::getCurrentTimestamp());
            // std::cout << "系统初始化失败\n";
            return false;
        }

        spdlog::info("[{}]: [InspectionApp:INFO]: 系统已初始化，等待连接...", common::getCurrentTimestamp());
        // std::cout << "系统已初始化，等待连接...\n";
        waitForConnection();
        spdlog::info("[{}]: [InspectionApp:INFO]: 已连接到设备", common::getCurrentTimestamp());
        // std::cout << "已连接到设备\n";

        return true;
    }

    // 运行应用程序
    void run() {
        printHelp();
        std::string command;

        while (program_running_) {
            spdlog::info("[{}]: [InspectionApp:INFO]: 请输入命令:\n", common::getCurrentTimestamp());
            // std::cout << "\n请输入命令:\n";
            std::getline(std::cin, command);

            if (not program_running_) {
                break;
            }
            auto result = handleCommand(command);
            if (result == CommandResult::EXIT) {
                break;
            }
        }

        // 程序结束，关闭系统
        system_->shutdown();
        system_.reset();
        spdlog::info("[{}]: [InspectionApp:INFO]: 程序结束", common::getCurrentTimestamp());
    }

private:
    // 设置事件处理器
    void setupEventHandlers() {
        // 订阅网络错误事件
        common::EventBus::getInstance().subscribe<common::NetworkErrorEvent>([](const std::shared_ptr<common::Event>&) {
            // auto errorEvent = std::static_pointer_cast<common::NetworkErrorEvent>(event);
            // spdlog::error("[{}]: 网络错误: {}, 请检查网络连接, 程序需要重新启动", common::getCurrentTimestamp(), errorEvent->message);
            // std::cout << fmt::format("[{}]: 网络错误: {}, 请检查网络连接, 程序需要重新启动", common::getCurrentTimestamp(), errorEvent->message) << std::endl;
            // common::Logger::getInstance().error(__FILE__, __LINE__, "网络错误: {}, 请检查网络连接, 程序需要重新启动", errorEvent->message);
            program_running_ = false;
        });

        // 订阅导航任务事件
        common::EventBus::getInstance().subscribe<common::NavigationTaskEvent>(
            [](const std::shared_ptr<common::Event>&) {
                // auto taskEvent = std::static_pointer_cast<common::NavigationTaskEvent>(event);
                // spdlog::info("[{}]: 导航任务执行状态: {}", common::getCurrentTimestamp(), taskEvent->status);
                // std::cout << fmt::format("[{}]: 导航任务执行状态: {}", common::getCurrentTimestamp(), taskEvent->status) << std::endl;
                // common::Logger::getInstance().info(__FILE__, __LINE__, "导航任务执行状态: {}", taskEvent->status);
            }
        );

        // 订阅状态查询消息响应事件
        common::EventBus::getInstance().subscribe<common::QueryStatusEvent>(
            [](const std::shared_ptr<common::Event>&) {
                // auto queryEvent = std::static_pointer_cast<common::QueryStatusEvent>(event);
                // if (queryEvent->status == protocol::NavigationStatus::EXECUTING) {
                //     static int lastValue = 0;
                //     if (lastValue != queryEvent->value) {
                //         lastValue = queryEvent->value;
                //         auto&& point = point_map_[queryEvent->value];
                //         spdlog::info("[{}]: 正在前往点位 {}， 目标点类型: {}，点位坐标: [{}, {}, {}]  ********************************",
                //             common::getCurrentTimestamp(), queryEvent->value, common::convertPointType(point.pointInfo), point.posX, point.posY, point.posZ);
                //         // std::cout << fmt::format("[{}]: 正在前往点位 {}， 目标点类型: {}，点位坐标: [{}, {}, {}] ********************************",
                //         //     common::getCurrentTimestamp(), queryEvent->value, common::convertPointType(point.pointInfo), point.posX, point.posY, point.posZ) << std::endl;
                //         // common::Logger::getInstance().info(__FILE__, __LINE__, "正在前往点位 {}， 目标点类型: {}，点位坐标: [{}, {}, {}]", queryEvent->value, common::convertPointType(point.pointInfo), point.posX, point.posY, point.posZ);
                //     }
                // }
                // else if (queryEvent->status == protocol::NavigationStatus::FAILED) {
                //     spdlog::error("[{}]: 查询到设备运行状态异常, status: {}", common::getCurrentTimestamp(), static_cast<int>(queryEvent->status));
                //     // std::cout << fmt::format("[{}]: 查询到设备运行状态异常, status: {} ********************************",
                //     //     common::getCurrentTimestamp(), queryEvent->status) << std::endl;
                //     // common::Logger::getInstance().info(__FILE__, __LINE__, "查询到设备运行状态异常, status: {}", queryEvent->status);
                // }
                // else if (queryEvent->status == protocol::NavigationStatus::COMPLETED) {
                //     spdlog::info("[{}]: 查询到导航任务执行完成, status: {}", common::getCurrentTimestamp(), static_cast<int>(queryEvent->status));
                //     // std::cout << fmt::format("[{}]: 查询到导航任务执行完成, status: {} ********************************",
                //     //     common::getCurrentTimestamp(), queryEvent->status) << std::endl;
                //     // common::Logger::getInstance().info(__FILE__, __LINE__, "查询到导航任务执行完成, status: {}", queryEvent->status);
                // }
            }
        );

        // 订阅实时状态消息响应事件
        common::EventBus::getInstance().subscribe<common::GetRealTimeStatusEvent>(
            [](const std::shared_ptr<common::Event>&) {
                // auto realTimeEvent = std::static_pointer_cast<common::GetRealTimeStatusEvent>(event);
                // spdlog::info("[{}]: 当前位置坐标: [{}, {}, {}]，累计里程数：{}, 机器人定位状态: {}",
                //     common::getCurrentTimestamp(), realTimeEvent->posX, realTimeEvent->posY, realTimeEvent->posZ, realTimeEvent->sumOdom, realTimeEvent->location);
                // std::cout << fmt::format("[{}]: 当前位置坐标: [{}, {}, {}]，累计里程数：{}, 机器人定位状态: {}",
                // common::getCurrentTimestamp(), realTimeEvent->posX, realTimeEvent->posY, realTimeEvent->posZ, realTimeEvent->sumOdom, realTimeEvent->location) << std::endl;
                // common::Logger::getInstance().info(__FILE__, __LINE__, "当前位置坐标: [{}, {}, {}]，累计里程数：{}, 机器人定位状态: {}", realTimeEvent->posX, realTimeEvent->posY, realTimeEvent->posZ, realTimeEvent->sumOdom, realTimeEvent->location);
            }
        );

        // 订阅错误事件
        common::EventBus::getInstance().subscribe<common::ErrorEvent>(
            [](const std::shared_ptr<common::Event>&) {
                // auto errorEvent = std::static_pointer_cast<common::ErrorEvent>(event);
                // spdlog::error("[{}]: 错误 [{}]: {}", common::getCurrentTimestamp(), errorEvent->code, errorEvent->message);
                // std::cout << fmt::format("[{}]: 错误 [{}]: {}", common::getCurrentTimestamp(), errorEvent->code, errorEvent->message) << std::endl;
                // common::Logger::getInstance().info(__FILE__, __LINE__, "错误 [{}]: {}", errorEvent->code, errorEvent->message);
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
            spdlog::warn("[{}]: [InspectionApp:WRN]: 未知命令，输入 'help' 查看可用命令\n", common::getCurrentTimestamp());
            // std::cout << "未知命令，输入 'help' 查看可用命令\n";
            return CommandResult::CONTINUE;
        }
    }

    // 打印帮助信息
    static void printHelp() {
        spdlog::info("[{}]: [InspectionApp:INFO]: 可用命令：\n1. start - 开始巡检任务\n2. cancel - 取消巡检任务\n3. status - 查询状态\n4. help - 显示此帮助信息\n5. quit - 退出程序\n", common::getCurrentTimestamp());
        // std::cout << "可用命令：\n"
        //           << "1. start - 开始巡检任务\n"
        //           << "2. cancel - 取消巡检任务\n"
        //           << "3. status - 查询状态\n"
        //           << "4. help - 显示此帮助信息\n"
        //           << "5. quit - 退出程序\n";
    }

private:
    std::unique_ptr<application::X30InspectionSystem> system_;
};

} // namespace x30

int main(int argc, char* argv[]) {
    // 初始化日志
    common::Logger::init();
    
    try {
        if (argc != 3) {
            spdlog::error("[{}]: 用法: {} <host> <port>", common::getCurrentTimestamp(), argv[0]);
            // std::cout << "用法: " << argv[0] << " <host> <port>\n";
            // common::Logger::getInstance().error(__FILE__, __LINE__, "用法: {} <host> <port>", argv[0]);
            return 1;
        }

        x30::InspectionApp app;
        std::string host = argv[1];
        uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));

        if (!app.initialize(host, port)) {
            spdlog::error("[{}]: 初始化失败", common::getCurrentTimestamp());
            // common::Logger::getInstance().error(__FILE__, __LINE__, "初始化失败");
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
