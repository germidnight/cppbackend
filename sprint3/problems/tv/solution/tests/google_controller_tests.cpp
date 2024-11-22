#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "../src/controller.h"

using namespace std::literals;

class ControllerWithTurnedOffTV : public testing::Test {
protected:
    void SetUp() override {
        ASSERT_FALSE(tv_.IsTurnedOn());
    }

    void RunMenuCommand(std::string command) {
        input_.str(std::move(command));
        input_.clear();
        menu_.Run();
    }

    void ExpectExtraArgumentsErrorInOutput(std::string_view command) const {
        ExpectOutput(
            "Error: the "s.append(command).append(" command does not require any arguments\n"sv));
    }

    void ExpectEmptyOutput() const {
        ExpectOutput({});
    }

    void ExpectOutput(std::string_view expected) const {
        // В g++ 10.3 не реализован метод ostringstream::view(), поэтому приходится
        // использовать метод str()
        EXPECT_EQ(output_.str(), std::string{expected});
    }

    TV tv_;
    std::istringstream input_;
    std::ostringstream output_;
    Menu menu_{input_, output_};
    Controller controller_{tv_, menu_};
};

TEST_F(ControllerWithTurnedOffTV, OnInfoCommandPrintsThatTVIsOff) {
    input_.str("Info"s);
    menu_.Run();
    ExpectOutput("TV is turned off\n"sv);
    EXPECT_FALSE(tv_.IsTurnedOn());
}
TEST_F(ControllerWithTurnedOffTV, OnInfoCommandPrintsErrorMessageIfCommandHasAnyArgs) {
    RunMenuCommand("Info some extra args"s);
    EXPECT_FALSE(tv_.IsTurnedOn());
    ExpectExtraArgumentsErrorInOutput("Info"sv);
}
TEST_F(ControllerWithTurnedOffTV, OnInfoCommandWithTrailingSpacesPrintsThatTVIsOff) {
    input_.str("Info  "s);
    menu_.Run();
    ExpectOutput("TV is turned off\n"sv);
}
TEST_F(ControllerWithTurnedOffTV, OnTurnOnCommandTurnsTVOn) {
    RunMenuCommand("TurnOn"s);
    EXPECT_TRUE(tv_.IsTurnedOn());
    ExpectEmptyOutput();
}
TEST_F(ControllerWithTurnedOffTV, OnTurnOnCommandPrintsErrorMessageIfCommandHasAnyArgs) {
    RunMenuCommand("TurnOn some extra args"s);
    EXPECT_FALSE(tv_.IsTurnedOn());
    ExpectExtraArgumentsErrorInOutput("TurnOn"sv);
}
/*
 * Протестируйте остальные аспекты поведения класса Controller, когда TV выключен
 */
TEST_F(ControllerWithTurnedOffTV, onSelectChannel_if_TV_is_off) {
    RunMenuCommand("SelectChannel 3"s);
    ExpectOutput("TV is turned off\n"sv);
}

TEST_F(ControllerWithTurnedOffTV, onSelectPreviousChannel_if_TV_is_off) {
    RunMenuCommand("SelectPreviousChannel"s);
    ExpectOutput("TV is turned off\n"sv);
}

//-----------------------------------------------------------------------------------

class ControllerWithTurnedOnTV : public ControllerWithTurnedOffTV {
protected:
    void SetUp() override {
        tv_.TurnOn();
    }
};

TEST_F(ControllerWithTurnedOnTV, OnTurnOffCommandTurnsTVOff) {
    RunMenuCommand("TurnOff"s);
    EXPECT_FALSE(tv_.IsTurnedOn());
    ExpectEmptyOutput();
}
TEST_F(ControllerWithTurnedOnTV, OnTurnOffCommandPrintsErrorMessageIfCommandHasAnyArgs) {
    RunMenuCommand("TurnOff some extra args"s);
    EXPECT_TRUE(tv_.IsTurnedOn());
    ExpectExtraArgumentsErrorInOutput("TurnOff"sv);
}
// Включите этот тест, после того, как реализуете метод TV::SelectChannel
TEST_F(ControllerWithTurnedOnTV, OnInfoPrintsCurrentChannel) {
    tv_.SelectChannel(42);
    RunMenuCommand("Info"s);
    ExpectOutput("TV is turned on\nChannel number is 42\n"sv);
}
/*
 * Протестируйте остальные аспекты поведения класса Controller, когда TV включен
 */
TEST_F(ControllerWithTurnedOnTV, onSelectChannel_good_channels) {
    RunMenuCommand("SelectChannel 30"s);
    ExpectEmptyOutput();
    EXPECT_EQ(tv_.GetChannel(), 30);

    RunMenuCommand("SelectChannel 1"s);
    ExpectEmptyOutput();
    EXPECT_EQ(tv_.GetChannel(), 1);

    RunMenuCommand("SelectChannel 99"s);
    ExpectEmptyOutput();
    EXPECT_EQ(tv_.GetChannel(), 99);
}

TEST_F(ControllerWithTurnedOnTV, on_SelectChannel_bad_channel_A) {
    tv_.SelectChannel(30);
    RunMenuCommand("SelectChannel A"s);
    ExpectOutput("Invalid channel\n"sv);
    EXPECT_EQ(tv_.GetChannel(), 30);
}

TEST_F(ControllerWithTurnedOnTV, on_SelectChannel_bad_channel_float3_14) {
    tv_.SelectChannel(30);
    RunMenuCommand("SelectChannel 3.14"s);
    ExpectOutput("Invalid channel\n"sv);
    EXPECT_EQ(tv_.GetChannel(), 30);
}

TEST_F(ControllerWithTurnedOnTV, on_SelectChannel_bad_channel_0) {
    tv_.SelectChannel(30);
    RunMenuCommand("SelectChannel 0"s);
    ExpectOutput("Channel is out of range\n"sv);
    EXPECT_EQ(tv_.GetChannel(), 30);
}

TEST_F(ControllerWithTurnedOnTV, on_SelectChannel_bad_channel_minus10) {
    tv_.SelectChannel(30);
    RunMenuCommand("SelectChannel -10"s);
    ExpectOutput("Channel is out of range\n"sv);
    EXPECT_EQ(tv_.GetChannel(), 30);
}

TEST_F(ControllerWithTurnedOnTV, on_SelectChannel_bad_channel_99999) {
    tv_.SelectChannel(30);
    RunMenuCommand("SelectChannel 99999"s);
    ExpectOutput("Channel is out of range\n"sv);
    EXPECT_EQ(tv_.GetChannel(), 30);
}

TEST_F(ControllerWithTurnedOnTV, on_SelectPreviousChannel_1) {
    tv_.SelectChannel(42);
    tv_.SelectChannel(30);
    RunMenuCommand("SelectPreviousChannel"s);
    ExpectEmptyOutput();
    EXPECT_EQ(tv_.GetChannel(), 42);
}

TEST_F(ControllerWithTurnedOnTV, on_SelectPreviousChannel_2) {
    tv_.SelectChannel(42);
    tv_.SelectChannel(30);
    tv_.SelectLastViewedChannel();
    RunMenuCommand("SelectPreviousChannel"s);
    ExpectEmptyOutput();
    EXPECT_EQ(tv_.GetChannel(), 30);
}

TEST_F(ControllerWithTurnedOnTV, on_SelectPreviousChannel_3) {
    tv_.SelectChannel(42);
    tv_.SelectChannel(30);
    tv_.SelectLastViewedChannel();
    tv_.SelectLastViewedChannel();
    RunMenuCommand("SelectChannel -54"s);
    ExpectOutput("Channel is out of range\n"sv);
    RunMenuCommand("SelectPreviousChannel"s);
    EXPECT_EQ(tv_.GetChannel(), 42);
}
