#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include "common/message_queue.hpp"
#include "network/base_network_model.hpp"
#include "network/network_model_factory.hpp"

namespace network {


// 异步通信管理器
// 换个名字，叫： NetworkModelManager
class NetworkModelManager {
public:
    explicit NetworkModelManager(network::NetworkModelType model_type, common::MessageQueue& message_queue);
    ~NetworkModelManager();

    // 启动和停止
    bool start(const std::string& host, uint16_t port);
    void stop();

    // 获取网络模型实例
    std::shared_ptr<BaseNetworkModel> getNetworkModel();

private:
    network::NetworkModelType model_type_;
    std::shared_ptr<BaseNetworkModel> network_model_;
    common::MessageQueue& message_queue_;
};

} // namespace network
