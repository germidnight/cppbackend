#define BOOST_TEST_MODULE TV tests
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <sstream>

#include "../src/tv.h"
#include "boost_test_helpers.h"

struct TVFixture {
    TV tv;
};
BOOST_FIXTURE_TEST_SUITE(TV_, TVFixture)
BOOST_AUTO_TEST_CASE(is_off_by_default) {
    // Внутри теста поля структуры TVFixture доступны по их имени
    BOOST_TEST(!tv.IsTurnedOn());
}

BOOST_AUTO_TEST_CASE(doesnt_show_any_channel_by_default) {
    BOOST_TEST(!tv.GetChannel().has_value());
}

BOOST_AUTO_TEST_CASE(cant_select_any_channel_when_it_is_off) {
    BOOST_CHECK_THROW(tv.SelectChannel(10), std::logic_error);
    BOOST_TEST(tv.GetChannel() == std::nullopt);
    tv.TurnOn();
    BOOST_TEST(tv.GetChannel() == 1);
}

BOOST_AUTO_TEST_CASE(cant_select_last_channel_when_it_is_off) {
    BOOST_CHECK_THROW(tv.SelectLastViewedChannel(), std::logic_error);
}

// Тестовый стенд "Включенный телевизор" унаследован от TVFixture.
struct TurnedOnTVFixture : TVFixture {
    // В конструкторе выполняем донастройку унаследованного поля tv
    TurnedOnTVFixture() {
        tv.TurnOn();
    }
};
// (Телевизор) после включения
BOOST_FIXTURE_TEST_SUITE(After_turning_on_, TurnedOnTVFixture)
// показывает канал #1
BOOST_AUTO_TEST_CASE(shows_channel_1) {
    BOOST_TEST(tv.IsTurnedOn());
    BOOST_TEST(tv.GetChannel() == 1);
}
// Может быть выключен
BOOST_AUTO_TEST_CASE(can_be_turned_off) {
    tv.TurnOff();
    BOOST_TEST(!tv.IsTurnedOn());
    BOOST_TEST(tv.GetChannel() == std::nullopt);
}
// Может выбирать каналы с 1 по 99
BOOST_AUTO_TEST_CASE(can_select_channel_from_1_to_99) {
    tv.SelectChannel(1);
    BOOST_TEST(tv.GetChannel() == 1);
    tv.SelectChannel(2);
    BOOST_TEST(tv.GetChannel() == 2);
    tv.SelectChannel(54);
    BOOST_TEST(tv.GetChannel() == 54);
    tv.SelectChannel(98);
    BOOST_TEST(tv.GetChannel() == 98);
    tv.SelectChannel(99);
    BOOST_TEST(tv.GetChannel() == 99);
    BOOST_CHECK_THROW(tv.SelectChannel(0), std::out_of_range);
    BOOST_CHECK_THROW(tv.SelectChannel(-10), std::out_of_range);
    BOOST_CHECK_THROW(tv.SelectChannel(100), std::out_of_range);
    BOOST_CHECK_THROW(tv.SelectChannel(99999990), std::out_of_range);
}

// Может переключать на последний просмотренный канал
BOOST_AUTO_TEST_CASE(can_select_lastViewedChannel) {
    tv.SelectChannel(11);
    tv.SelectLastViewedChannel();
    BOOST_TEST(tv.GetChannel() == 1);
    tv.SelectLastViewedChannel();
    BOOST_TEST(tv.GetChannel() == 11);
    tv.SelectLastViewedChannel();
    BOOST_TEST(tv.GetChannel() == 1);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
