#include "network/x30_communication.hpp"
#include <memory>
#include "network/asio_network_model.hpp"
#include "network/base_network_model.hpp"
// #include "network/epoll_network_model.hpp"
// #include "network/libhv_network_model.hpp"
// #include "common/event_bus.hpp"
#include <iostream>
namespace network {


// NetworkModelManager实现
// NetworkModelManager::NetworkModelManager(common::MessageQueue& message_queue)
// : io_context_(std::make_unique<boost::asio::io_context>())
// , message_queue_(message_queue)
//     , work_(std::make_unique<boost::asio::io_context::work>(*io_context_)) {
// }
NetworkModelManager::NetworkModelManager(network::NetworkModelType model_type, common::MessageQueue& message_queue)
    : model_type_(model_type)
    , message_queue_(message_queue)
{
}

NetworkModelManager::~NetworkModelManager() {
    stop();
}

bool NetworkModelManager::start(const std::string& host, uint16_t port) {
    try {
        network_model_ = network::NetworkModelFactory::createNetworkModel(
            model_type_,
            message_queue_
        );
        std::cout << "NetworkModelManager::start" << std::endl;
        
        return network_model_->connect(host, port);
    } catch (const std::exception& e) {
        std::cout << "[NetworkModelManager]: 启动失败, 错误: " << e.what() << std::endl;
        return false;
    }
}

void NetworkModelManager::stop() {
    network_model_->disconnect();
}

std::shared_ptr<BaseNetworkModel> NetworkModelManager::getNetworkModel() {
    return network_model_;
}

}  // namespace network
