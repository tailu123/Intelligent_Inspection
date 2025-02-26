#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace protocol {
struct NavigationPoint;
} // namespace protocol

namespace common {

// 辅助函数：获取当前时间戳
std::string getCurrentTimestamp();

std::vector<protocol::NavigationPoint> loadDefaultNavigationPoints(const std::string& configPath);

// 辅助函数：加载默认导航点
std::vector<protocol::NavigationPoint> loadNavigationPoints();

// 辅助函数：加载导航点map
std::unordered_map<int, protocol::NavigationPoint> loadNavigationPointsMap();

// 转换目标点类型
std::string convertPointType(int pointType);
} // namespace common
