#include "common/utils.hpp"
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "protocol/x30_protocol.hpp"


namespace common {

// 辅助函数：获取当前时间戳
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::vector<protocol::NavigationPoint> loadDefaultNavigationPoints(const std::string& configPath) {
    std::vector<protocol::NavigationPoint> points;
    try {
        // 检查文件是否存在
        if (!std::filesystem::exists(configPath)) {
            std::cerr << "配置文件不存在: " << configPath << std::endl;
            return points;
        }

        // 读取JSON文件
        std::ifstream file(configPath);
        nlohmann::json jsonArray;
        file >> jsonArray;

        // 解析每个导航点
        for (const auto& jsonPoint : jsonArray) {
            points.push_back(protocol::NavigationPoint::fromJson(jsonPoint));
        }

        std::cout << "成功从配置文件加载了 " << points.size() << " 个导航点" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "加载配置文件失败: " << e.what() << std::endl;
    }
    return points;
}

// 辅助函数：加载默认导航点
std::vector<protocol::NavigationPoint> loadNavigationPoints() {
    std::filesystem::path exePath = std::filesystem::canonical("/proc/self/exe");
    std::filesystem::path projectRoot = exePath.parent_path().parent_path();
    std::filesystem::path configPath = projectRoot / "config" / "default_params.json";

    std::vector<protocol::NavigationPoint> points;
    points = loadDefaultNavigationPoints(configPath.string());
    if (points.empty()) {
        std::cout << "警告: 未能加载默认导航点，将使用示例导航点\n";
        points = {
            {0, 1, -4.2181582, 3.4758759, -0.056337897, -3.044234, 0, 0, 1, 0, 0, 0, 0, 0},
            {0, 2, -9.1335344, 2.9462891, 0.093159825, -1.4948614, 0, 0, 1, 0, 0, 0, 0, 0}
        };
    }

    return points;
}
} // namespace common