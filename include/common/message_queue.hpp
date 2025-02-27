#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "protocol/x30_protocol.hpp"
#include <atomic>

namespace common {

// 消息队列目的
// 1. 解耦：消息发送者和消息处理器解耦
// 2. 线程安全：使用互斥锁和条件变量保证线程安全
class MessageQueue {
public:
    MessageQueue() = default;
    ~MessageQueue() = default;
    void push(std::unique_ptr<protocol::IMessage> msg);
    std::unique_ptr<protocol::IMessage> pop();
    void clear();
    void close();

private:
    const MessageQueue& operator=(const MessageQueue&) = delete;
    MessageQueue(const MessageQueue&) = delete;

    std::queue<std::unique_ptr<protocol::IMessage>> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> close_flag_{false};
};

} // namespace common
