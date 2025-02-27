#include "common/Logger.hpp"

// 实现 extractFileName 函数
std::string extractFileName(const std::string& path) {
    auto pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return path;
}