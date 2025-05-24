#include <string_view>
#include <vector>
#include <regex>
#include "strings.h"
/*
    This file provides a robust return-value interface for converting string-views (and, implicitly, strings) into a candidate type
    This type can be any value accepted by std::from_chars, or those which can be implicitly converted to from that type

    Converter also handles conversion of multiple values at once (of the same type) into a vector.

    Special overloads are provided that allow direct stringview->string conversion.
*/


//!The backend structure -- written as a static functor in order to allow partial specialisation (and therefore the vector loop)
template<typename T>
struct Converter
{
	static T convert(std::string_view sv)
	{
        sv = trim(sv);
        RejectEmpty(sv);
        
        //create an object and read from_chars into it. Some implicit type conversion is allowed here (i.e. if T is a bool)
        T output;
		auto result = std::from_chars(sv.data(), sv.data() + sv.size(),output);

        CheckErrors(result,sv);		
				
		return output;
	}

    //Throws if the from_chars reached some undesirable state
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

    static void RejectEmpty(std::string_view sv)
    {
        if (sv.empty()) 
        {
            LOG(ERROR) << "Cannot convert an empty string to to type " << typeid(T).name();
			throw std::logic_error("Could not complete conversion");
		} 
    }
};

// Full specialization for std::string (because from_chars can't do strings!)
//Could just rely on user to call this manually, but it's nicer to have a unified interface
template <>
struct Converter<std::string> {
    static std::string convert(std::string_view sv) 
	{

        return std::string(sv);
    }
};

// Full specialization for boolean (because from_chars can't do boolean)
template <>
struct Converter<bool> {
    static bool convert(std::string_view sv) 
	{
        auto snap = trim(sv);
        if (snap == "1" || insensitiveEquals(snap,"true"))
        {
            return true;
        }
        if (snap == "0" || insensitiveEquals(snap,"false"))
        {
            return false;
        }

        LOG(ERROR) << "Cannot convert string " << sv << "to boolean";
        throw std::logic_error("Cannot convert string to boolean");
    }
};

// Specialization for double (using std::stod as required by Apple Clang limitation)
//This is really, really annoying that this is necessary. Apple-clang does not currently support from_chars for non-integral types
//Have to use the slow version for Apple people (which includes me!)

#if defined(__clang__) && defined(__APPLE__)
    template <>
    struct Converter<double>
    {
        static double convert(std::string_view sv)
        {
            sv = trim(sv);
            RejectEmpty(sv);

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
        static void RejectEmpty(std::string_view sv)
        {
            if (sv.empty()) 
            {
                LOG(ERROR) << "Cannot convert an empty string to to type double";
                throw std::logic_error("Could not complete conversion");
            } 
        }
    };

   
#endif




template <typename T_Inner>
struct Converter<std::vector<T_Inner>> 
{
    // Use SFINAE to disable this specialization if T_Inner is 'char'
    // to prevent ambiguity with std::string (which can be confused with std::vector<char>)
    // Overload 1: Takes only string_view, uses default delimiter
	static std::vector<T_Inner> convert(std::string_view sv,typename std::enable_if_t<!std::is_same_v<T_Inner, char>>* = nullptr)
    {
        // Calls the other overload with a default delimiter
        return convert(sv, ",", nullptr); // Pass nullptr for the dummy SFINAE arg
    }

    // Overload 2: Takes string_view and a custom delimiter
    static std::vector<T_Inner> convert(std::string_view sv, std::string_view element_delimiter,typename std::enable_if_t<!std::is_same_v<T_Inner, char>>* = nullptr) 
    {
        sv = trim(sv);
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
                LOG(ERROR) << "Element " << i << " of the vector " << sv << " is empty.\n\tVector-conversion does not accept empty strings (even if empty strings are allowed for base type";
                throw std::logic_error("Vectors cannot convert empty arguments");
            }
            ++i;
            result_vec.push_back(Converter<T_Inner>::convert(elem_sv));
        }
        return result_vec;
    }
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


//wrapper function so that the functor never has to be exposed
template <typename T>
T inline convert(std::string_view sv)
{
    return Converter<T>::convert(sv);
}


// A wrapper function to allow vector-types to be called with the delimiter
//helper templates
template <typename T> struct is_vector_specialization : std::false_type {};
template <typename U, typename Alloc> struct is_vector_specialization<std::vector<U, Alloc>> : std::true_type {};
template <typename T,typename = std::enable_if_t<is_vector_specialization<T>::value>>
T inline convert(std::string_view sv, std::string_view delimiter)
{   
    return Converter<T>::convert(sv, delimiter);
}

//a hack for double-> float implicit conversion. Prevents needing to rewrite
template <>
float inline convert<float>(std::string_view sv)
{
    return Converter<double>::convert(sv);
}