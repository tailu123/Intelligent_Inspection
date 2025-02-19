#include "application/MessageQueue.hpp"

namespace x30 {
namespace application {
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
}   // namespace application
}   // namespace x30