#include <catch2/catch_test_macros.hpp>

#include "../src/htmldecode.h"

using namespace std::literals;

TEST_CASE("Text without mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode(""sv) == ""s);
    CHECK(HtmlDecode("hello"sv) == "hello"s);
    CHECK(HtmlDecode("&lt;&gt;"sv) == "<>"s);
    CHECK(HtmlDecode("HELLO &AMPWORLD&AMP;"sv) == "HELLO &WORLD&"s);
    CHECK(HtmlDecode("Johnson&amp;Johnson"sv) == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&ampJohnson"sv) == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&AMP;Johnson"sv) == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&AMPJohnson"sv) == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&Johnson"sv) == "Johnson&Johnson"s);
    CHECK(HtmlDecode("&abracadabra"sv) == "&abracadabra"s);
    CHECK(HtmlDecode("&APOSgi RA&apos;"sv) == "\'gi RA\'"s);
    CHECK(HtmlDecode("&Apos;  &aPOs"sv) == "&Apos;  &aPOs"s);
    CHECK(HtmlDecode("&quot;&QuOt;&quo&QUOT"sv) == "\"&QuOt;&quo\""s);
}

// Напишите недостающие тесты самостоятельно
