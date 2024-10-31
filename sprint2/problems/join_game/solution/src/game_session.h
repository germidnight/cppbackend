#pragma once
#include "tagged.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace model {

    class Map; // описан в model.h

    /* --------------------------------------- Собака --------------------------------------- */
    class Dog {
    public:
        explicit Dog(size_t id, std::string name)
                : id_(id)
                , name_(name) {}

        size_t GetDogId() const noexcept {
            return id_;
        }

        const std::string& GetDogName() const noexcept {
            return name_;
        }

    private:
        size_t id_;
        std::string name_;
    };

    /* --------------------------------------- Игровая сессия --------------------------------------- */
    class GameSession {
    public:
        using Dogs = std::vector<Dog*>;

        void AddDog(Dog* dog) {
            dogs_.emplace_back(dog);
            map_id_to_index_[dog->GetDogId()] = dogs_.size() - 1;
        }

        const Dogs& GetDogs() const noexcept {
            return dogs_;
        }

        const Dog* FindDog(const size_t dog_id) const noexcept {
            if (auto it = map_id_to_index_.find(dog_id); it != map_id_to_index_.end()) {
                return dogs_.at(it->second);
            }
            return nullptr;
        }

        const size_t CountDogsInSession() const noexcept {
            return dogs_.size();
        }

    private:
        Dogs dogs_;
        Map* map_;
        std::unordered_map<size_t, size_t> map_id_to_index_; // <dog_id, index_in_dogs_>
    };

} // namespace model