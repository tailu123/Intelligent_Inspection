#include "common/message_queue.hpp"

namespace common {

// TODO: 可以考虑多模型消息队列, 比如无锁队列
void MessageQueue::push(std::unique_ptr<protocol::IMessage> msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(std::move(msg));
    cv_.notify_one();
}

std::unique_ptr<protocol::IMessage> MessageQueue::pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !queue_.empty(); });
    auto msg = std::move(queue_.front());
    queue_.pop();
    return msg;
}

void MessageQueue::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    while (!queue_.empty()) {
        queue_.pop();
    }
}
} // namespace common
