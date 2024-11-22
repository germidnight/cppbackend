#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "../src/tv.h"

// Тестовый стенд "Телевизор по умолчанию"
class TVByDefault : public testing::Test {
protected:
    TV tv_;
};
TEST_F(TVByDefault, IsOff) {
    EXPECT_FALSE(tv_.IsTurnedOn());
}
TEST_F(TVByDefault, DoesntShowAChannelWhenItIsOff) {
    EXPECT_FALSE(tv_.GetChannel().has_value());
}
// Включите этот тест и доработайте класс TV, чтобы тест выполнился успешно
TEST_F(TVByDefault, CantSelectAnyChannel) {
    EXPECT_THROW(tv_.SelectChannel(10), std::logic_error);
    EXPECT_EQ(tv_.GetChannel(), std::nullopt);
    tv_.TurnOn();
    EXPECT_THAT(tv_.GetChannel(), testing::Optional(1));
}

// Тестовый стенд "Включенный телевизор"
class TurnedOnTV : public TVByDefault {
protected:
    void SetUp() override {
        tv_.TurnOn();
    }
};
TEST_F(TurnedOnTV, ShowsChannel1) {
    EXPECT_TRUE(tv_.IsTurnedOn());
    EXPECT_THAT(tv_.GetChannel(), testing::Optional(1));
}
TEST_F(TurnedOnTV, AfterTurningOffTurnsOffAndDoesntShowAnyChannel) {
    tv_.TurnOff();
    EXPECT_FALSE(tv_.IsTurnedOn());
    // Сравнение с nullopt в GoogleTest выполняется так:
    EXPECT_EQ(tv_.GetChannel(), std::nullopt);
}
TEST_F(TurnedOnTV, CanSelectChannelFrom1To99) {
    tv_.SelectChannel(1);
    EXPECT_EQ(tv_.GetChannel(), 1);
    tv_.SelectChannel(2);
    EXPECT_EQ(tv_.GetChannel(), 2);
    tv_.SelectChannel(54);
    EXPECT_EQ(tv_.GetChannel(), 54);
    tv_.SelectChannel(98);
    EXPECT_TRUE(tv_.GetChannel() == 98);
    tv_.SelectChannel(99);
    EXPECT_TRUE(tv_.GetChannel() == 99);
    EXPECT_THROW(tv_.SelectChannel(0), std::out_of_range);
    EXPECT_THROW(tv_.SelectChannel(-10), std::out_of_range);
    EXPECT_THROW(tv_.SelectChannel(100), std::out_of_range);
    EXPECT_THROW(tv_.SelectChannel(99999990), std::out_of_range);
}
/* Реализуйте самостоятельно остальные тесты класса TV */
TEST_F(TurnedOnTV, can_select_lastViewedChannel) {
    tv_.SelectChannel(11);
    tv_.SelectChannel(33);
    tv_.SelectLastViewedChannel();
    EXPECT_EQ(tv_.GetChannel(), 11);
    tv_.SelectLastViewedChannel();
    EXPECT_EQ(tv_.GetChannel(), 33);
    tv_.SelectLastViewedChannel();
    EXPECT_EQ(tv_.GetChannel(), 11);
}
