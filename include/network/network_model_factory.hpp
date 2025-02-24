#pragma once
#include <memory>
#include <string>
#include "network/base_network_model.hpp"

namespace common
{
class MessageQueue;
} // namespace common

namespace network {

enum class NetworkModelType {
    ASIO,
    EPOLL,
    LIBHV
};

class NetworkModelFactory {
public:
    static std::unique_ptr<BaseNetworkModel> createNetworkModel(
        NetworkModelType type,
        common::MessageQueue& message_queue);
};

} // namespace network
