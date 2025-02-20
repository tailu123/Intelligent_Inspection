#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include <memory>
#include <queue>
#include <functional>
#include <thread>
#include "../protocol/x30_protocol.hpp"
#include <string_view>
#include "protocol/protocol_header.hpp"

namespace x30 {
namespace communication {

class X30Communication : public std::enable_shared_from_this<X30Communication> {
public:
    using MessageCallback = std::function<void(std::unique_ptr<protocol::IMessage>)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    X30Communication(boost::asio::io_context& io_context);

    ~X30Communication();

    // 连接管理
    void connect(const std::string& host, uint16_t port);
    void disconnect();
    bool isConnected() const;

    // 消息发送
    void sendMessage(const protocol::IMessage& message);

    // 回调设置
    void setMessageCallback(MessageCallback callback);
    void setErrorCallback(ErrorCallback callback);
private:
    // 内部实现
    void doConnect(const boost::asio::ip::tcp::endpoint& endpoint);
    void doRead();
    void doWrite();
    void handleRead(const boost::system::error_code& error, size_t bytes_transferred);
    void handleWrite(const boost::system::error_code& error);
    void processMessage(const std::string& message);

    // 成员变量
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::asio::streambuf read_buffer_;
    std::queue<std::string> write_queue_;
    std::atomic<bool> is_writing_{false};

    MessageCallback message_callback_;
    ErrorCallback error_callback_;

    void readMessageBody();
    void handleError(std::string_view error_msg);
    void processMessage(const std::vector<std::uint8_t>& message_data);

    protocol::ProtocolHeader current_header_;
    std::vector<std::uint8_t> message_buffer_;
    std::size_t received_bytes_{0};
};

// 异步通信管理器
class AsyncCommunicationManager {
public:
    AsyncCommunicationManager();
    ~AsyncCommunicationManager();

    // 启动和停止
    void start();
    void stop();

    // 获取通信实例
    std::shared_ptr<X30Communication> getCommunication();

private:
    std::unique_ptr<boost::asio::io_context> io_context_;
    // boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    std::thread io_thread_;
    std::unique_ptr<boost::asio::io_context::work> work_;
    std::shared_ptr<X30Communication> communication_;
};

} // namespace communication
} // namespace x30