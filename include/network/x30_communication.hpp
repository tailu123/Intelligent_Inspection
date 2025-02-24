#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include "common/message_queue.hpp"
#include "network/base_network_model.hpp"

namespace network {


// 异步通信管理器
class AsyncCommunicationManager {
public:
    AsyncCommunicationManager(common::MessageQueue& message_queue);
    ~AsyncCommunicationManager();

    // 启动和停止
    void start();
    void stop();

    // 获取通信实例
    std::shared_ptr<BaseNetworkModel> getCommunication();

private:
    std::unique_ptr<boost::asio::io_context> io_context_;
    // boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    common::MessageQueue& message_queue_;
    std::thread io_thread_;
    std::unique_ptr<boost::asio::io_context::work> work_;
    std::shared_ptr<BaseNetworkModel> communication_;
};

} // namespace network
