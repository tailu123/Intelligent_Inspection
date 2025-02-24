#pragma once

#include "network/base_network_model.hpp"
#include <atomic>
#include <memory>
#include <thread>
#include <queue>
#include <mutex>
#include <sys/epoll.h>

namespace common {
class MessageQueue;
}

namespace network {

class EpollNetworkModel : public BaseNetworkModel {
public:
    explicit EpollNetworkModel(common::MessageQueue& message_queue);
    ~EpollNetworkModel() override;

    bool connect(const std::string& host, uint16_t port) override;
    void disconnect() override;
    bool isConnected() const override;
    void sendMessage(const protocol::IMessage& message) override;
    void setErrorCallback(ErrorCallback callback) override;

private:
    // 初始化epoll
    bool initEpoll();
    // 事件循环线程函数
    void eventLoop();
    // 处理网络事件
    void poll();
    // 处理读事件
    bool handleRead();
    // 处理写事件
    bool handleWrite();
    // 设置非阻塞
    void setNonBlocking(int fd);

private:
    common::MessageQueue& message_queue_;
    int epoll_fd_;
    int socket_fd_;
    std::atomic<bool> connected_;
    std::atomic<bool> running_;

    std::unique_ptr<std::thread> event_thread_;
    std::queue<std::string> write_queue_;
    std::mutex write_mutex_;
    ErrorCallback error_callback_;

    static const ssize_t MAX_BUFFER_SIZE = 4096;
    static const int MAX_EVENTS = 10;
};

} // namespace network
