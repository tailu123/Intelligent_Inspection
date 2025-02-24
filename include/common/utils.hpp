#pragma once


// #include "protocol/x30_protocol.hpp"
#include <string>
#include <vector>


namespace protocol {
struct NavigationPoint;
} // namespace protocol

namespace common {

// 辅助函数：获取当前时间戳
std::string getCurrentTimestamp();

std::vector<protocol::NavigationPoint> loadDefaultNavigationPoints(const std::string& configPath);

// 辅助函数：加载默认导航点
std::vector<protocol::NavigationPoint> loadNavigationPoints();

// static std::vector<protocol::NavigationPoint> global_points = loadNavigationPoints();
} // namespace common
