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

// TODO: 巡检应用程序打印方式
// 开始巡检任务
// 正在前往点位 {Value}， 目标点类型: {过渡点/任务点/站立点}，点位坐标: [PosX, PosY, PosZ]...
// 当前位置坐标: [PosX, PosY, PosZ]，累计里程数：{SumOdom}, 机器人定位状态: {Location}
// 当前位置坐标: [PosX, PosY, PosZ]，累计里程数：{SumOdom}, 机器人定位状态: {Location}
// 当前位置坐标: [PosX, PosY, PosZ]，累计里程数：{SumOdom}, 机器人定位状态: {Location}
// ...............................................................
// 到达点位 {Value}

// 正在前往点位 {Value}， 目标点类型: {过渡点/任务点/站立点}，点位坐标: [PosX, PosY, PosZ]...
// 当前位置坐标: [PosX, PosY, PosZ]，累计里程数：{SumOdom}, 机器人定位状态: {Location}
// 当前位置坐标: [PosX, PosY, PosZ]，累计里程数：{SumOdom}, 机器人定位状态: {Location}
// 当前位置坐标: [PosX, PosY, PosZ]，累计里程数：{SumOdom}, 机器人定位状态: {Location}
// ...............................................................
// 到达点位 {Value}

// 正在前往点位 {Value}， 目标点类型: {过渡点/任务点/站立点}，点位坐标: [PosX, PosY, PosZ]...
// 当前位置坐标: [PosX, PosY, PosZ]，累计里程数：{SumOdom}, 机器人定位状态: {Location}
// 当前位置坐标: [PosX, PosY, PosZ]，累计里程数：{SumOdom}, 机器人定位状态: {Location}
// 当前位置坐标: [PosX, PosY, PosZ]，累计里程数：{SumOdom}, 机器人定位状态: {Location}
// ...............................................................
// 到达点位 {Value}

// 巡检任务结束
class InspectionApp {
public:

    explicit InspectionApp() : system_(std::make_unique<application::X30InspectionSystem>()) {}
    ~InspectionApp() = default;

    // 初始化应用程序
    bool initialize(const std::string& host, uint16_t port) {
        setupCallbacks();
        setupEventHandlers(); // TODO: 新增业务逻辑

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
        system_->setCallback(callback);
    }

    // 设置事件处理器
    void setupEventHandlers() {
        // 订阅状态查询事件
        common::EventBus::getInstance().subscribe<common::QueryStatusEvent>(
            [](const std::shared_ptr<common::Event>& event) {
                // std::cout << "callback by EventBus" << std::endl;
                auto queryEvent = std::static_pointer_cast<common::QueryStatusEvent>(event);
                if (queryEvent->status == protocol::NavigationStatus::EXECUTING) {
                    static int lastValue = 0;
                    if (lastValue != queryEvent->value) {
                        lastValue = queryEvent->value;
                        // 正在前往点位 {Value}， 目标点类型: {过渡点/任务点/站立点}，点位坐标: [PosX, PosY, PosZ]...
                        auto&& point = common::getNavigationPointByValue(queryEvent->value);
                        std::string pointType = point.pointInfo == 0 ? "过渡点" :
                            point.pointInfo == 1 ? "任务点" : 
                            point.pointInfo == 2 ? "站立点" : 
                            point.pointInfo == 3 ? "充电点" : "未知";
                        std::cout << fmt::format("正在前往点位 {}， 目标点类型: {}，点位坐标: [{}, {}, {}]...",
                        queryEvent->value, pointType, point.posX, point.posY, point.posZ) << std::endl;
                    }
                }
            }
        );

        // 订阅实时状态事件
        common::EventBus::getInstance().subscribe<common::GetRealTimeStatusEvent>(
            [](const std::shared_ptr<common::Event>& event) {
                auto realTimeEvent = std::static_pointer_cast<common::GetRealTimeStatusEvent>(event);
                // 当前位置坐标: [PosX, PosY, PosZ]，累计里程数：{SumOdom}, 机器人定位状态: {Location}
                std::cout << fmt::format("当前位置坐标: [{}, {}, {}]，累计里程数：{}, 机器人定位状态: {}",
                realTimeEvent->posX, realTimeEvent->posY, realTimeEvent->posZ, realTimeEvent->sumOdom, realTimeEvent->location) << std::endl;
            }
        );

        // 订阅导航任务响应事件
        common::EventBus::getInstance().subscribe<common::MessageResponseEvent>(
            [](const std::shared_ptr<common::Event>& event) {
                auto respEvent = std::static_pointer_cast<common::MessageResponseEvent>(event);
                std::cout << "收到响应: " << respEvent->data <<
                    (respEvent->success ? " (成功)" : " (失败)") << std::endl;
            }
        );

        // 订阅连接状态事件
        common::EventBus::getInstance().subscribe<common::ConnectionStatusEvent>(
            [](const std::shared_ptr<common::Event>& event) {
                auto connEvent = std::static_pointer_cast<common::ConnectionStatusEvent>(event);
                std::cout << "连接状态: " <<
                    (connEvent->connected ? "已连接" : "已断开") <<
                    " - " << connEvent->message << std::endl;
            }
        );

        // 订阅导航状态事件
        common::EventBus::getInstance().subscribe<common::NavigationStatusEvent>(
            [](const std::shared_ptr<common::Event>& event) {
                auto navEvent = std::static_pointer_cast<common::NavigationStatusEvent>(event);
                std::cout << "导航状态: " << navEvent->status <<
                    (navEvent->completed ? " (已完成)" : "") <<
                    " - 当前点: " << navEvent->currentPoint << std::endl;
            }
        );

        // 订阅错误事件
        common::EventBus::getInstance().subscribe<common::ErrorEvent>(
            [](const std::shared_ptr<common::Event>& event) {
                auto errorEvent = std::static_pointer_cast<common::ErrorEvent>(event);
                std::cout << "错误事件: [" << errorEvent->code << "] " <<
                    errorEvent->message << std::endl;
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
