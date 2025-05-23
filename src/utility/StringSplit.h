#pragma once
#include <vector>
#include <string_view>
#include "Log.h"
#include <typeinfo>
#include <type_traits> // Required for std::enable_if_t, std::is_same_v

//Splits a string -- but has the limitation that the output is only meaningful so long as the original string survives.
//Copies out of the string view do persist
std::vector<std::string_view> split(std::string_view s, std::string_view delimiter) {
    std::vector<std::string_view> tokens;
    size_t start = 0;
    size_t end = s.find(delimiter);
    size_t delim_len = delimiter.length();

    while (end != std::string_view::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + delim_len;
        end = s.find(delimiter, start);
    }
    tokens.push_back(s.substr(start)); // Add the last token
    return tokens;
}
