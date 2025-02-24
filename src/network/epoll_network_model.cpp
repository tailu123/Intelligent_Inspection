#include "network/epoll_network_model.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include "protocol/protocol_header.hpp"
#include <algorithm>
#include <vector>
#include "common/message_queue.hpp"
namespace network {

EpollNetworkModel::EpollNetworkModel(common::MessageQueue& message_queue)
    : message_queue_(message_queue)
    , epoll_fd_(-1)
    , socket_fd_(-1)
    , connected_(false)
    , running_(false) {
}

EpollNetworkModel::~EpollNetworkModel() {
    disconnect();
}

void EpollNetworkModel::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

bool EpollNetworkModel::initEpoll() {
    // epoll_fd_ = epoll_create1(0);
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ == -1) {
        return false;
    }
    return true;
}

bool EpollNetworkModel::connect(const std::string& host, uint16_t port) {
    if (connected_) {
        return false;
    }

    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ == -1) {
        if (error_callback_) {
            error_callback_("Failed to create socket");
        }
        return false;
    }

    setNonBlocking(socket_fd_);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        close(socket_fd_);
        if (error_callback_) {
            error_callback_("Invalid address");
        }
        return false;
    }

    if (::connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        if (errno != EINPROGRESS) {
            close(socket_fd_);
            if (error_callback_) {
                error_callback_("Connect failed");
            }
            return false;
        }
    }

    if (!initEpoll()) {
        close(socket_fd_);
        if (error_callback_) {
            error_callback_("Failed to init epoll");
        }
        return false;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = socket_fd_;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket_fd_, &ev) == -1) {
        close(socket_fd_);
        close(epoll_fd_);
        if (error_callback_) {
            error_callback_("Failed to add socket to epoll");
        }
        return false;
    }

    connected_ = true;
    running_ = true;

    // 启动事件循环线程
    event_thread_ = std::make_unique<std::thread>(&EpollNetworkModel::eventLoop, this);
    return true;
}

void EpollNetworkModel::disconnect() {
    if (!connected_) {
        return;
    }

    running_ = false;
    connected_ = false;

    if (event_thread_ && event_thread_->joinable()) {
        event_thread_->join();
    }

    if (socket_fd_ != -1) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
    if (epoll_fd_ != -1) {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }
}

void EpollNetworkModel::eventLoop() {
    while (running_) {
        poll();
    }
}

void EpollNetworkModel::poll() {
    if (!connected_) {
        return;
    }

    struct epoll_event events[MAX_EVENTS];
    int nfds = epoll_wait(epoll_fd_, events, MAX_EVENTS, 100);

    if (nfds == -1) {
        if (errno != EINTR) {
            disconnect();
        }
        return;
    }

    for (int i = 0; i < nfds; ++i) {
        if (events[i].events & EPOLLIN) {
            if (!handleRead()) {
                disconnect();
                return;
            }
        }
        if (events[i].events & EPOLLOUT) {
            if (!handleWrite()) {
                disconnect();
                return;
            }
        }
    }
}

bool EpollNetworkModel::handleRead() {
    // char buffer[MAX_BUFFER_SIZE];

    while (true) {
        // 接收并验证协议头
        protocol::ProtocolHeader header;
        ssize_t n = read(socket_fd_, &header, sizeof(header));
        if (n > 0) {
            if (header.validateSyncBytes()) {
                // 读取消息体
                ssize_t body_size = header.getBodySize();
                if (body_size > 0) {
                    // 读取消息体
                    std::vector<std::uint8_t> buffer(body_size);
                    ssize_t bytes_read = 0;
                    while (bytes_read < body_size) {
                        ssize_t bytes_to_read = std::min(body_size - bytes_read, MAX_BUFFER_SIZE);
                        ssize_t bytes_read_now = read(socket_fd_, buffer.data() + bytes_read, bytes_to_read);
                        if (bytes_read_now > 0) {
                            bytes_read += bytes_read_now;
                            std::cout << "Read " << bytes_read_now << " bytes" << std::endl;
                        } else if (bytes_read_now == 0) {
                            return false;  // 连接关闭
                        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                            return false;  // 发生错误
                        }
                    }

                    // 处理消息
                    std::string message(buffer.begin(), buffer.begin() + bytes_read);
                    if (auto msg = protocol::MessageFactory::parseMessage(message)) {
                        message_queue_.push(std::move(msg));
                    } else {
                        std::cout << "Message parse failed" << std::endl;
                        return false;  // 消息解析失败
                    }
                }
            }
        } else if (n == 0) {
            return false;  // 连接关闭
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return true;  // 暂无数据
            }
            return false;  // 发生错误
        }
    }
}

bool EpollNetworkModel::handleWrite() {
    std::lock_guard<std::mutex> lock(write_mutex_);
    while (!write_queue_.empty()) {
        const std::string& data = write_queue_.front();
        ssize_t n = write(socket_fd_, data.c_str(), data.size());
        if (n > 0) {
            write_queue_.pop();
        } else if (n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return true;  // 写缓冲区已满
            }
            return false;  // 发生错误
        }
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = socket_fd_;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, socket_fd_, &ev) == -1) {
        return false;
    }
    return true;
}

void EpollNetworkModel::setErrorCallback(ErrorCallback callback) {
    error_callback_ = std::move(callback);
}

bool EpollNetworkModel::isConnected() const {
    return connected_;
}

void EpollNetworkModel::sendMessage(const protocol::IMessage& message) {
    if (!connected_) {
        return;
    }

    // 序列化消息并加入发送队列
    std::string serialized_message = message.serialize();
    {
        std::lock_guard<std::mutex> lock(write_mutex_);
        write_queue_.push(std::move(serialized_message));
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    ev.data.fd = socket_fd_;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, socket_fd_, &ev) == -1) {
        close(socket_fd_);
        close(epoll_fd_);
        if (error_callback_) {
            error_callback_("Failed to add socket to epoll");
        }
        return;
    }
}

} // namespace network
