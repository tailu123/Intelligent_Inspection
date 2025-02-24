#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "protocol/x30_protocol.hpp"

namespace common {

class MessageQueue {
public:
    MessageQueue() = default;
    ~MessageQueue() = default;
    void push(std::unique_ptr<protocol::IMessage> msg);
    std::unique_ptr<protocol::IMessage> pop();
    void clear();

private:
    const MessageQueue& operator=(const MessageQueue&) = delete;
    MessageQueue(const MessageQueue&) = delete;

    std::queue<std::unique_ptr<protocol::IMessage>> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

} // namespace common
