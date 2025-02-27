#pragma once

#include "network/base_network_model.hpp"

namespace common {
class MessageQueue;
}  // namespace common

namespace state {
// 导航上下文
struct NavigationContext {
    // MessageQueue保持依赖注入
    common::MessageQueue& message_queue;

    std::shared_ptr<network::BaseNetworkModel> network_model;
};

// EventBus 适合使用单例模式，因为：
// 确实需要全局唯一的事件总线
// 主要是发布/订阅模式，状态相对简单
// 通常不需要在测试中mock
// MessageQueue 建议保持当前的依赖注入方式，因为：
// 可能需要多个不同用途的消息队列
// 消息处理逻辑可能需要在测试中mock
// 队列状态管理较复杂

}  // namespace state
