#ifdef WIN32
#include <sdkddkver.h>
#endif
// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <thread>
#include <optional>

namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace std::literals;
namespace beast = boost::beast;
namespace http = beast::http;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
};

std::optional<StringRequest> ReadRequest(tcp::socket &socket, beast::flat_buffer &buffer) {
    beast::error_code ec;
    StringRequest req;

    http::read(socket, buffer, req, ec);
    if (ec == http::error::end_of_stream) {
        return std::nullopt;
    }
    if (ec) {
        throw std::runtime_error("Failed to read request: "s.append(ec.message()));
    }
    return req;
}

void DumpRequest(const StringRequest &req) {
    std::cout << req.method_string() << ' ' << req.target() << std::endl;
    // Выводим заголовки запроса
    for (const auto &header : req) {
        std::cout << "  "sv << header.name_string() << ": "sv << header.value() << std::endl;
    }
}

// Создаёт StringResponse - ответ сервера с заданными параметрами
StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
                                  bool keep_alive, std::string_view content_type = ContentType::TEXT_HTML,
                                  size_t length = 0) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    if (status == http::status::method_not_allowed) {
        response.set("Allow"sv, "GET, HEAD"sv);
    }
    response.body() = body;
    if (length == 0) {
        length = body.size();
    }
    response.content_length(length);
    response.keep_alive(keep_alive);
    return response;
}

StringResponse HandleRequest(StringRequest &&req) {
    const auto text_response = [&req](http::status status, std::string_view text, size_t length = 0) {
        return MakeStringResponse(status, text, req.version(), req.keep_alive(),
                            ContentType::TEXT_HTML, length);
    };
    // Здесь можно обработать запрос и сформировать ответ
    std::string answer = "Hello, "s
                             .append(req.target().substr(1));
    if (req.method() == http::verb::get) {
        return text_response(http::status::ok, answer);
    } else if (req.method() == http::verb::head) {
        return text_response(http::status::ok, ""sv, answer.length());
    } else {
        return text_response(http::status::method_not_allowed, "Invalid method"sv);
    }
}

template <typename RequestHandler>
void HandleConnection(tcp::socket &socket, RequestHandler &&handle_request) {
    try {
        beast::flat_buffer buffer;

        while (std::optional<StringRequest> request = ReadRequest(socket, buffer)) {
            //DumpRequest(*request);
            StringResponse response = handle_request(*std::move(request));

            http::write(socket, response);
            if (response.need_eof()) {
                break;
            }
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    beast::error_code ec;
    socket.shutdown(tcp::socket::shutdown_send, ec);
}

int main() {
    net::io_context ioc;

    const auto address = net::ip::make_address("0.0.0.0"); // слушаем порт на всех сетевых интерфейсах
    constexpr unsigned short port = 8080;                  // порт 8080 чтобы не использовать sudo
    tcp::acceptor acceptor(ioc, {address, port});
    std::cout << "Server has started..."sv << std::endl;

    while (true) {
        tcp::socket socket(ioc);
        acceptor.accept(socket);

        std::thread t(
            [](tcp::socket socket) {
                HandleConnection(socket, HandleRequest);
            },
            std::move(socket));
        t.detach();
    }
    return 0;
}
