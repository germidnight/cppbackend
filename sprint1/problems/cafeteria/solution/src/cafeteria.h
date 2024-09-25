#pragma once
#ifdef _WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <chrono>
#include <memory>

#include "hotdog.h"
#include "result.h"

namespace net = boost::asio;
using namespace std::literals;

// Функция-обработчик операции приготовления хот-дога
using HotDogHandler = std::function<void(Result<HotDog> hot_dog)>;

/*
 * Заказ, здесь получаем из магазина, жарим хлеб и сосиску,
 * собираем хот-дог
 */
class Order : public std::enable_shared_from_this<Order> {
public:
    Order(net::io_context &io, std::shared_ptr<GasCooker> gas_cooker,
          std::shared_ptr<Sausage> sausage, std::shared_ptr<Bread> bread
          , HotDogHandler handler, int id)
          : io_{io}
          , gas_cooker_{std::move(gas_cooker)}
          , sausage_{std::move(sausage)}
          , bread_{std::move(bread)}
          , handler_{std::move(handler)}
          , hamburger_id_{id} {}

    void Execute() {
        BakeBread();
        FrySausage();
    }

private:
    void BakeBread() {
        bread_->StartBake(*gas_cooker_, [self = shared_from_this()] {
                self->bread_timer_.expires_after(HotDog::MIN_BREAD_COOK_DURATION);
                self->bread_timer_.async_wait(
                        net::bind_executor(self->strand_, [self = std::move(self)] (sys::error_code ec) {
                                self->OnBaked(ec);
                            }));
            });
    }

    void OnBaked(sys::error_code ec) {
        bread_->StopBake();
        if (ec) {
            throw std::logic_error("Bread bake error : " + ec.what());
        }
        CompleteHotDog();
    }

    void FrySausage() {
        sausage_->StartFry(*gas_cooker_, [self = shared_from_this()] {
                self->sausage_timer_.expires_after(HotDog::MIN_SAUSAGE_COOK_DURATION);
                self->sausage_timer_.async_wait(
                        net::bind_executor(self->strand_, [self = std::move(self)] (sys::error_code ec) {
                                self->OnFried(ec);
                            }));
            });
    }
    void OnFried(sys::error_code ec) {
        sausage_->StopFry();
        if(ec) {
            throw std::logic_error("Sausage fry error : " + ec.what());
        }
        CompleteHotDog();
    }

    void CompleteHotDog() {
        if (hotdog_complete_) {
            return;
        }
        if (bread_->IsCooked() && sausage_->IsCooked()) {
            hotdog_complete_ = true;
            handler_(Result{HotDog{hamburger_id_, sausage_, bread_}});
        }
    }

    net::io_context& io_;
    std::shared_ptr<GasCooker> gas_cooker_;
    std::shared_ptr<Sausage> sausage_;
    std::shared_ptr<Bread> bread_;
    HotDogHandler handler_;
    int hamburger_id_ = 0;
    net::strand<net::io_context::executor_type> strand_{net::make_strand(io_)};
    net::steady_timer bread_timer_{io_};
    net::steady_timer sausage_timer_{io_};
    bool hotdog_complete_ = false;
};

/*
 * Класс "Кафетерий". Готовит хот-доги
 */
class Cafeteria {
public:
    explicit Cafeteria(net::io_context& io)
        : io_{io} {
    }

    /* Асинхронно готовит хот-дог и вызывает handler, как только хот-дог будет готов.
     * Этот метод может быть вызван из произвольного потока */
    void OrderHotDog(HotDogHandler handler) {
        std::make_shared<Order>(io_, gas_cooker_, store_.GetSausage(), store_.GetBread()
                        , std::move(handler), ++next_id_)->Execute();
    }

private:
    net::io_context& io_;
    // Используется для создания ингредиентов хот-дога
    Store store_;
    /* Газовая плита. По условию задачи в кафетерии есть только одна газовая плита на 8 горелок
     * Используйте её для приготовления ингредиентов хот-дога.
     * Плита создаётся с помощью make_shared, так как GasCooker унаследован от
     * enable_shared_from_this. */
    std::shared_ptr<GasCooker> gas_cooker_ = std::make_shared<GasCooker>(io_);
    std::atomic_int next_id_ = 0;
};
