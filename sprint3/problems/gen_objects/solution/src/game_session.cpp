#include "game_session.h"
#include "loot_generator.h"
#include "model.h"

#include <random>

namespace model {

    /* 1) Получаем от генератора количество новых потерянных предметов случайным образом.
     * 2) Генерируем для каждого из них:
     *      - Тип предмета — целое число от 0 до K−1 включительно, где K — количество элементов в массиве lootTypes
     *      - Объект генерируется в случайно выбранной точке на случайно выбранной дороге карты. */
    void GameSession::AddLostObjectsOnSession(loot_gen::LootGenerator& loot_generator,
                                  		loot_gen::LootGenerator::TimeInterval time_delta) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> random_gen(0, map_->GetLootTypesCount() - 1);

        size_t lost_obj_count = loot_generator.Generate(time_delta, lost_objects_.size(), dogs_.size());
        for (size_t i = 0; i < lost_obj_count; ++i) {
            lost_objects_.emplace_back(std::make_shared<LostObject>(random_gen(gen), map_->GetRandomPositionOnRoads()));
        }
    }

} // namespace model
