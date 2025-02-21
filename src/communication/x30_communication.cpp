#include "communication/x30_communication.hpp"
#include <iostream>
#include <memory>
#include <string>
// #include <span>
// #include <fmt/format.h>

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
    // std::cout << "发送消息: " << msg << std::endl;

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

    // 读取固定长度的协议头
    boost::asio::async_read(socket_,
        boost::asio::buffer(&current_header_, sizeof(protocol::ProtocolHeader)),
        boost::asio::bind_executor(strand_,
            [this, self](const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
                if (error) {
                    // handleError(fmt::format("读取协议头失败: {}", error.message()));

                    handleError({"读取协议头失败: " + error.message()});
                    return;
                }

                if (!current_header_.validateSyncBytes()) {
                    handleError("协议头同步字节错误");
                    return;
                }

                // 2. 读取消息体
                // std::vector<std::uint8_t> message_buffer;
                message_buffer_.resize(current_header_.length);
                boost::asio::async_read(socket_,
                    boost::asio::buffer(message_buffer_),
                    boost::asio::bind_executor(strand_,
                        [this, self](const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
                            if (error) {
                                handleError("读取消息体失败: " + error.message());
                                return;
                            }

                            // 3. 处理完整消息
                            processMessage(message_buffer_);

                            // 4. 继续读取下一条消息
                            doRead();
                        }));
                // // 预分配消息体缓冲区
                // try {
                //     message_buffer_.resize(current_header_.length);
                // } catch (const std::exception& e) {
                //     // handleError(fmt::format("分配消息缓冲区失败: {}", e.what()));
                //     std::string msg = "分配消息缓冲区失败: ";
                //     msg += e.what();
                //     // handleError({"分配消息缓冲区失败: " + e.what()});
                //     handleError(msg);
                //     return;
                // }
                // std::cout << "X30Communication::doRead-current_header_.length:" << current_header_.length << std::endl;

                // received_bytes_ = 0;
                // readMessageBody();
            }));
}

void X30Communication::readMessageBody() {
    auto self = shared_from_this();

    const auto remaining = current_header_.length - received_bytes_;

    boost::asio::async_read(socket_,
        boost::asio::buffer(message_buffer_.data() + received_bytes_, remaining),
        boost::asio::transfer_at_least(1),
        boost::asio::bind_executor(strand_,
            [this, self](const boost::system::error_code& error, std::size_t bytes_transferred) {
                if (error) {
                    // handleError(fmt::format("读取消息体失败: {}", error.message()));
                    handleError({"读取消息体失败: " + error.message()});
                    return;
                }

                received_bytes_ += bytes_transferred;

                if (received_bytes_ < current_header_.length) {
                    readMessageBody();
                    return;
                }

                processMessage(message_buffer_);

                // 开始下一轮读取
                doRead();
            }));
}

void X30Communication::handleError(std::string_view error_msg) {
    if (error_callback_) {
        error_callback_(std::string{error_msg});
    }
    disconnect();
}

void X30Communication::processMessage(const std::vector<std::uint8_t>& message_data) {
    if (!message_callback_) {
        return;
    }

    try {
        // std::cout << "X30Communication::processMessage-message_data.size:" << message_data.size() << std::endl;
        // for (size_t i = 0; i < message_data.size(); ++i) {
        //     printf("%02X ", message_data[i]);
        //     if ((i + 1) % 16 == 0) std::cout << std::endl;
        // }

        std::cout << std::endl;

        std::string message{reinterpret_cast<const char*>(message_data.data()),
                          message_data.size()};
        // for (size_t i = 0; i < message.size(); ++i) {
        //     printf("%02X ", message[i]);
        //     if ((i + 1) % 16 == 0) std::cout << std::endl;
        // }
        // std::cout << std::endl;
        // std::cout << "收到消息: " << message << std::endl;
        if (auto msg = protocol::MessageFactory::parseMessage(message)) {
            message_callback_(std::move(msg));
        } else {
            handleError("消息解析失败");
        }
    } catch (const std::exception& e) {
        // handleError(fmt::format("消息处理异常: {}", e.what()));
        // handleError({"消息处理异常: " + e.what()});
        std::string msg = "消息处理异常: ";
        msg += e.what();
        handleError(msg);
    }
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