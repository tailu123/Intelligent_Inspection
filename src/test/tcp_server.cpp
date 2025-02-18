#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>

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
        auto buffer = std::make_shared<std::array<char, 4096>>();
        socket->async_read_some(boost::asio::buffer(*buffer),
            [this, socket, buffer](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::cout << "\n收到数据 (" << length << " 字节):" << std::endl;
                    std::cout << std::string_view(buffer->data(), length) << std::endl;
                    startRead(socket);
                } else {
                    std::cout << "客户端断开连接: "
                              << socket->remote_endpoint().address().to_string()
                              << ":" << socket->remote_endpoint().port() << std::endl;
                }
            });
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