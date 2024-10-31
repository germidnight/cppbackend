#pragma once
#include "tagged.h"
#include "model.h"

#include <cstdint>
#include <iomanip>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>

namespace players {

    namespace detail {
        struct TokenTag {
            TokenTag(uint64_t first, uint64_t second) : tag{first, second} {}

            // длина строки всегда 32 hex цифры (128 бит)
            std::string Serialize() {
                const size_t len_of_64bit_word_in_hex = 16;
                std::ostringstream os;
                for (int i = 0; i < 2; ++i) {
                    os << std::hex << std::setfill('0') << std::setw(len_of_64bit_word_in_hex) << tag[i];
                }
                return os.str();
            }
            uint64_t tag[2];
        };
    } // namespace detail

    using Token = util::Tagged<std::string, detail::TokenTag>;

    class Player;

    /* ----------------- Все токены игроков собраны тут ----------------- */
    class PlayerTokens {
    public:
        using TokenHasher = util::TaggedHasher<Token>;
        using TokenToPlayer = std::unordered_map<Token, const Player*, TokenHasher>;

        /* Должны ли имена пользователей (собак) быть уникальными?
         * Нужно ли имена пользователей проверять перед добавлением?
         * На всякий случай (хоть и 128 бит это очень много), но проверяем не сгенерировался ли повторяющийся токен*/
        const Token& AddPlayer(const Player* player) {
            Token token(detail::TokenTag{generator1_(), generator2_()}.Serialize());
            while (token_to_player_.count(token) > 0) {
                token = Token(detail::TokenTag{generator1_(), generator2_()}.Serialize());
            }
            token_to_player_[token] = player;
            return tokens_.emplace_back(std::move(token));
        }

        const Player* FindPlayerByToken(const Token& token) const noexcept {
            if (token_to_player_.count(token) > 0) {
                return token_to_player_.at(token);
            }
            return nullptr;
        }

    private:
        std::random_device random_device_;
        std::mt19937_64 generator1_{[this] {
            std::uniform_int_distribution<std::mt19937_64::result_type> dist;
            return dist(random_device_);
        }()};
        std::mt19937_64 generator2_{[this] {
            std::uniform_int_distribution<std::mt19937_64::result_type> dist;
            return dist(random_device_);
        }()};

        std::vector<Token> tokens_;
        TokenToPlayer token_to_player_;
    };

    /* ------------------------------------------- Игрок ------------------------------------------- */
    class Player {
    public:
        /*
         * Создание пользователя:
         * 1) создаём собаку для пользователя
         * 2) сохраняем игровую сессию пользователя
         */
        explicit Player(model::Dog dog, model::GameSession* game_session)
                        : dog_(std::make_shared<model::Dog>(dog))
                        , session_(game_session) {}

        model::Dog* GetDog() const noexcept {
            return dog_.get();
        }

        model::GameSession* GetGameSession() const noexcept {
            return session_;
        }

    private:
        std::shared_ptr<model::Dog> dog_;
        model::GameSession* session_; /* Станет невалидным при увеличении capacity вектора model::Game::sessions_ - что делать непонятно */
    };

    /* --------------------------------------- Перечень всех игроков --------------------------------------- */
    /*
     * Здесь храним:
     * 1) перечень всех пользователей в игре
     * 2) перечень токенов пользователей
     * - dog_id - будет уникальный для всех созданных игроков
     */
    class Players {
    public:
        /* Добавление пользователя:
         * 1) создаём и добавляем игрока с собакой в перечень игроков
         * 2) в выбранной игровой сессии добавляем собаку нового игрока
         */
        const Player& Add(std::string player_name, model::GameSession* game_session) {
            const Player& player = players_.emplace_back(model::Dog(++next_dog_id_, player_name), game_session);
            game_session->AddDog(player.GetDog());
            return player;
        }

        /*Player& FindByDogIdAndMapId(unsigned int dog_id, model::Map::Id map_id) {
            //
        }*/
    private:
        std::vector<Player> players_;
        size_t next_dog_id_ = 0;
    };

} // namespace players