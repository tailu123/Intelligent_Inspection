#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include "protocol/protocol_header.hpp"
#include "protocol/x30_protocol.hpp"

using boost::asio::ip::tcp;

class TcpServer {
public:
    TcpServer(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        std::cout << "TCP服务器启动，监听端口: " << port << std::endl;
        startAccept();
    }

private:
    void startAccept() {
        auto socket = std::make_shared<tcp::socket>(acceptor_.get_executor());
        acceptor_.async_accept(*socket,
            [this, socket](boost::system::error_code ec) {
                if (!ec) {
                    std::cout << "新的客户端连接: "
                              << socket->remote_endpoint().address().to_string()
                              << ":" << socket->remote_endpoint().port() << std::endl;
                    startRead(socket);
                }
                startAccept();
            });
    }

    void startRead(std::shared_ptr<tcp::socket> socket) {
        auto header = std::make_shared<protocol::ProtocolHeader>();
        boost::asio::async_read(*socket,
            boost::asio::buffer(header.get(), sizeof(protocol::ProtocolHeader)),
            [this, socket, header](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cout << "读取协议头错误: " << ec.message() << std::endl;
                    return;
                }

                auto buffer = std::make_shared<std::vector<char>>(header->length);
                boost::asio::async_read(*socket,
                    boost::asio::buffer(buffer->data(), header->length),
                    [this, socket, header, buffer](boost::system::error_code ec, std::size_t length) {
                        if (ec) {
                            std::cout << "读取消息体错误: " << ec.message() << std::endl;
                            return;
                        }

                        std::cout << "收到消息，长度: " << length << std::endl;

                        // 发送响应
                        // sendResponse(socket, header->message_id);

                        // 继续读取下一条消息
                        startRead(socket);
                    });
            });
    }

    void sendResponse(std::shared_ptr<tcp::socket> socket, uint16_t message_id) {
        std::unique_ptr<protocol::IMessage> response;

        // 根据消息ID创建对应的响应
        switch (message_id) {
            case 1003: {  // NAVIGATION_TASK_REQ
                auto resp = std::make_unique<protocol::NavigationTaskResponse>();
                resp->errorCode = protocol::ErrorCode::SUCCESS;
                resp->value = 1;
                resp->errorStatus = 0;
                resp->timestamp = "2024-03-21 10:00:00";
                response = std::move(resp);
                break;
            }
            case 1004: {  // CANCEL_TASK_REQ
                auto resp = std::make_unique<protocol::CancelTaskResponse>();
                resp->errorCode = protocol::ErrorCode::SUCCESS;
                resp->timestamp = "2024-03-21 10:00:00";
                response = std::move(resp);
                break;
            }
            case 1007: {  // QUERY_STATUS_REQ
                auto resp = std::make_unique<protocol::QueryStatusResponse>();
                resp->errorCode = protocol::ErrorCode::SUCCESS;
                resp->value = 1;
                resp->status = protocol::NavigationStatus::EXECUTING;
                resp->timestamp = "2024-03-21 10:00:00";
                response = std::move(resp);
                break;
            }
            default: {
                std::cout << "未知的消息ID: " << message_id << std::endl;
                return;
            }
        }

        if (response) {
            // 序列化响应（包含协议头）
            auto response_data = response->serialize();
            std::cout << "发送响应，长度: " << response_data.length() << std::endl;

            // 发送响应
            boost::asio::async_write(*socket,
                boost::asio::buffer(response_data),
                [](boost::system::error_code ec, std::size_t /*length*/) {
                    if (ec) {
                        std::cout << "发送响应错误: " << ec.message() << std::endl;
                    }
                });
        }
    }

    tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "用法: tcp_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;
        TcpServer server(io_context, std::atoi(argv[1]));
        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "异常: " << e.what() << "\n";
    }

    return 0;
}
