#include "audio.h"

#ifdef WIN32
#include <sdkddkver.h>
#endif
#include <boost/asio.hpp>

#include <iostream>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>

using boost::asio::ip::udp;

using namespace std::literals;

class RadioServerUDP {
public:
    RadioServerUDP(boost::asio::io_context& io, Player& player, std::string ip = "0.0.0.0", int port = 3333)
                : io_context_(io), player_(player), ip_addr_(ip), port_num_(port)
                , server_socket_(io_context_, udp::endpoint(boost::asio::ip::make_address(ip_addr_), port_num_)) {}

    void ReceiveSound() {
        std::array<char, max_buffer_size> recv_buf;
        udp::endpoint remote_endpoint;
        auto size = server_socket_.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);
        player_.PlayBuffer(recv_buf.data(), static_cast<size_t>(size / bytes_per_frame_), 1.5s);
    }
private:
    boost::asio::io_context& io_context_;
    Player& player_;
    std::string ip_addr_;
    int port_num_;
    udp::socket server_socket_;
    static const int max_buffer_size = 65000;
    static const int bytes_per_frame_ = 1;
};

class RadioClientUDP {
public:
    RadioClientUDP(boost::asio::io_context& io, Recorder& recorder)
                : io_context_(io), recorder_(recorder)
                , server_socket_(io_context_, udp::v4()) {}

    boost::system::error_code SendSound(std::string ip = "127.0.0.1", int port = 3333) {
        boost::system::error_code ec;
        auto endpoint = udp::endpoint(boost::asio::ip::make_address(ip, ec), port);
        if (ec) return ec;
        udp::endpoint sender_endpoint;

        Recorder::RecordingResult sound = recorder_.Record(max_buffer_size, 1.5s);
        server_socket_.send_to(boost::asio::buffer(sound.data, sound.frames * bytes_per_frame_), endpoint);
        return ec;
    }

private:
    boost::asio::io_context& io_context_;
    Recorder& recorder_;
    udp::socket server_socket_;
    static const int max_buffer_size = 65000;
    static const int bytes_per_frame_ = 1;
};

void UsageMessage(char** argv) {
    std::cout << "Usage: "sv << std::endl;
    std::cout << argv[0] << " server <port_num>"sv << std::endl;
    std::cout << argv[0] << " client <server IP> <port_num>"sv << std::endl;
}

std::optional<int> ExtractPortNum(std::string port) {
    int port_num;
    try {
        port_num = std::stoi(port);
        if (port_num > 65535) {
            std::cout << "Too large port_num" << std::endl;
            return std::nullopt;
        }
    } catch (const std::invalid_argument &) {
        std::cout << "Not correct port_num"sv << std::endl;
        return std::nullopt;
    } catch (const std::out_of_range &) {
        std::cout << "Too large port_num" << std::endl;
        return std::nullopt;
    }
    return port_num;
}

int main(int argc, char** argv) {
    static const char* client_str = "client";
    static const char* server_str = "server";

    static const std::string server_listen_ip = "0.0.0.0";

    if (argc < 3) {
        UsageMessage(argv);
        return 1;
    }

    boost::asio::io_context io_context;

    if (std::strcmp(argv[1], server_str) == 0) {
        // работает сервер
        Player player(ma_format_u8, 1);

        auto port_num = ExtractPortNum(argv[2]);
        if (!port_num.has_value()) {return 2;}

        RadioServerUDP radio_play(io_context, player, server_listen_ip, port_num.value());

        while (true) {
            radio_play.ReceiveSound();
            std::cout << "Playing done" << std::endl;
        }

    } else if (std::strcmp(argv[1], client_str) == 0) {
        // работает клиент
        Recorder recorder(ma_format_u8, 1);

        RadioClientUDP radio_record(io_context, recorder);

        if (argc < 4) {
            UsageMessage(argv);
            return 3;
        }

        std::string ip(argv[2]);
        auto port_num = ExtractPortNum(argv[3]);
        if (!port_num.has_value()) {return 2;}

        while(true) {
            std::string str;
            std::cout << "Press Enter to record message..." << std::endl;
            std::getline(std::cin, str);

            auto ec = radio_record.SendSound(ip, port_num.value());
            if (!ec) {
                std::cout << "Recording done" << std::endl;
            } else {
                std::cout << "Error sending data"sv << std::endl;
            }
        }
    } else {
        UsageMessage(argv);
        return 4;
    }

    return 0;
}
