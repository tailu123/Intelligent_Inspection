#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "protocol/x30_protocol.hpp"

namespace x30 {
namespace application {

class MessageQueue {
public:
    void push(std::unique_ptr<protocol::IMessage> msg);
    std::unique_ptr<protocol::IMessage> pop();
    void clear();


private:
    std::queue<std::unique_ptr<protocol::IMessage>> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

} // namespace application
} // namespace x30