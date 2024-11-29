#include "players.h"

#include <iomanip>
#include <sstream>

namespace players {

namespace detail {
    // длина строки всегда 32 hex цифры (128 бит)
    std::string TokenTag::Serialize() const {
        const size_t len_of_64bit_word_in_hex = 16;
        std::ostringstream os;
        for (int i = 0; i < 2; ++i) {
            os << std::hex << std::setfill('0') << std::setw(len_of_64bit_word_in_hex) << tag[i];
        }
        return os.str();
    }

} // namespace detail

    std::shared_ptr<Token> PlayerTokens::AddPlayer(std::shared_ptr<Player> player) {
        Token token(detail::TokenTag{generator1_(), generator2_()}.Serialize());
        while (token_to_player_.count(token) > 0) {
            token = Token(detail::TokenTag{generator1_(), generator2_()}.Serialize());
        }
        token_to_player_[token] = std::move(player);
        return tokens_.emplace_back(std::make_shared<Token>(token));
    }

    std::shared_ptr<Player> PlayerTokens::FindPlayerByToken(const Token& token) const noexcept {
        if (token_to_player_.count(token) > 0) {
            return token_to_player_.at(token);
        }
        return nullptr;
    }

    /* Создание и добавление пользователя:
     * 1) создаём и добавляем игрока с собакой в перечень игроков
     * 2) в выбранной игровой сессии добавляем собаку нового игрока
     */
    std::shared_ptr<Player> Players::Add(std::string player_name,
                                         std::shared_ptr<model::GameSession> game_session,
                                         bool randomize_spawn_point) {
        model::Map* game_map = game_session->GetMap();

        std::shared_ptr<Player> player;
        if (randomize_spawn_point) {
	        player = players_.emplace_back(std::make_shared<Player>(model::Dog(++next_dog_id_, player_name,
                                                        game_session->GetMap()->GetRandomPositionOnRoads()),
                                                game_session));
        } else {
            player = players_.emplace_back(std::make_shared<Player>(model::Dog(++next_dog_id_, player_name,
                                                        game_session->GetMap()->GetTestPositionOnRoads()),
                                                game_session));
        }
        game_session->AddDog(player->GetDog());
        map_id_to_index_.emplace(player->GetDog()->GetDogId(), players_.size() - 1);
        return player;
    }

    std::shared_ptr<Player> Players::FindPlayerByDogId(size_t dog_id) const {
        if (map_id_to_index_.count(dog_id) > 0) {
            return players_[map_id_to_index_.at(dog_id)];
        }
        return nullptr;
    }

    /* Добавляем нового пользователя в игру
     * 1) находим карту
     * 2) получаем игровую сессию
     * 3) добавляем пользователя
     * 4) получаем токен пользователя */
    JoinGameResult Application::JoinPlayerToGame(model::Map::Id map_id, std::string_view player_name) {
        const model::Map* map = game_.FindMap(map_id);
        if (map == nullptr) {
            return JoinGameResult(nullptr, 0, JoinGameErrorCode::MAP_NOT_FOUND);
        }

        auto game_session = game_.PlacePlayerOnMap(map->GetId());
        if (game_session == nullptr) {
            return JoinGameResult(nullptr, 0, JoinGameErrorCode::SESSION_NOT_FOUND);
        }

        auto player = players_.Add(std::string(player_name), game_session, IsRandomSpawnPoint());
        auto token = player_tokens_.AddPlayer(player);
        return JoinGameResult(token.get(), player->GetDog()->GetDogId(), JoinGameErrorCode::NONE);
    }

    std::vector<std::shared_ptr<Player>> Application::GetPlayersInSession(const std::shared_ptr<Player> player) const {
        std::vector<std::shared_ptr<Player>> res_players;

        auto session = player->GetGameSession();
        for (const auto next_dog : session->GetDogs()) {
            auto next_player = players_.FindPlayerByDogId(next_dog->GetDogId());
            if (next_player != nullptr) {
                res_players.push_back(next_player);
            }
        }
        return res_players;
    }

    void Application::SetDogAction(std::shared_ptr<Player> player, ActionMove action_move) {
        auto dog_speed = player->GetGameSession()->GetMap()->GetSpeed();
        auto dog = player->GetDog();
        switch (action_move) {
        case ActionMove::LEFT : {
            dog->SetVelocity({-dog_speed, 0.});
            dog->SetDirection(model::Direction::WEST);
            break;
        }
        case ActionMove::RIGHT : {
            dog->SetVelocity({dog_speed, 0.});
            dog->SetDirection(model::Direction::EAST);
            break;
        }
        case ActionMove::UP : {
            dog->SetVelocity({0., -dog_speed});
            dog->SetDirection(model::Direction::NORTH);
            break;
        }
        case ActionMove::DOWN : {
            dog->SetVelocity({0., dog_speed});
            dog->SetDirection(model::Direction::SOUTH);
            break;
        }
        default: {
            dog->SetVelocity({0., 0.});
            break;
        }
        }
    }

    /* Пересчёт событий на карте:
     * 1) двигаем собак по картам
     * 2) размещаем потерянные объекты в сессиях
     * 3) отдаём находки в офис
     * 4) подбираем предметы */
    void Application::MoveDogs(double time_period) {
        using namespace std::chrono_literals;
        loot_gen::LootGenerator::TimeInterval duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                std::chrono::duration<double, std::milli>{time_period * 1s});

        std::unordered_map<size_t, collision_detector::Gatherer> all_gatherers;
        // перебор всех игроков
        for (auto player : players_.GetPlayers()) {
            model::DogState state = player->GetGameSession()->GetMap()->MoveDog(player->GetDog(), time_period);

            all_gatherers[player->GetDog()->GetDogId()] = collision_detector::Gatherer{
                                        player->GetDog()->GetDogState().position,
                                        state.position,
                                        model::LostObject::GATHERER_HALF_WIDTH};

            player->GetDog()->SetState(state);
        }
        // перебор всех сессий
        for (auto& map : game_.GetSessions()) {
            for (auto session : map) {
                // размещаем потерянные объекты в сессии
                session->AddLostObjectsOnSession(game_.GetLootGenerator(), duration);

                std::vector<collision_detector::Gatherer> gatherers;
                std::unordered_map<size_t, std::shared_ptr<model::Dog>> idx_to_dog;
                for (const auto& dog : session->GetDogs()) {
                    gatherers.emplace_back(all_gatherers.at(dog->GetDogId()));
                    idx_to_dog[gatherers.size() - 1] = dog;
                }
                BringItemsToOffices(session, gatherers, idx_to_dog);
                PickUpItems(session, gatherers, idx_to_dog);
            }
        }
    }

    /* подбираем предметы:
     * 1) формируем вектор items (gatherers сформирован выше) и передаём их провайдеру
     * 2) получаем вектор событий подбора вещей собаками
     * 3) для каждого события:
     *      - подбираем собаками ещё не подобранные вещи, не забывая пометить подобранные вещи
     * 4) удаляем подобранные вещи из списка потерянных */
    void Application::PickUpItems(std::shared_ptr<model::GameSession> session,
                                  const std::vector<collision_detector::Gatherer>& gatherers,
                                  const std::unordered_map<size_t, std::shared_ptr<model::Dog>>& idx_to_dog) {
        std::vector<std::shared_ptr<model::LostObject>> items(session->GetLostObjects().begin(),
                                                              session->GetLostObjects().end());
        ItemGatherer ig(items.size(), items, gatherers.size(), gatherers);
        std::vector<bool> item_picked(session->CountLostObjects(), false);

        for (const auto &event : collision_detector::FindGatherEvents(ig)) {
            if (!item_picked[event.item_id]) {
                item_picked[event.item_id] = idx_to_dog.at(event.gatherer_id)->AddPickedObject(
                                                        model::PickedObject(items[event.item_id]->GetId(),
                                                                            items[event.item_id]->GetType()),
                                                        session->GetMap()->GetBagCapacity());
            }
        }
        session->RemoveObjectsFromLost(item_picked); // удаляем только подобранные вещи
    }

    /* отдаём находки в офис
     * 1) формируем вектор offices (gatherers сформирован выше) и передаём их провайдеру
     * 2) получаем вектор событий посещения собаками офисов
     * 3) для каждого события:
     *      - сбрасываем все подобранные вещи */
    void Application::BringItemsToOffices(std::shared_ptr<model::GameSession> session,
                         const std::vector<collision_detector::Gatherer> &gatherers,
                         const std::unordered_map<size_t, std::shared_ptr<model::Dog>> &idx_to_dog) {
        std::vector<std::shared_ptr<model::LostObject>> offices;
        for (const auto& office : session->GetMap()->GetOffices()) {
            offices.emplace_back(std::make_shared<model::LostObject>(model::LostObject(0,
                                                        {static_cast<double>(office.GetPosition().x),
                                                        static_cast<double>(office.GetPosition().y)},
                                                   offices.size(),
                                                   model::LostObject::OFFICE_HALF_WIDTH)));
        }
        ItemGatherer og(offices.size(), offices, gatherers.size(), gatherers);
        for (const auto& event : collision_detector::FindGatherEvents(og)) {
            auto dog = idx_to_dog.at(event.gatherer_id);
            if (dog->IsBagEmpty()) {
                continue;
            }
            for(const auto& obj : dog->FlushPickedObjects()) {
                dog->AddScores(session->GetMap()->GetLootByIndex(obj.GetType()).GetScores());
            }
        }
    }

} // namespace players
