#include "network/x30_communication.hpp"
#include <memory>
#include "network/asio_network_model.hpp"
#include "network/base_network_model.hpp"
#include "network/epoll_network_model.hpp"

namespace network {


// AsyncCommunicationManager实现
AsyncCommunicationManager::AsyncCommunicationManager(common::MessageQueue& message_queue)
: io_context_(std::make_unique<boost::asio::io_context>())
, message_queue_(message_queue)
    , work_(std::make_unique<boost::asio::io_context::work>(*io_context_)) {
}

AsyncCommunicationManager::~AsyncCommunicationManager() {
    stop();
}

void AsyncCommunicationManager::start() {
    if (!communication_) {
        communication_ = std::make_shared<AsioNetworkModel>(*io_context_, message_queue_);
        // communication_ = std::make_shared<EpollNetworkModel>(message_queue_);
        // TODO: 增加多模型支持，如：EpollNetworkModel, libhvNetworkModel, factory pattern
    }

    if (!io_thread_.joinable()) {
        io_thread_ = std::thread([this]() {
            io_context_->run();
        });
    }
}

void AsyncCommunicationManager::stop() {
    work_.reset();
    if (io_thread_.joinable()) {
        io_thread_.join();
    }
    io_context_->stop();
}

std::shared_ptr<BaseNetworkModel> AsyncCommunicationManager::getCommunication() {
    return communication_;
}

}  // namespace network
