#include "urldecode.h"

#include <cctype>
#include <charconv>
#include <set>
#include <stdexcept>

using namespace std::literals;

std::string UrlDecode(std::string_view str) {
    const std::set<char> not_encoded_syms = {'-', '_', '.', '~'};
    std::string result = "";
    size_t pos = 0;
    while (pos < str.length()) {
        if (str[pos] == '+') {
            result.push_back(' ');
        } else if ((std::isalnum(str[pos]) != 0) || (not_encoded_syms.count(str[pos]) > 0)) {
            result.push_back(str[pos]);
        } else if (str[pos] == '%') {
            if (((pos + 1) >= str.length()) || ((pos + 2) >= str.length())) {
                throw std::invalid_argument("Invalid URL-encoded string"s);
            }
            long symbol{};
            auto res = std::from_chars(str.data() + pos+1, str.data() + pos+3, symbol, 16);
            if ((res.ec == std::errc()) && (res.ptr == (str.data() + pos+3))) {
                result.push_back(static_cast<char>(symbol));
            } else {
                throw std::invalid_argument("Invalid URL-encoded string"s);
            }
            pos += 2;
        } else {
            throw std::invalid_argument("Invalid URL-encoded string"s);
        }
        ++pos;
    }
    return result;
}
