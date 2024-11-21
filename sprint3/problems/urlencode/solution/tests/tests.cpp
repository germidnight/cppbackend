#include <gtest/gtest.h>

#include "../src/urlencode.h"

using namespace std::literals;

TEST(UrlEncodeTestSuite, OrdinaryCharsAreNotEncoded) {
    EXPECT_EQ(UrlEncode("hello"sv), "hello"s);
    EXPECT_EQ(UrlEncode(""sv), ""s);
    EXPECT_EQ(UrlEncode(" "sv), "+"s);
    EXPECT_EQ(UrlEncode("           "sv), "+++++++++++"s);
    EXPECT_EQ(UrlEncode("!#$&'()*+,/:;=?@[]"sv), "%21%23%24%26%27%28%29%2a%2b%2c%2f%3a%3b%3d%3f%40%5b%5d"s);
    EXPECT_EQ(UrlEncode("\nA"sv), "%0aA"s);
    EXPECT_EQ(UrlEncode("\t\r\n"sv), "%09%0d%0a"s);
    EXPECT_EQ(UrlEncode("\nhello\nWORLD"sv), "%0ahello%0aWORLD"s);
}

/* Напишите остальные тесты самостоятельно */
