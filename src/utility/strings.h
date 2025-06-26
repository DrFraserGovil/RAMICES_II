#pragma once
#include <vector>
#include <string_view>

#include <typeinfo>
#include <type_traits> // Required for std::enable_if_t, std::is_same_v
#include <cctype>    // std::tolower
#include <algorithm> // std::equal



/*! @brief Splits a string into a vector, with each element indicated by the delimiter string
    @warning The output is a vector of string_views - references to the original string. This has the limitation that the output is only meaningful so long as the original string survives. Copies out of the string_view do persist
    @param string A (view) of a string to be split
    @param delimiter The string which indicates a 'break'. Delimiters do not appear in the split output
    @returns A vector of windows into the original string, indicating which elements have been grouped together.
*/
std::vector<std::string_view> split(std::string_view string, std::string_view delimiter);


//! Removes leading or trailing whitespace from a string_view
//! @param sv The original string_view
//! @returns A modified string_view with no leading or trailing whitespace
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

//! Removes leading or trailing whitespace from a string_view and trims any 'comments' from the string.
//! @details Comments are signified by the 'commentIndicator', and all text after the indicator is removed.
//! With indicator '#', the string "hello, my name is #Put your name here" would be trimmed to "hello my name is";
//! @param sv The original string_view
//! @param commentIndicator The string after which all text is to be removed.
//! @returns A modified string_view with no leading or trailing whitespace
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

//! Performs a case-insensitive equality check on two strings
//! @returns True if a and b are (aside from cases) equal strings
bool inline insensitiveEquals(const std::string_view a, const std::string_view b)
{
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), insensitiveEqualsChar);
}

