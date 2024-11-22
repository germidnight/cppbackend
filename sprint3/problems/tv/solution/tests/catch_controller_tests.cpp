#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "../src/controller.h"

SCENARIO("Controller", "[Controller]") {
    using namespace std::literals;
    GIVEN("Controller and TV") {
        TV tv;
        std::istringstream input;
        std::ostringstream output;
        Menu menu{input, output};
        Controller controller{tv, menu};

        auto run_menu_command = [&menu, &input](std::string command) {
            input.str(std::move(command));
            input.clear();
            menu.Run();
        };
        auto expect_output = [&output](std::string_view expected) {
            // В g++ 10.3 не реализован метод ostringstream::view(), поэтому приходится
            // использовать метод str()
            // Также в conan есть баг, из-за которого Catch2 не подхватывает поддержку string_view:
            // https://github.com/conan-io/conan-center-index/issues/13993
            // Поэтому expected преобразуется к строке, чтобы обойти ошибку компиляции
            CHECK(output.str() == std::string{expected});
        };
        auto expect_extra_arguments_error = [&expect_output](std::string_view command) {
            expect_output("Error: the "s.append(command).append(
                " command does not require any arguments\n"sv));
        };
        auto expect_empty_output = [&expect_output] {
            expect_output({});
        };

        WHEN("The TV is turned off") {
            AND_WHEN("Info command is entered without arguments") {
                run_menu_command("Info"s);

                THEN("output contains info that TV is turned off") {
                    expect_output("TV is turned off\n"s);
                }
            }

            AND_WHEN("Info command is entered with some arguments") {
                run_menu_command("Info some extra arguments");

                THEN("Error message is printed") {
                    expect_extra_arguments_error("Info"s);
                }
            }

            AND_WHEN("Info command has trailing spaces") {
                run_menu_command("Info  "s);

                THEN("output contains information that TV is turned off") {
                    expect_output("TV is turned off\n"s);
                }
            }

            AND_WHEN("TurnOn command is entered without arguments") {
                run_menu_command("TurnOn"s);

                THEN("TV is turned on") {
                    CHECK(tv.IsTurnedOn());
                    expect_empty_output();
                }
            }

            AND_WHEN("TurnOn command is entered with some arguments") {
                run_menu_command("TurnOn some args"s);

                THEN("the error message is printed and TV is not turned on") {
                    CHECK(!tv.IsTurnedOn());
                    expect_extra_arguments_error("TurnOn"s);
                }
            }
            /* Протестируйте остальные аспекты поведения класса Controller, когда TV выключен */
            AND_WHEN("SelectChannel command whet TV is off") {
                run_menu_command("SelectChannel 3"s);

                THEN("output contains information that TV is turned off") {
                    CHECK(!tv.IsTurnedOn());
                    expect_output("TV is turned off\n"s);
                }
            }
            AND_WHEN("SelectChannel command whet TV is off") {
                run_menu_command("SelectPreviousChannel"s);

                THEN("output contains information that TV is turned off") {
                    CHECK(!tv.IsTurnedOn());
                    expect_output("TV is turned off\n"s);
                }
            }
        }

        WHEN("The TV is turned on") {
            tv.TurnOn();
            AND_WHEN("TurnOff command is entered without arguments") {
                run_menu_command("TurnOff"s);

                THEN("TV is turned off") {
                    CHECK(!tv.IsTurnedOn());
                    expect_empty_output();
                }
            }

            AND_WHEN("TurnOff command is entered with some arguments") {
                run_menu_command("TurnOff some args");

                THEN("the error message is printed and TV is not turned off") {
                    CHECK(tv.IsTurnedOn());
                    expect_extra_arguments_error("TurnOff"s);
                }
            }
// Включите эту секцию, после того как реализуете метод TV::SelectChannel
            AND_WHEN("Info command is entered without arguments") {
                tv.SelectChannel(12);
                run_menu_command("Info"s);

                THEN("current channel is printed") {
                    expect_output("TV is turned on\nChannel number is 12\n"s);
                }
            }

            /* Протестируйте остальные аспекты поведения класса Controller, когда TV включен */
            AND_WHEN("SelectChannel coomand on good channels") {
                tv.SelectChannel(40);
                THEN("TV switched to channel 40") {
                    CHECK(tv.GetChannel() == 40);
                }

                tv.SelectChannel(1);
                AND_THEN("TV switched to channel 1") {
                    CHECK(tv.GetChannel() == 1);
                }

                tv.SelectChannel(99);
                AND_THEN("TV switched to channel 99") {
                    CHECK(tv.GetChannel() == 99);
                }
            }

            AND_WHEN("SelectChannel command on bad channel u") {
                tv.SelectChannel(50);
                run_menu_command("SelectChannel u"s);

                THEN("Error message expected, and channel is not changed") {
                    expect_output("Invalid channel\n"sv);
                    CHECK(tv.GetChannel() == 50);
                }
            }
            AND_WHEN("SelectChannel command on bad channel 2.71") {
                tv.SelectChannel(11);
                run_menu_command("SelectChannel 2.71"s);

                THEN("Error message expected, and channel is not changed") {
                    expect_output("Invalid channel\n"sv);
                    CHECK(tv.GetChannel() == 11);
                }
            }
            AND_WHEN("SelectChannel command on bad channel 0") {
                tv.SelectChannel(30);
                run_menu_command("SelectChannel 0"s);

                THEN("Error message expected? and channel is not changed") {
                    expect_output("Channel is out of range\n"sv);
                    CHECK(tv.GetChannel() == 30);
                }
            }
            AND_WHEN("SelectChannel command on bad channel -45") {
                tv.SelectChannel(30);
                run_menu_command("SelectChannel -45"s);

                THEN("Error message expected? and channel is not changed") {
                    expect_output("Channel is out of range\n"sv);
                    CHECK(tv.GetChannel() == 30);
                }
            }
            AND_WHEN("SelectChannel command on bad channel 99999") {
                tv.SelectChannel(30);
                run_menu_command("SelectChannel 99999"s);

                THEN("Error message expected? and channel is not changed") {
                    expect_output("Channel is out of range\n"sv);
                    CHECK(tv.GetChannel() == 30);
                }
            }

            AND_WHEN("SelectPreviousChannel command 1") {
                tv.SelectChannel(42);
                tv.SelectChannel(30);
                run_menu_command("SelectPreviousChannel"s);

                THEN("No message expected, and channel is must be 42") {
                    expect_empty_output();
                    CHECK(tv.GetChannel() == 42);
                }
            }
            AND_WHEN("SelectPreviousChannel command 2") {
                tv.SelectChannel(42);
                tv.SelectChannel(30);
                tv.SelectLastViewedChannel();
                run_menu_command("SelectPreviousChannel"s);

                THEN("No message expected, and channel is must be 30") {
                    expect_empty_output();
                    CHECK(tv.GetChannel() == 30);
                }
            }
            AND_WHEN("SelectPreviousChannel command 3") {
                tv.SelectChannel(42);
                tv.SelectChannel(30);
                tv.SelectLastViewedChannel();
                tv.SelectLastViewedChannel();
                run_menu_command("SelectPreviousChannel"s);

                THEN("No message expected, and channel is must be 30") {
                    expect_empty_output();
                    CHECK(tv.GetChannel() == 42);
                }
            }
        }
    }
}
