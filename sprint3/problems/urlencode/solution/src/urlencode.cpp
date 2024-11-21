#include "urlencode.h"

#include <iomanip>
#include <set>
#include <sstream>

using namespace std::literals;

std::string UrlEncode(std::string_view str) {
    const std::set<char> not_encoded_syms = {'-', '_', '.', '~'};
    std::string result;
    for (size_t pos = 0; pos < str.length(); ++pos) {
        if (str[pos] == ' ') {
            result.push_back('+');
        } else if ((std::isalnum(str[pos]) != 0) || (not_encoded_syms.count(str[pos]) > 0)) {
            result.push_back(str[pos]);
        } else {
            std::stringstream ss;
            ss << "%"s << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(str[pos]);
            result.append(ss.str());
        }
    }
    return std::move(result);
}
