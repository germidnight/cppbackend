#define BOOST_TEST_MODULE urlencode tests
#include <boost/test/unit_test.hpp>

#include "../src/urldecode.h"



BOOST_AUTO_TEST_CASE(UrlDecode_tests) {
    using namespace std::literals;

    BOOST_TEST(UrlDecode(""sv) == ""s);
    BOOST_TEST(UrlDecode("+"sv) == " "s);
    BOOST_TEST(UrlDecode("hello_world"sv) == "hello_world"s);
    BOOST_TEST(UrlDecode("HELLO+world"sv) == "HELLO world"s);
    BOOST_TEST(UrlDecode("hello%20world%21%21%21"sv) == "hello world!!!"s);
    BOOST_TEST(UrlDecode("special_ALL%20%21%23%24%26%27%28%29%2a%2b%2C%2F%3A%3B%3d%3f%40%5B%5D"sv) == "special_ALL !#$&'()*+,/:;=?@[]"s);
    BOOST_TEST(UrlDecode("all%20%21%23%24%26%27%28%29%2A%2B%2C%2F%3A%3B%3D%3F%40%5B%5D%20special%20"sv) == "all !#$&'()*+,/:;=?@[] special "s);
    BOOST_TEST(UrlDecode("%21%23%24%26%27%28%29%2A%2B%2C%2F%3A%3B%3D%3F%40%5B%5D%20specials"sv) == "!#$&'()*+,/:;=?@[] specials"s);
    BOOST_TEST(UrlDecode("%21__ok%20%23%24%20%20%26%20some%20%27%20symbols%20%28spec%29%2A%2B%2C%2F%3A%3B%3D%3F%20%20%40%5Bials%5D"sv) ==
                            "!__ok #$  & some ' symbols (spec)*+,/:;=?  @[ials]"s);
    BOOST_TEST(UrlDecode("%21__ok%20%23%24BASH%20%20%26%26%20%27symbols%27%20%28spec%29%20%2A%20val%20%2B%20ZZZ%2C%20%2F%20Y%3A%20%20%20%3BE%3D%3F466%20%20%40%5Bials%5D"sv) ==
               "!__ok #$BASH  && 'symbols' (spec) * val + ZZZ, / Y:   ;E=?466  @[ials]"s);
    BOOST_TEST(UrlDecode("%20"sv) == " "s);
    BOOST_TEST(UrlDecode("%21%21%21%21%21%21%21%21"sv) == "!!!!!!!!"s);
    BOOST_TEST(UrlDecode("%21%23%24%26%27%28%29%2A%2B%2C%2F%3A%3B%3D%3F%40%5B%5D"sv) == "!#$&'()*+,/:;=?@[]"s);

    BOOST_CHECK_THROW(UrlDecode("hello%20world%21%21%2"sv), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("hello%20world%21%21%"sv), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("hello%2world%21"sv), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("hello%world%21"sv), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("%1"sv), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("%"sv), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("%%%%%"sv), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("%2%21%21%21%21%21%21%21hello"sv), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("ggg%C___"sv), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("f%3S"sv), std::invalid_argument);
}
