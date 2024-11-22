#include <boost/test/unit_test.hpp>
#include <iostream>
#include <sstream>

#include "../src/controller.h"
#include "boost_test_helpers.h"

using namespace std::literals;

struct ControllerFixture {
    TV tv;
    std::istringstream input;
    std::ostringstream output;
    Menu menu{input, output};
    Controller controller{tv, menu};

    void RunMenuCommand(std::string command) {
        input.str(std::move(command));
        input.clear();
        menu.Run();
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
        BOOST_TEST(output.str() == expected);
    }
};

struct WhenTVIsOffFixture : ControllerFixture {
    WhenTVIsOffFixture() {
        BOOST_REQUIRE(!tv.IsTurnedOn());
    }
};

BOOST_AUTO_TEST_SUITE(Controller_)

BOOST_FIXTURE_TEST_SUITE(WhenTVIsOff, WhenTVIsOffFixture)
BOOST_AUTO_TEST_CASE(on_Info_command_prints_that_tv_is_off) {
    RunMenuCommand("Info"s);
    ExpectOutput("TV is turned off\n"sv);
    BOOST_TEST(!tv.IsTurnedOn());
}
BOOST_AUTO_TEST_CASE(on_Info_command_prints_error_message_if_comand_has_any_args) {
    RunMenuCommand("Info some extra args"s);
    BOOST_TEST(!tv.IsTurnedOn());
    ExpectExtraArgumentsErrorInOutput("Info"sv);
}
BOOST_AUTO_TEST_CASE(on_Info_command_ignores_trailing_spaces) {
    RunMenuCommand("Info  "s);
    ExpectOutput("TV is turned off\n"sv);
}
BOOST_AUTO_TEST_CASE(on_TurnOn_command_turns_TV_on) {
    RunMenuCommand("TurnOn"s);
    BOOST_TEST(tv.IsTurnedOn());
    ExpectEmptyOutput();
}
BOOST_AUTO_TEST_CASE(on_TurnOn_command_with_some_arguments_prints_error_message) {
    RunMenuCommand("TurnOn some args"s);
    BOOST_TEST(!tv.IsTurnedOn());
    ExpectExtraArgumentsErrorInOutput("TurnOn"sv);
}

BOOST_AUTO_TEST_CASE(onSelectChannel_if_TV_is_off) {
    RunMenuCommand("SelectChannel 3"s);
    ExpectOutput("TV is turned off\n"sv);
}

BOOST_AUTO_TEST_CASE(onSelectPreviousChannel_if_TV_is_off) {
    RunMenuCommand("SelectPreviousChannel"s);
    ExpectOutput("TV is turned off\n"sv);
}

BOOST_AUTO_TEST_SUITE_END()

struct WhenTVIsOnFixture : ControllerFixture {
    WhenTVIsOnFixture() {
        tv.TurnOn();
    }
};

BOOST_FIXTURE_TEST_SUITE(WhenTVIsOn, WhenTVIsOnFixture)
BOOST_AUTO_TEST_CASE(on_TurnOff_command_turns_tv_off) {
    RunMenuCommand("TurnOff"s);
    BOOST_TEST(!tv.IsTurnedOn());
    ExpectEmptyOutput();
}
BOOST_AUTO_TEST_CASE(on_TurnOff_command_with_some_arguments_prints_error_message) {
    RunMenuCommand("TurnOff some args"s);
    BOOST_TEST(tv.IsTurnedOn());
    ExpectExtraArgumentsErrorInOutput("TurnOff"sv);
}

BOOST_AUTO_TEST_CASE(on_Info_prints_current_channel) {
    tv.SelectChannel(42);
    RunMenuCommand("Info"s);
    ExpectOutput("TV is turned on\nChannel number is 42\n"sv);
}

BOOST_AUTO_TEST_CASE(onSelectChannel_good_channels) {
    RunMenuCommand("SelectChannel 30"s);
    ExpectEmptyOutput();
    BOOST_CHECK(tv.GetChannel() == 30);

    RunMenuCommand("SelectChannel 1"s);
    ExpectEmptyOutput();
    BOOST_CHECK(tv.GetChannel() == 1);

    RunMenuCommand("SelectChannel 99"s);
    ExpectEmptyOutput();
    BOOST_CHECK(tv.GetChannel() == 99);
}

BOOST_AUTO_TEST_CASE(on_SelectChannel_bad_channel_A) {
    tv.SelectChannel(30);
    RunMenuCommand("SelectChannel A"s);
    ExpectOutput("Invalid channel\n"sv);
    BOOST_CHECK(tv.GetChannel() == 30);
}

BOOST_AUTO_TEST_CASE(on_SelectChannel_bad_channel_float3_14) {
    tv.SelectChannel(30);
    RunMenuCommand("SelectChannel 3.14"s);
    ExpectOutput("Invalid channel\n"sv);
    BOOST_CHECK(tv.GetChannel() == 30);
}

BOOST_AUTO_TEST_CASE(on_SelectChannel_bad_channel_0) {
    tv.SelectChannel(30);
    RunMenuCommand("SelectChannel 0"s);
    ExpectOutput("Channel is out of range\n"sv);
    BOOST_CHECK(tv.GetChannel() == 30);
}

BOOST_AUTO_TEST_CASE(on_SelectChannel_bad_channel_minus10) {
    tv.SelectChannel(30);
    RunMenuCommand("SelectChannel -10"s);
    ExpectOutput("Channel is out of range\n"sv);
    BOOST_CHECK(tv.GetChannel() == 30);
}

BOOST_AUTO_TEST_CASE(on_SelectChannel_bad_channel_99999) {
    tv.SelectChannel(30);
    RunMenuCommand("SelectChannel 99999"s);
    ExpectOutput("Channel is out of range\n"sv);
    BOOST_CHECK(tv.GetChannel() == 30);
}

BOOST_AUTO_TEST_CASE(on_SelectPreviousChannel_1) {
    tv.SelectChannel(42);
    tv.SelectChannel(30);
    RunMenuCommand("SelectPreviousChannel"s);
    ExpectEmptyOutput();
    BOOST_CHECK(tv.GetChannel() == 42);
}

BOOST_AUTO_TEST_CASE(on_SelectPreviousChannel_2) {
    tv.SelectChannel(42);
    tv.SelectChannel(30);
    tv.SelectLastViewedChannel();
    RunMenuCommand("SelectPreviousChannel"s);
    ExpectEmptyOutput();
    BOOST_CHECK(tv.GetChannel() == 30);
}

BOOST_AUTO_TEST_CASE(on_SelectPreviousChannel_3) {
    tv.SelectChannel(42);
    tv.SelectChannel(30);
    tv.SelectLastViewedChannel();
    tv.SelectLastViewedChannel();
    RunMenuCommand("SelectChannel -54"s);
    ExpectOutput("Channel is out of range\n"sv);
    RunMenuCommand("SelectPreviousChannel"s);
    BOOST_CHECK(tv.GetChannel() == 42);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
