#include "model.h"

#include <stdexcept>
#include <iterator>

namespace model {
using namespace std::literals;

void Map::AddOffice(Office office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        offices_.pop_back();
        throw;
    }
}

void Game::AddMap(Map map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
            sessions_.push_back(Sessions{});
        } catch (...) {
            map_id_to_index_.erase(it);
            sessions_.pop_back();
            throw;
        }
    }
}

GameSession* Game::PlacePlayerOnMap(const Map::Id &map_id) {
    if (map_id_to_index_.count(map_id) == 0) {
        return nullptr;
    }
    const size_t map_index = map_id_to_index_.at(map_id);
    if (sessions_[map_index].empty() || (sessions_[map_index].back().CountDogsInSession() == MAX_DOGS_ON_MAP)) {
        sessions_[map_index].emplace_back(model::GameSession());
    }
    return &(sessions_[map_index].back());
}

}  // namespace model
