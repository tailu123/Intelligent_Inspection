#include "network/x30_communication.hpp"
#include <memory>
#include "network/asio_network_model.hpp"
#include "network/base_network_model.hpp"
#include "network/epoll_network_model.hpp"
#include "network/libhv_network_model.hpp"
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

void NetworkModelManager::start() {
    if (!network_model_) {
        // network_model_ = std::make_shared<AsioNetworkModel>(*io_context_, message_queue_);
        network_model_ = std::make_shared<EpollNetworkModel>(message_queue_);
        // network_model_ = std::make_shared<LibhvNetworkModel>(message_queue_);
        // TODO: 增加多模型支持，如：EpollNetworkModel, libhvNetworkModel, factory pattern
    }

    // try {
    //     network_model_ = network::NetworkModelFactory::createNetworkModel(
    //         model_type_,
    //         message_queue_
    //     );

    //     network_model_->setErrorCallback([this](const std::string& error) {
    //         // 处理错误...
    //     });

    //     return network_model_->connect(host, port);
    // } catch (const std::exception& e) {
    //     // 处理异常...
    //     return false;
    // }

}

void NetworkModelManager::stop() {
    network_model_->disconnect();
}

std::shared_ptr<BaseNetworkModel> NetworkModelManager::getNetworkModel() {
    return network_model_;
}

}  // namespace network
