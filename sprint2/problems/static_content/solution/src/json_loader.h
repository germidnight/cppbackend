#pragma once

#include <boost/json.hpp>

#include <filesystem>
#include <optional>
#include <string>

#include "model.h"

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path);

void LoadAndAddRoads(const boost::json::array &road_value, model::Map &map);
void LoadAndAddBuildings(const boost::json::array &building_value, model::Map &map);
void LoadAndAddOffices(const boost::json::array& office_value, model::Map &map);

// Функции для формирования ответа (request_handler)
std::string GetListOfMaps(const model::Game& game);
std::optional<std::string> GetMap(const model::Map::Id& map_id, const model::Game& game);
std::string GetMapNotFoundString();
std::string GetBadRequestString();

boost::json::array GetRoadsArray(const model::Map &map);
boost::json::array GetBuildingsArray(const model::Map &map);
boost::json::array GetOfficesArray(const model::Map &map);

}  // namespace json_loader
