#include "api_handler.h"
#include "json_loader.h"

namespace http_handler {

    // Создаёт StringResponse с заданными параметрами
    StringResponse MakeStringResponse(http::status status,
                                      std::string_view body,
                                      unsigned http_version,
                                      bool keep_alive,
                                      std::string_view content_type,
                                      size_t length,
                                      std::string allowed_methods) {
        StringResponse response(status, http_version);
        response.set(http::field::content_type, content_type);
        response.set(http::field::cache_control, "no-cache");
        if (status == http::status::method_not_allowed) {
            response.set("Allow"sv, allowed_methods);
        }
        response.body() = body;
        if (length == 0) {
            length = response.body().size();
        }
        response.content_length(length);
        response.keep_alive(keep_alive);
        return response;
    }

    StringResponse APIHandler::ReturnAPIResponse(const StringRequest &&req, std::string req_str,
                                                 unsigned int version, bool keep_alive) {
        const std::string command_maps1_str = "/api/v1/maps";
        const std::string command_map2_str = "/api/v1/maps/";
        const size_t command_map2_symbols = command_map2_str.length();

        const std::string command_join_str = "/api/v1/game/join";
        const std::string command_session_players_str = "/api/v1/game/players";

        const auto text_response = [version, keep_alive](http::status status, std::string_view text, size_t length = 0,
                                                         std::string allowed_methods = "GET, HEAD, POST"s) {
            return MakeStringResponse(status, text, version, keep_alive, ContentType::JSON, length, allowed_methods);
        };

        if (req_str.starts_with(command_join_str)) { // -------------------------------- вход в игру
            if (ContentType::JSON.compare(req[http::field::content_type]) != 0) {
                return text_response(http::status::bad_request, json_loader::MakeErrorString("invalidArgument", "Join game request parse error"));
            }
            if (req.method() != http::verb::post) {
                return text_response(http::status::method_not_allowed,
                                    json_loader::MakeErrorString("invalidMethod", "Only POST method is expected"),
                                    0, "POST");
            }
            return HandleJoining(req.body(), version, keep_alive);

        } else if (req_str.starts_with(command_session_players_str)) { // -------------- запрашивается список игроков в сессии
            if ((req.method() == http::verb::get) || (req.method() == http::verb::head)) {
                auto auth_header = req[http::field::authorization];
                if (auth_header.empty() || (auth_header.substr(0, 7) != "Bearer ")) {
                    return text_response(http::status::unauthorized,
                                json_loader::MakeErrorString("invalidToken", "Authorization header is missing"));
                }
                return HandlePlayersList(auth_header.substr(7), version, keep_alive, (req.method() == http::verb::head));

            } else {
                return text_response(http::status::method_not_allowed,
                                     json_loader::MakeErrorString("invalidMethod", "Only GET, HEAD methods is expected"),
                                     0, "GET, HEAD");
            }
            //
        } else if (req_str.starts_with(command_map2_str)) { // ------------------------- запрашивается карта с заданным id
            std::string_view map_id = req_str.substr(command_map2_symbols);
            std::optional<std::string> map_result = json_loader::GetMap(model::Map::Id{std::string(map_id)}, game_);
            if (map_result.has_value()) {
                return text_response(http::status::ok, map_result.value());
            } else { // Запрашиваемый id карты не найден
                return text_response(http::status::not_found, json_loader::MakeErrorString("mapNotFound", "Map not found"));
            }
        } else if (req_str.compare(command_maps1_str) == 0) { // ----------------------- запрашивается список карт
            return text_response(http::status::ok, json_loader::GetListOfMaps(game_));
        } else {
            // Неправильный запрос
            return text_response(http::status::bad_request, json_loader::MakeErrorString("badRequest", "Bad request"));
        }
    }

    /*
     * Обработка запроса на подключение к игре
     */
    StringResponse APIHandler::HandleJoining(std::string_view body, unsigned int version, bool keep_alive) {
        const auto text_response = [version, keep_alive](http::status status, std::string_view text, size_t length = 0) {
            return MakeStringResponse(status, text, version, keep_alive, ContentType::JSON, length);
        };

        auto join_data = json_loader::LoadJSONJoinGame(body);

        if (join_data.error) {
            return text_response(http::status::bad_request, json_loader::MakeErrorString("invalidArgument", "Join game request parse error"));
        }

        if (join_data.user_name.empty()) {
            return text_response(http::status::bad_request, json_loader::MakeErrorString("invalidArgument", "Invalid name"));
        }

        if (join_data.map_id.empty()) {
            return text_response(http::status::bad_request, json_loader::MakeErrorString("invalidArgument", "Invalid map"));
        }
        const model::Map *map = game_.FindMap(model::Map::Id{join_data.map_id});
        if (map == nullptr) {
            return text_response(http::status::not_found, json_loader::MakeErrorString("mapNotFound", "Map not found"));
        }
        /* Прошли все проверки:
         * 1) получаем игровую сессию
         * 2) добавляем пользователя
         * 3) добавляем токен */
        auto game_session = game_.PlacePlayerOnMap(map->GetId());
        if (game_session == nullptr) {
            return text_response(http::status::not_found, json_loader::MakeErrorString("mapNotFound", "In Game::map_id_to_index_ Map not found"));
        }
        auto player = players_.Add(join_data.user_name, game_session);
        auto player_token = player_tokens_.AddPlayer(player);

        return text_response(http::status::ok, json_loader::GetPlayerAddedAnswer(**player_token, player->GetDog()->GetDogId()));
    }

    /*
     * Обработка запроса на получение списка игроков в сессии игрока (кто делает запрос)
     */
    StringResponse APIHandler::HandlePlayersList(std::string_view bearer_token,
                                                unsigned int version,
                                                bool keep_alive,
                                                bool head_only) {
        constexpr size_t bearer_token_length = 32;
        const auto text_response = [version, keep_alive](http::status status, std::string_view text, size_t length = 0) {
            return MakeStringResponse(status, text, version, keep_alive, ContentType::JSON, length);
        };

        if (bearer_token.length() != bearer_token_length) {
            return text_response(http::status::unauthorized,
                                 json_loader::MakeErrorString("invalidToken", "Authorization header is missing"));
        }

        auto found_player = player_tokens_.FindPlayerByToken(players::Token(std::move(std::string(bearer_token))));
        if (found_player == nullptr) {
            return text_response(http::status::unauthorized,
                                 json_loader::MakeErrorString("unknownToken", "Player token has not been found"));
        }

        auto session = found_player->GetGameSession();
        model::GameSession::Dogs dogs = session->GetDogs();
        if (head_only) {
            return text_response(http::status::ok, "", json_loader::GetSessionPlayers(dogs).length());
        }
        return text_response(http::status::ok, json_loader::GetSessionPlayers(dogs));
    }

} // namespace http_handler