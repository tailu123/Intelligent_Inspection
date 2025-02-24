#pragma once

#include "network/base_network_model.hpp"

namespace common
{
class MessageQueue;
} // namespace common

namespace network {

class LibhvNetworkModel : public BaseNetworkModel {
public:
LibhvNetworkModel(common::MessageQueue &message_queue);
~LibhvNetworkModel();

void connect(const std::string& host, uint16_t port) override;
void disconnect() override;
bool isConnected() const override;

void sendMessage(const protocol::IMessage& message) override;

void setErrorCallback(ErrorCallback callback) override;
private:

};
} // namespace network
