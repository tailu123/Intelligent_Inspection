#pragma once
#include <string>
#include <functional>
#include "protocol/x30_protocol.hpp"

namespace network {
using ErrorCallback = std::function<void(const std::string&)>;

class BaseNetworkModel {
public:
virtual bool connect(const std::string& host, uint16_t port) = 0;
virtual void disconnect() = 0;
virtual bool isConnected() const = 0;

virtual void sendMessage(const protocol::IMessage& message) = 0;

virtual void setErrorCallback(ErrorCallback callback) = 0;
};
}  // namespace network
