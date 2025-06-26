#pragma once
#include <string>
#include <vector>
#include <type_traits> // For std::is_arithmetic_v, std::enable_if_t, etc.
#include <sstream>     // For std::stringstream for more controlled numeric to_string
#include <string_view> // For delimiter in vector case


/*! @brief Internal interface for MakeString. 
    
@details As with convert(), we use `typename`
    , an internal struct and SFINAE to enforce type behaviour and allow vector partial specialisation. 
    Default struct only applies to numeric types. Overloads handle the others
*/
template<typename T, typename = void> // Generic template
struct MakeStringStruct
{
    // Default implementation for arithmetic types
    // Using stringstream for more control over floating point precision than std::to_string
    template<typename U = T, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    static std::string stringify(const U& value) {
        // std::stringstream ss;
        // ss << value;
        return std::to_string(value);
    }
};

//! @brief Specialization for `bool`
//! @param value A boolean true or false
//! @returns The string 'true' or 'false', as appropriate
template<>
struct MakeStringStruct<bool, void> {
    static std::string stringify(bool value) {
        return value ? "true" : "false"; // Consistent with common text formats
    }
};

//! Specialization for `char`
//! @details Converting chars to strings is surprisingly unintuitive. This makes it easier.
template<>
struct MakeStringStruct<char, void> {
    static std::string stringify(char value) {
        return std::string(1, value); // Converts a single char into a std::string
    }
};

//! Specialization for `std::string` 
//! @details This exists for performance reasons more than anything else - it's quicker than the streaming used for numerics.
template<>
struct MakeStringStruct<std::string, void> {
    static std::string stringify(const std::string& value) {
        // For plain strings, just return the value.
        // If your strings can contain the delimiter, you might need quoting logic here.
        return value;
    }
};


//! Specialization for `std::string_view`
template<>
struct MakeStringStruct<std::string_view, void> {
    static std::string stringify(const std::string_view& value) {
        // For plain strings, just return the value.
        // If your strings can contain the delimiter, you might need quoting logic here.
        return std::string(value);
    }
};

template<typename T_Inner>
struct MakeStringStruct<std::vector<T_Inner>, void> {
    //! @brief Specialization for `std::vector<T_Inner>`
    //! @details Calls with default delimiter
	static std::string stringify(const std::vector<T_Inner>& vec)
	{
        return stringify(vec,", ");
	}
    
    //! @brief Specialization for `std::vector<T_Inner>`
    //! @details Calls with custom delimiter
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

/*! @brief Converts the object into a string. 
    @brief If the object is a vector, the default delimiter - "," - is used.
    @param obj An object (usually a numeric, boolean or vector) to be converted
    @returns A string representation
*/
template<typename T>
std::string inline MakeString(T obj)
{
	return MakeStringStruct<T>::stringify(obj);
};

/*! @brief Converts a vector into a string
    @param obj An object (usually a numeric, boolean or vector) to be converted
    @param delimiter The character(s) which delimit the elements of the vector
    @returns A string representation
*/
template<typename T>
std::string inline MakeString(T obj,std::string delimiter)
{
	return MakeStringStruct<T>::stringify(obj,delimiter);
};