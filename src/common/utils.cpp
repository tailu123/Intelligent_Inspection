#include "common/utils.hpp"
#include <spdlog/spdlog.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
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

        spdlog::info("[{}]: [Utils:INFO]: 成功从配置文件加载了 {} 个导航点", common::getCurrentTimestamp(),
                     points.size());
        // std::cout << "成功从配置文件加载了 " << points.size() << " 个导航点" << std::endl;
    }
    catch (const std::exception& e) {
        spdlog::error("[{}]: [Utils:ERR]: 加载配置文件失败: {}", common::getCurrentTimestamp(), e.what());
        // std::cerr << "加载配置文件失败: " << e.what() << std::endl;
    }
    return points;
}

// 辅助函数：加载默认导航点
std::vector<protocol::NavigationPoint> loadNavigationPoints() {
    std::filesystem::path exePath = std::filesystem::canonical("/proc/self/exe");
    std::filesystem::path projectRoot = exePath.parent_path().parent_path();
    std::filesystem::path configPath = projectRoot / "config" / "default_params.json";

    static std::vector<protocol::NavigationPoint> points = loadDefaultNavigationPoints(configPath.string());
    return points;
}

// 辅助函数：加载导航点map
std::unordered_map<int, protocol::NavigationPoint> loadNavigationPointsMap() {
    auto&& points = loadNavigationPoints();
    std::unordered_map<int, protocol::NavigationPoint> pointsMap;
    for (const auto& point : points) {
        pointsMap[point.value] = point;
    }
    return pointsMap;
}

// 转换目标点类型
std::string convertPointType(int pointType) {
    switch (pointType) {
        case 0:
            return "过渡点";
        case 1:
            return "任务点";
        case 2:
            return "站立点";
        case 3:
            return "充电点";
        default:
            return "未知点";
    }
}
}  // namespace common
