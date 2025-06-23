#include "strings.h"
#include "Log.h"



std::vector<std::string_view> split(std::string_view s, std::string_view delimiter)
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