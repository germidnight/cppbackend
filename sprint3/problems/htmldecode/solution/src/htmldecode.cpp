#include "htmldecode.h"

std::string HtmlDecode(std::string_view str) {
    std::string result = "";

    for (size_t pos = 0; pos < str.length(); ++pos) {
        if (str[pos] == '&') {
            if ((pos + 5) < str.length()) {
                std::string_view substr = str.substr(pos + 1, 5);
                if ((substr.compare("quot;") == 0) || (substr.compare("QUOT;") == 0)) {
                    pos += 5;
                    result.push_back('"');
                    continue;
                }
                if ((substr.compare("apos;") == 0) || (substr.compare("APOS;") == 0)) {
                    pos += 5;
                    result.push_back('\'');
                    continue;
                }
            }
            if ((pos + 4) < str.length()) {
                std::string_view substr = str.substr(pos + 1, 4);
                if ((substr.compare("quot") == 0) || (substr.compare("QUOT") == 0)) {
                    pos += 4;
                    result.push_back('"');
                    continue;
                }
                if ((substr.compare("amp;") == 0) || (substr.compare("AMP;") == 0)) {
                    pos += 4;
                    result.push_back('&');
                    continue;
                }
                if ((substr.compare("apos") == 0) || (substr.compare("APOS") == 0)) {
                    pos += 4;
                    result.push_back('\'');
                    continue;
                }
            }
            if ((pos + 3) < str.length()) {
                std::string_view substr = str.substr(pos + 1, 3);
                if ((substr.compare("amp") == 0) || (substr.compare("AMP") == 0)) {
                    pos += 3;
                    result.push_back('&');
                    continue;
                }
                if ((substr.compare("lt;") == 0) || (substr.compare("LT;") == 0)) {
                    pos += 3;
                    result.push_back('<');
                    continue;
                }
                if ((substr.compare("gt;") == 0) || (substr.compare("GT;") == 0)) {
                    pos += 3;
                    result.push_back('>');
                    continue;
                }
            }
            if ((pos + 2) < str.length()) {
                std::string_view substr = str.substr(pos + 1, 2);
                if ((substr.compare("lt") == 0) || (substr.compare("LT") == 0)) {
                    pos += 2;
                    result.push_back('<');
                    continue;
                }
                if ((substr.compare("gt") == 0) || (substr.compare("GT") == 0)) {
                    pos += 2;
                    result.push_back('>');
                    continue;
                }
            }
        }
        result.push_back(str[pos]);
    }
    return result;
}