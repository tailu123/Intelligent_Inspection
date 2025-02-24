#pragma once

#include "network/base_network_model.hpp"
// #include "hv/TcpClient.h"
#include <atomic>
#include <queue>
#include <mutex>
#include <memory>
#include "protocol/protocol_header.hpp"

namespace common
{
class MessageQueue;
} // namespace common

namespace network {

class LibhvNetworkModel : public BaseNetworkModel {
public:
    explicit LibhvNetworkModel(common::MessageQueue&) { }
    ~LibhvNetworkModel() override = default;

    bool connect(const std::string& host, uint16_t port) override { return true; }
    void disconnect() override {}
    bool isConnected() const override { return true; }

    void sendMessage(const protocol::IMessage& message) override {}

    void setErrorCallback(ErrorCallback callback) override {}
};
} // namespace network
