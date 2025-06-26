#pragma once
#include <string_view>
#include <vector>
#include <regex>
#include <charconv>
#include "Log.h"
#include "strings.h"
/*
    This file provides a robust return-value interface for converting string-views (and, implicitly, strings) into a candidate type
    This type can be any value accepted by std::from_chars, or those which can be implicitly converted to from that type

    Converter also handles conversion of multiple values at once (of the same type) into a vector.

    Special overloads are provided that allow direct stringview->string conversion.
*/


/*!
    The backend structure for the convert function. 
    @details Required because functions are not able to undergo partial specialisation, whilst classes can. This therefore amounts to some compile-time wizardry.
*/
template<typename T>
struct Converter
{
    /*!
        The default conversion operator -- where the actual conversion happens. 
        @details Specialisations (i.e. for new convert-types) must define their own version of this function
         @param sv A string_view (implicitly converted from std::strings) representing a value to be converted
        @returns A value of type T corresponding to the input string
    */
	static T internalConvert(std::string_view sv)
	{
        sv = trim(sv,"//");
        RejectEmpty(sv);
        
        //create an object and read from_chars into it. Some implicit type conversion is allowed here (i.e. if T is a bool)
        T output;
		auto result = std::from_chars(sv.data(), sv.data() + sv.size(),output);

        CheckErrors(result,sv);		
				
		return output;
	}

    private:
    /*!Throws errors if the internalConvert reached some undesirable state
    @throws logic_error If the characters could not be converted into numerics (i.e. ab)
    @throws logic_error If some, but not all, characters could be converted into numerics (i.e. 123ab)
    @throws logic_error If the result is out of range (i.e. convert<int>(INT_MAX + 10))
    */
    static void CheckErrors(std::from_chars_result & result,std::string_view sv)
    {
        if (result.ec == std::errc() &&  (result.ptr != sv.data() + sv.size()))
		{ 
			LOG(ERROR) << "Partial conversion of `" << sv << "` to type " << typeid(T).name() << " unconverted characters were: " << std::string_view(result.ptr, sv.data() + sv.size() - result.ptr);
			throw std::logic_error("Could not complete conversion");
		}
		else if (result.ec == std::errc::invalid_argument) 
        {
			LOG(ERROR) <<  "Error: Invalid argument for conversion: '" << sv   << "` to type " << typeid(T).name()<< "\n";
			throw std::logic_error("Could not complete conversion");
		} 
        else if (result.ec == std::errc::result_out_of_range)
        {
			LOG(ERROR) <<  "Error: Result out of range for conversion: '" << sv << "` to type " << typeid(T).name() << "\n";
			throw std::logic_error("Could not complete conversion");
		}
        return;
    }

    //! Check if converted string is empty @throws logic_error If target string is empty
    static void RejectEmpty(std::string_view sv)
    {
        if (sv.empty()) 
        {
            LOG(ERROR) << "Cannot convert an empty string to to type " << typeid(T).name();
			throw std::logic_error("Could not complete conversion");
		} 
    }
};

//! @brief Full specialization for std::string (because from_chars can't do strings!)
//! @details Could just rely on user to call this manually, but it's nicer to have a unified interface
template <>
struct Converter<std::string> {
    //! Performs a basic string-initialisation.
    static std::string internalConvert(std::string_view sv) 
	{

        return std::string(sv);
    }
};

//!Full specialization for boolean (because from_chars can't do boolean)
template <>
struct Converter<bool> {
    /*!
        Checks if the input string is 1,0, true or false (case insensitive), and converts to the relevant bool
        @throws runtime_error If the string is not in ["1","0", "true" ,"false"]
    */
    static bool internalConvert(std::string_view sv) 
	{
        auto snap = trim(sv,"//");
        if (snap == "1" || insensitiveEquals(snap,"true"))
        {
            return true;
        }
        if (snap == "0" || insensitiveEquals(snap,"false"))
        {
            return false;
        }

        LOG(ERROR) << "Cannot convert string " << sv << "to boolean";
        throw std::runtime_error("Cannot convert string to boolean");
    }
};

//! Full specialisation for single chars
//! @details Converting between strings and chars is notoriously more difficult than it feels it should be
template <>
struct Converter<char> {
    /*! Converts a string into a char
        @throws logic_error if the string is not a single character long
    */
    static char internalConvert(std::string_view sv)
    {
        // Trim whitespace first
        sv = trim(sv,"//");
        // A single char conversion should only accept a single character string_view
        if (sv.length() != 1) {
            LOG(ERROR) << "Cannot convert string_view '" << sv << "' to char: Expected a single character.";
            throw std::logic_error("Could not complete conversion: Expected single character.");
        }
        return sv[0];
    }
    // No need for CheckErrors or RejectEmpty for char, as the length check covers it.
};


/*! Specialization for double, activated on Apple-Clang compilers
    @details For reasons unknown to me, Apple-Clang does not fully implement the C++ standard. std::from_chars does not work on non-integral types.
    This is really, really annoying that this is necessary. 
    Must therefore resort to using the (slower, worse) std::stod for Apple people (which includes me!)
*/
#if defined(__clang__) && defined(__APPLE__)
    template <>
    struct Converter<double>
    {
        //! Performs a basic std::stod check, and then checks the validity of the argument. Has its own nested version of Converter<T>::RejectEmpty and Converter<T>::CheckErrors.
        static double internalConvert(std::string_view sv)
        {
            sv = trim(sv,"//");

            if (sv.empty()) 
            {
                LOG(ERROR) << "Cannot convert an empty string to to type double";
                throw std::logic_error("Could not complete conversion");
            } 

            try
            {
                std::string s_temp(sv);
                size_t pos = 0;
                double output = std::stod(s_temp, &pos);

                if (pos != s_temp.length())
                {
                    LOG(ERROR) << "Partial conversion of `" << sv << "` to double; unconverted characters were: `" << s_temp.substr(pos) << "`";
                    throw std::logic_error("Could not complete conversion (trailing characters).");
                }
                return output;
            }
            catch (const std::out_of_range& e)
            {
                LOG(ERROR) << "Error: Result out of range for conversion: '" << sv << "` to double\n";
                throw std::logic_error("Could not complete conversion (value out of range).");
            }
            catch (const std::invalid_argument& e)
            {
                LOG(ERROR) << "Error: Invalid argument for conversion: '" << sv << "` to double\n";
                throw std::logic_error("Could not complete conversion (invalid format).");
            }
        }
       
    };

   
#endif



/*! A partial specialisation of Converter<T> to allow the extraction of vectors.
    @details Functionally, this acts as a wrapper, iteratively calling Converter<T_Inner> on the values extracted
*/
template <typename T_Inner>
struct Converter<std::vector<T_Inner>> 
{
    //! Calls internalConvert(std::string_view, std::string_view,typename) with the default delimiter (a comma)
	static std::vector<T_Inner> internalConvert(std::string_view sv,typename std::enable_if_t<!std::is_same_v<T_Inner, char>>* = nullptr)
    {
        return internalConvert(sv, ",", nullptr); // Pass nullptr for the dummy SFINAE arg
    }

    /*!
        @brief The iterative converter for vector types -- loops over the split-vector and converts the internal types.
        @details The `typename` argument allows the function to disable itself at compile time if T_Inner is a char, thus inducing a Substitution Failure. Through SFINAE principles, this allows std::string (internally a std::vector<char> in many ways) to be ignored by this, and thus picked up by the standard converter
    */
    static std::vector<T_Inner> internalConvert(std::string_view sv, std::string_view element_delimiter,typename std::enable_if_t<!std::is_same_v<T_Inner, char>>* = nullptr) 
    {
        sv = trim(sv,"//");
        if (sv.empty()) 
        {
            LOG(ERROR) << "Empty-vectors can only be instantiated if they have enclosing braces -- empty strings are not valid.";
            throw std::logic_error("Cannot convert an empty string");
        }
        sv = StripEndCaps(sv);

        if (sv.empty()) 
        {
            return std::vector<T_Inner>{};
		} 
        std::vector<T_Inner> result_vec;
        std::vector<std::string_view> elements_sv = split(sv, element_delimiter);
        int i = 0;
        for (const auto& elem_sv : elements_sv)
        {
            if (elem_sv.empty())
            {
                LOG(ERROR) << "Element " << i << " of the vector " << sv << " is empty.\nVector-conversion does not accept empty strings (even if empty strings are allowed for base type";
                throw std::logic_error("Vectors cannot convert empty arguments");
            }
            ++i;
            result_vec.push_back(Converter<T_Inner>::internalConvert(elem_sv));
        }
        return result_vec;
    }

    //! We allow vectors to be wrapped in either [], {} or (). This function removes them for internal use.
    //! @warning We do *not* do any form of parsing or checking to allow nested braces. End caps are purely for user readability.
    static std::string_view StripEndCaps(std::string_view sv)
    {
        size_t start = 0;
        size_t end = sv.size();
        bool leftCap = false;
        bool rightCap = false;
        // Check for common opening brackets
        if (sv[0] == '[' || sv[0] == '{' || sv[0] == '(')
        {
            start = 1;
            leftCap = true;
        }
        // Check for common closing brackets
        if ( (sv[end - 1] == ']' || sv[end - 1] == '}' || sv[end - 1] == ')'))
        {
            end -= 1;
            rightCap = true;
        }

        if (leftCap ^ rightCap || start > end)
        {
            LOG(WARN) << "Unbalanced vector-braces detected in string " << std::string(sv);
        }
        return sv.substr(start,end-start);
    }
    

};


/*! @brief Converts input strings into values of different types
    @details Provides a larger wrapper for the std::from_chars, and extends the functionality to more data types
    @param sv A string_view (implicitly converted from std::strings) representing a value to be converted
    @returns A value of type T corresponding to the input string
*/
template <typename T>
T inline convert(std::string_view sv)
{
    return Converter<T>::internalConvert(sv);
}


/*! A specialised converter for vector data types.
    @details Has a lot of horrible template stuff in order to allow compile-time inference of `is this a vector', without std::strings also being caught. 
    @param sv The string to be converted
    @param delimiter The delimiter of individual data within the string
    @returns A std::vector object corresponding to the input string
*/
template <typename T> struct is_vector_specialization : std::false_type {};
template <typename U, typename Alloc> struct is_vector_specialization<std::vector<U, Alloc>> : std::true_type {};
template <typename T,typename = std::enable_if_t<is_vector_specialization<T>::value>>
T inline convert(std::string_view sv, std::string_view delimiter)
{   
    return Converter<T>::internalConvert(sv, delimiter);
}

//a hack for double-> float implicit conversion. Prevents needing to rewrite
template <>
float inline convert<float>(std::string_view sv)
{
    return Converter<double>::internalConvert(sv);
}



// Helper function to create a tuple from a vector of string_views
// This uses std::index_sequence and a fold expression for compile-time unpacking
template <typename... Ts, std::size_t... Is>
std::tuple<Ts...> ImplicitTupleConverter(const std::vector<std::string_view>& sv_vec,std::index_sequence<Is...>)
{
    return std::make_tuple(convert<Ts>(sv_vec[Is])...);
}

/*! Converts a vector of string(_views) in to a tuple of compile-time specified types
    @param Ts The templated types. The number of templates must be equal to the length of the input vector
    @param sv_vec A vector of strings, each element of which is to be converted
    @returns A tuple<Ts> object corresponding to sv_vec
*/
template <typename... Ts>
std::tuple<Ts...> inline convertTuple(const std::vector<std::string_view>& sv_vec)
{
    constexpr std::size_t expected_size = sizeof...(Ts);
    if (sv_vec.size() != expected_size) 
	{
        LOG(ERROR) << "Tuple conversion error: Token count in vector (" << sv_vec.size()<< ") does not equal tuple size (" << expected_size <<")";
        throw std::logic_error("Tuple conversion: Incorrect token count.");
    }
    

    // Now, call the implementation which will do the actual conversions
    return ImplicitTupleConverter<Ts...>(sv_vec, std::index_sequence_for<Ts...>{});
}