#include "communication/x30_communication.hpp"
#include <iostream>
#include <memory>

namespace x30 {
namespace communication {

X30Communication::X30Communication(boost::asio::io_context& io_context)
    : io_context_(io_context)
    , socket_(io_context)
    , strand_(io_context.get_executor())
    , is_writing_(false)
{
}

X30Communication::~X30Communication() {
    disconnect();
}

void X30Communication::connect(const std::string& host, uint16_t port) {
    boost::asio::ip::tcp::resolver resolver(io_context_);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    doConnect(*endpoints.begin());
}

void X30Communication::disconnect() {
    if (socket_.is_open()) {
        boost::system::error_code ec;
        socket_.close(ec);
    }
}

bool X30Communication::isConnected() const {
    return socket_.is_open();
}

void X30Communication::sendMessage(const protocol::IMessage& message) {
    auto self = shared_from_this();
    auto msg = message.serialize();

    boost::asio::post(strand_, [this, self, msg = std::move(msg)]() {
        bool write_in_progress = !write_queue_.empty();
        write_queue_.push(std::move(msg));
        if (!write_in_progress) {
            doWrite();
        }
    });
}

void X30Communication::setMessageCallback(MessageCallback callback) {
    message_callback_ = std::move(callback);
}

void X30Communication::setErrorCallback(ErrorCallback callback) {
    error_callback_ = std::move(callback);
}

void X30Communication::doConnect(const boost::asio::ip::tcp::endpoint& endpoint) {
    auto self = shared_from_this();
    socket_.async_connect(endpoint,
        boost::asio::bind_executor(strand_,
            [this, self](const boost::system::error_code& error) {
                if (!error) {
                    doRead();
                } else if (error_callback_) {
                    error_callback_("连接失败: " + error.message());
                }
            }));
}

void X30Communication::doRead() {
    auto self = shared_from_this();
    boost::asio::async_read_until(socket_, read_buffer_, "\n",
        boost::asio::bind_executor(strand_,
            [this, self](const boost::system::error_code& error, std::size_t bytes_transferred) {
                handleRead(error, bytes_transferred);
            }));
}

void X30Communication::doWrite() {
    // 已在 strand_ 中执行，无需额外保护
    if (write_queue_.empty()) {
        is_writing_ = false;
        return;
    }

    is_writing_ = true;
    boost::asio::async_write(socket_,
        boost::asio::buffer(write_queue_.front()),
        boost::asio::bind_executor(strand_,
            [this, self = shared_from_this()](
                const boost::system::error_code& error,
                std::size_t /*bytes_transferred*/) {
                    handleWrite(error);
                }));
}

void X30Communication::handleRead(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        std::string message;
        std::istream is(&read_buffer_);
        std::getline(is, message);

        processMessage(message);
        doRead();
    } else if (error != boost::asio::error::operation_aborted) {
        if (error_callback_) {
            error_callback_("读取错误: " + error.message());
        }
        disconnect();
    }
}

void X30Communication::handleWrite(const boost::system::error_code& error) {
    if (!error) {
        write_queue_.pop();
        doWrite();
    } else {
        if (error_callback_) {
            error_callback_("写入错误: " + error.message());
        }
        disconnect();
    }
}

void X30Communication::processMessage(const std::string& message) {
    if (message_callback_) {
        auto msg = protocol::MessageFactory::parseMessage(message);
        if (msg) {
            message_callback_(std::move(msg));
        } else if (error_callback_) {
            error_callback_("消息解析失败");
        }
    }
}

// AsyncCommunicationManager实现
AsyncCommunicationManager::AsyncCommunicationManager()
    : io_context_(std::make_unique<boost::asio::io_context>())
    // , strand_(io_context_->get_executor())
    , work_(std::make_unique<boost::asio::io_context::work>(*io_context_)) {
}

AsyncCommunicationManager::~AsyncCommunicationManager() {
    stop();
}

void AsyncCommunicationManager::start() {
    if (!communication_) {
        communication_ = std::make_shared<X30Communication>(*io_context_);
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

std::shared_ptr<X30Communication> AsyncCommunicationManager::getCommunication() {
    return communication_;
}

} // namespace communication
} // namespace x30