#include "model.h"

#include <random>
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

GameSession* Game::PlacePlayerOnMap(const Map::Id& map_id) {
    if (map_id_to_index_.count(map_id) == 0) {
        return nullptr;
    }
    const size_t map_index = map_id_to_index_.at(map_id);
    if (sessions_[map_index].empty() || (sessions_[map_index].back().CountDogsInSession() == MAX_DOGS_ON_MAP)) {
        sessions_[map_index].emplace_back(model::GameSession(&maps_[map_index]));
    }
    return &(sessions_[map_index].back());
}

Position Map::GetRandomPositionOnRoads() {
    Position res_pos;
    // определить дорогу
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> road_gen(0, roads_.size() - 1);
    const Road& rand_road = roads_[road_gen(gen)];
    // определить позицию на дороге
    if (rand_road.IsHorizontal()) {
        res_pos.y = rand_road.GetStart().y;
        Coord begin, end;
        if (rand_road.GetStart().x > rand_road.GetEnd().x) {
            end = rand_road.GetStart().x;
            begin = rand_road.GetEnd().x;
        } else {
            begin = rand_road.GetStart().x;
            end = rand_road.GetEnd().x;
        }
        std::uniform_int_distribution<> coord_gen(begin, end);
        res_pos.x = coord_gen(gen);
    } else {
        res_pos.x = rand_road.GetStart().x;
        Coord begin, end;
        if (rand_road.GetStart().y > rand_road.GetEnd().y) {
            end = rand_road.GetStart().y;
            begin = rand_road.GetEnd().y;
        } else {
            begin = rand_road.GetStart().y;
            end = rand_road.GetEnd().y;
        }
        std::uniform_int_distribution<> coord_gen(begin, end);
        res_pos.y = coord_gen(gen);
    }
    return res_pos;
}

}  // namespace model
