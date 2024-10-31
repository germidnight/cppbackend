#pragma once
#include "model.h"
#include "players.h"

#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace http_handler {

    namespace beast = boost::beast;
    namespace http = beast::http;

    using namespace std::literals;

    using StringRequest = http::request<http::string_body>;

    using StringResponse = http::response<http::string_body>;

    StringResponse MakeStringResponse(http::status status,
                                      std::string_view body,
                                      unsigned http_version,
                                      bool keep_alive,
                                      std::string_view content_type,
                                      size_t length = 0,
                                      std::string allowed_methods = "GET, HEAD, POST");

    class APIHandler {
    public:
        explicit APIHandler(model::Game& game) : game_{game} {}

        APIHandler(const APIHandler&) = delete;
        APIHandler& operator=(const APIHandler&) = delete;

        StringResponse ReturnAPIResponse(const StringRequest&& req, std::string req_str,
                                         unsigned int version, bool keep_alive);

    private:
        StringResponse HandleJoining(std::string_view body, unsigned int version, bool keep_alive);
        StringResponse HandlePlayersList(std::string_view bearer_token,
                                            unsigned int version,
                                            bool keep_alive,
                                            bool head_only);

        model::Game& game_;
        players::Players players_;
        players::PlayerTokens player_tokens_;
    };

    struct ContentType {
        ContentType() = delete;
        constexpr static std::string_view BINARY = "application/octet-stream"sv;
        constexpr static std::string_view JSON  = "application/json"sv;
        constexpr static std::string_view XML   = "application/xml"sv;
        constexpr static std::string_view MP3   = "audio/mpeg"sv;
        constexpr static std::string_view BMP   = "image/bmp"sv;
        constexpr static std::string_view GIF   = "image/gif"sv;
        constexpr static std::string_view JPG   = "image/jpeg"sv;
        constexpr static std::string_view SVG   = "image/svg+xml"sv;
        constexpr static std::string_view PNG   = "image/png"sv;
        constexpr static std::string_view TIFF  = "image/tiff"sv;
        constexpr static std::string_view ICO   = "image/vnd.microsoft.icon"sv;
        constexpr static std::string_view CSS   = "text/css"sv;
        constexpr static std::string_view HTML  = "text/html"sv;
        constexpr static std::string_view JS    = "text/javascript"sv;
        constexpr static std::string_view PLAIN = "text/plain"sv;
    };

} // namespace http_handler
