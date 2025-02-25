#pragma once


// #include "protocol/x30_protocol.hpp"
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

// 辅助函数：根据导航点目标点编号获取导航点信息
protocol::NavigationPoint getNavigationPointByValue(int value);
} // namespace common
