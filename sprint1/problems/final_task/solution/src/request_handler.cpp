#include "request_handler.h"
#include "json_loader.h"

#include <optional>

namespace http_handler {

    using namespace std::literals;

    // Создаёт StringResponse с заданными параметрами
    StringResponse MakeStringResponse(http::status status,
                                    std::string_view body,
                                    unsigned http_version,
                                    bool keep_alive,
                                    std::string_view content_type,
                                    size_t length = 0) {
        StringResponse response(status, http_version);
        response.set(http::field::content_type, content_type);
        if (status == http::status::method_not_allowed) {
            response.set("Allow"sv, "GET"sv);
        }
        response.body() = body;
        if (length == 0) {
            length = body.size();
        }
        response.content_length(length);
        response.keep_alive(keep_alive);
        return response;
    }

    // Обрабатываем запрос
    StringResponse RequestHandler::HandleRequest(StringRequest&& req) {
        const std::string command_base_str = "/api/";
        const std::string command1_str = "/api/v1/maps";
        const std::string command2_str = "/api/v1/maps/";
        const size_t command2_symbols = command2_str.length();

        const auto text_response = [&req](http::status status, std::string_view text, size_t length = 0) {
            return MakeStringResponse(status, text, req.version(), req.keep_alive(),
                                      ContentType::APP_JSON, length);
        };

        if (req.method() == http::verb::get) {
            std::string req_str{req.target()};
            if (req_str.starts_with(command_base_str)) {
                if (req_str.starts_with(command2_str)) {
                    // запрашивается карта с заданным id
                    std::string map_id = req_str.substr(command2_symbols);
                    std::optional<std::string> map_result = json_loader::GetMap(model::Map::Id{map_id}, game_);
                    if (map_result.has_value()) {
                        return text_response(http::status::ok, map_result.value());
                    } else {
                        // Запрашиваемый id карты не найден
                        return text_response(http::status::not_found, json_loader::GetMapNotFoundString());
                    }
                } else if (req_str.compare(command1_str) == 0) {
                    // запрашивается список карт
                    return text_response(http::status::ok, json_loader::GetListOfMaps(game_));
                } else {
                    // Неправильный запрос
                    return text_response(http::status::bad_request, json_loader::GetBadRequestString());
                }
            } else {
                // Неправильный запрос
                return text_response(http::status::bad_request, json_loader::GetBadRequestString());
            }
        } else {
            return text_response(http::status::method_not_allowed, "Invalid method"sv);
        }
    }

}  // namespace http_handler
