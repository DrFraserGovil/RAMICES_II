#pragma once
#include <string>
#include <vector>
#include <type_traits> // For std::is_arithmetic_v, std::enable_if_t, etc.
#include <sstream>     // For std::stringstream for more controlled numeric to_string
#include <string_view> // For delimiter in vector case


// Primary template for basic types (numbers, chars, bools) that can be stringifyed to string
// We'll use SFINAE to ensure this only applies to arithmetic types, and then specialize others.
template<typename T, typename = void> // Generic template
struct MakeStringStruct
{
    // Default implementation for arithmetic types
    // Using stringstream for more control over floating point precision than std::to_string
    template<typename U = T, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    static std::string stringify(const U& value) {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
};

// Specialization for `bool`
template<>
struct MakeStringStruct<bool, void> {
    static std::string stringify(bool value) {
        return value ? "true" : "false"; // Consistent with common text formats
    }
};

// Specialization for `char`
template<>
struct MakeStringStruct<char, void> {
    static std::string stringify(char value) {
        return std::string(1, value); // Converts a single char into a std::string
    }
};

// Specialization for `std::string`
template<>
struct MakeStringStruct<std::string, void> {
    static std::string stringify(const std::string& value) {
        // For plain strings, just return the value.
        // If your strings can contain the delimiter, you might need quoting logic here.
        return value;
    }
};


// Specialization for `std::string_view`
template<>
struct MakeStringStruct<std::string_view, void> {
    static std::string stringify(const std::string_view& value) {
        // For plain strings, just return the value.
        // If your strings can contain the delimiter, you might need quoting logic here.
        return std::string(value);
    }
};

// Specialization for `std::vector<T_Inner>`
template<typename T_Inner>
struct MakeStringStruct<std::vector<T_Inner>, void> {
    // This `stringify` function takes an additional `delimiter_str` argument.
    // The `Setting::Parameter` object (which stores the delimiter) will provide this.

	static std::string stringify(const std::vector<T_Inner>& vec)
	{
		return stringify(vec,", ");
	}

    static std::string stringify(const std::vector<T_Inner>& vec, std::string_view delimiter_str) {
        std::string result = "["; // Consistent with your `StripEndCaps` for parsing vectors

        for (size_t i = 0; i < vec.size(); ++i) {
            // Recursively call ToStringConverter for the inner type
            result += MakeStringStruct<T_Inner>::stringify(vec[i]);
            if (i < vec.size() - 1) {
                result += delimiter_str;
            }
        }
        result += "]";
        return result;
    }
};

template<typename T>
std::string inline MakeString(T obj)
{
	return MakeStringStruct<T>::stringify(obj);
};

template<typename T>
std::string inline MakeString(T obj,std::string delimiter)
{
	return MakeStringStruct<T>::stringify(obj,delimiter);
};