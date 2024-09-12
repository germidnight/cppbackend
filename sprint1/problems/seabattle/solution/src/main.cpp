#ifdef WIN32
#include <sdkddkver.h>
#endif

#include "seabattle.h"

#include <atomic>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <string_view>

namespace net = boost::asio;
using net::ip::tcp;
using namespace std::literals;

void PrintFieldPair(const SeabattleField& left, const SeabattleField& right) {
    auto left_pad = "  "s;
    auto delimeter = "    "s;
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
    for (size_t i = 0; i < SeabattleField::field_size; ++i) {
        std::cout << left_pad;
        left.PrintLine(std::cout, i);
        std::cout << delimeter;
        right.PrintLine(std::cout, i);
        std::cout << std::endl;
    }
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
}

template <size_t sz>
static std::optional<std::string> ReadExact(tcp::socket& socket) {
    boost::array<char, sz> buf;
    boost::system::error_code ec;

    net::read(socket, net::buffer(buf), net::transfer_exactly(sz), ec);

    if (ec) {
        return std::nullopt;
    }

    return {{buf.data(), sz}};
}

static bool WriteExact(tcp::socket& socket, std::string_view data) {
    boost::system::error_code ec;

    net::write(socket, net::buffer(data), net::transfer_exactly(data.size()), ec);

    return !ec;
}

class SeabattleAgent {
public:
    SeabattleAgent(const SeabattleField& field)
        : my_field_(field) {
    }

    void StartGame(tcp::socket& socket, bool my_initiative) {
        std::string move_str;
        while(!IsGameEnded()) {
            PrintFields();
            if (my_initiative) {
                PrintYourTurn();
                std::cin >> move_str;
                std::cout << std::endl;
                auto turn_pos = ParseMove(move_str);
                if (!turn_pos.has_value()) {continue;}

                SendMove(socket, move_str);
                auto res = ReadResult(socket);
                switch (res) {
                    case SeabattleField::ShotResult::MISS: {
                        other_field_.MarkMiss(turn_pos.value().second, turn_pos.value().first);
                        my_initiative = false;
                        PrintMiss();
                        break;
                    }
                    case SeabattleField::ShotResult::HIT: {
                        other_field_.MarkHit(turn_pos.value().second, turn_pos.value().first);
                        PrintHit();
                        break;
                    }
                    case SeabattleField::ShotResult::KILL: {
                        other_field_.MarkKill(turn_pos.value().second, turn_pos.value().first);
                        PrintKill();
                        break;
                    }
                }
            } else {
                PrintWaitTurn();
                auto pos = ReadMove(socket);
                if (!pos.has_value()) {
                    SendResult(socket, SeabattleField::ShotResult::MISS);
                    PrintShotTo();
                    continue;
                }
                move_str = MoveToString(pos.value());
                PrintShotTo();
                std::cout << move_str << std::endl;

                auto shot_res = my_field_.Shoot(pos.value().second, pos.value().first);
                SendResult(socket, shot_res);
                switch (shot_res) {
                    case SeabattleField::ShotResult::MISS: {
                        my_initiative = true;
                        break;
                    }
                    case SeabattleField::ShotResult::HIT: {
                        my_field_.MarkHit(pos.value().second, pos.value().first);
                        break;
                    }
                    case SeabattleField::ShotResult::KILL: {
                        my_field_.MarkKill(pos.value().second, pos.value().first);
                        break;
                    }
                }
            }
        }
    }

private:
    static std::optional<std::pair<int, int>> ParseMove(const std::string_view& sv) {
        if (sv.size() != 2) return std::nullopt;

        int p1 = sv[0] - 'A', p2 = sv[1] - '1';

        if (p1 < 0 || p1 > 8) return std::nullopt;
        if (p2 < 0 || p2 > 8) return std::nullopt;

        return {{p1, p2}};
    }

    static std::string MoveToString(std::pair<int, int> move) {
        char buff[] = {static_cast<char>(move.first) + 'A', static_cast<char>(move.second) + '1'};
        return {buff, 2};
    }

    void PrintFields() const {
        PrintFieldPair(my_field_, other_field_);
    }

    bool IsGameEnded() const {
        return my_field_.IsLoser() || other_field_.IsLoser();
    }

    static std::optional<std::pair<int, int>> ReadMove(tcp::socket &socket) {
        static const int buf_size = 2;
        char buf[buf_size];
        size_t size = socket.read_some(net::buffer(buf, buf_size));
        if (size != buf_size) {
            std::cerr << "Error reading data for ReadMove"sv << std::endl;
            return std::nullopt;
        }
        return ParseMove(std::basic_string_view(buf, size));
    }

    SeabattleField::ShotResult ReadResult(tcp::socket& socket) {
        static const int buf_size = 1;
        char buf[buf_size];
        size_t size = socket.read_some(net::buffer(buf, buf_size));
        if (size != buf_size) {
            std::cerr << "Error reading data for ReadResult"sv << std::endl;
        }
        if (buf[0] == 'K') {
            return SeabattleField::ShotResult::KILL;
        }
        if (buf[0] == 'H') {
            return SeabattleField::ShotResult::HIT;
        }
        return SeabattleField::ShotResult::MISS;
    }

    boost::system::error_code SendMove(tcp::socket &socket, std::string_view move) {
        static const int buf_size = 2;
        boost::system::error_code ec;
        socket.write_some(net::buffer(move.data(), buf_size), ec);
        return ec;
    }

    boost::system::error_code SendResult(tcp::socket& socket, SeabattleField::ShotResult shot_result) {
        static const int buf_size = 1;
        char buf[buf_size] = {'M'}; // SeabattleField::ShotResult::MISS
        switch (shot_result) {
        case SeabattleField::ShotResult::HIT: {
                buf[0] = 'H';
                break;
            }
        case SeabattleField::ShotResult::KILL: {
                buf[0] = 'K';
                break;
            }
        }
        boost::system::error_code ec;
        socket.write_some(net::buffer(buf, 1), ec);
        return ec;
    }

    void PrintYourTurn() const {
        std::cout << "Your turn: "sv;
    }

    void PrintWaitTurn() const {
        std::cout << "Waiting for turn..."sv << std::endl;
    }

    void PrintShotTo() const {
        std::cout << "Shot to "sv;
    }

    void PrintMiss() const {
        std::cout << "Miss!"sv << std::endl;
    }

    void PrintHit() const {
        std::cout << "Hit!"sv << std::endl;
    }

    void PrintKill() const {
        std::cout << "Kill!"sv << std::endl;
    }

private:
    SeabattleField my_field_;
    SeabattleField other_field_;
};

void StartServer(const SeabattleField& field, unsigned short port) {
    SeabattleAgent agent(field);

    net::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(net::ip::make_address("0.0.0.0"), port));
    boost::system::error_code ec;
    tcp::socket socket{io_context};
    std::cout << "Waiting for connection..."sv << std::endl;
    acceptor.accept(socket, ec);
    if (ec) {
        std::cerr << "Can't accept connection"sv << std::endl;
        throw std::ios_base::failure("Can't accept connection");
    }

    agent.StartGame(socket, false);
};

void StartClient(const SeabattleField& field, const std::string& ip_str, unsigned short port) {
    SeabattleAgent agent(field);

    boost::system::error_code ec;
    auto endpoint = tcp::endpoint(net::ip::make_address(ip_str), port);
    if (ec) {
        std::cerr << "Wrong entered IP address: "sv << ip_str << std::endl;
        throw std::invalid_argument("Wrong entered IP address");
    }
    net::io_context io_context;
    tcp::socket socket{io_context};
    socket.connect(endpoint, ec);
    if (ec) {
        std::cerr << "Can't connect to server"sv << std::endl;
        throw std::ios_base::failure("Can't connect to server");
    }

    agent.StartGame(socket, true);
};

int main(int argc, const char** argv) {
    if (argc != 3 && argc != 4) {
        std::cout << "Usage: program <seed> [<ip>] <port>" << std::endl;
        return 1;
    }

    std::mt19937 engine(std::stoi(argv[1]));
    SeabattleField fieldL = SeabattleField::GetRandomField(engine);

    if (argc == 3) {
        StartServer(fieldL, std::stoi(argv[2]));
    } else if (argc == 4) {
        StartClient(fieldL, argv[2], std::stoi(argv[3]));
    }
}
