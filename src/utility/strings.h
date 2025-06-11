#pragma once
#include <vector>
#include <string_view>
#include "Log.h"
#include <typeinfo>
#include <type_traits> // Required for std::enable_if_t, std::is_same_v
#include <cctype>    // std::tolower
#include <algorithm> // std::equal



//Splits a string -- but has the limitation that the output is only meaningful so long as the original string survives.
//Copies out of the string view do persist
std::vector<std::string_view> inline split(std::string_view s, std::string_view delimiter) 
{
    if (delimiter.size() == 0)
    {
        LOG(ERROR) << "Cannot use an empty delimiter to split, this is meaningless";
        throw std::runtime_error("Split called with empty delimiter");
    }
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


std::string_view inline trim(std::string_view sv)
{
    // Find the first non-whitespace character
    size_t first = 0;
    while (first < sv.length() && std::isspace(static_cast<unsigned char>(sv[first])))
    {
        first++;
    }

    // Find the last non-whitespace character
    size_t last = sv.length();
    while (last > first && std::isspace(static_cast<unsigned char>(sv[last - 1])))
    {
        last--;
    }

    return sv.substr(first, last - first);
}

std::string_view inline trim(std::string_view sv,const std::string & commentIndicator)
{
    auto commentStart = sv.find(commentIndicator);
    if (commentStart != std::string::npos)
    {
        sv = sv.substr(0,commentStart);
    }
    return trim(sv);
}




bool inline insensitiveEqualsChar(char a, char b)
{
    return std::tolower(static_cast<unsigned char>(a)) ==
           std::tolower(static_cast<unsigned char>(b));
}


bool inline insensitiveEquals(const std::string_view a, const std::string_view b)
{
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), insensitiveEqualsChar);
}

