#include <string_view>
#include <vector>
template<typename T>
struct Converter
{
	static T convert(std::string_view sv)
	{
		T output;// only works if there's a default constructor
		if (sv.empty()) {
			LOG(ERROR) << "Cannot convert an empty string to to type " << typeid(T).name();
			throw std::logic_error("Could not complete conversion");
		} 

		auto result = std::from_chars(sv.data(), sv.data() + sv.size(),output);

		if (result.ec == std::errc() &&  (result.ptr != sv.data() + sv.size()))
		{ 
			LOG(ERROR) << "Partial conversion of " << sv << " to type " << typeid(T).name() << "unconverted characters were: " << std::string_view(result.ptr, sv.data() + sv.size() - result.ptr);
			throw std::logic_error("Could not complete conversion");
		}
		else if (result.ec == std::errc::invalid_argument) {
			LOG(ERROR) <<  "Error: Invalid argument for conversion: '" << sv   << " to type " << typeid(T).name()<< "'\n";
			throw std::logic_error("Could not complete conversion");
		} else if (result.ec == std::errc::result_out_of_range) {
			LOG(ERROR) <<  "Error: Result out of range for conversion: '" << sv << " to type " << typeid(T).name() << "'\n";
			throw std::logic_error("Could not complete conversion");
		}

				
				
		return output;
	}
};

// Full specialization for std::string
template <>
struct Converter<std::string> {
    static std::string convert(std::string_view sv) 
	{
        return std::string(sv);
    }
};

template <typename T_Inner>
struct Converter<std::vector<T_Inner>> {
    // Use SFINAE to disable this specialization if T_Inner is 'char'
    // to prevent ambiguity with std::string
     // Overload 1: Takes only string_view, uses default delimiter
	 static std::vector<T_Inner> convert(std::string_view sv,
        typename std::enable_if_t<!std::is_same_v<T_Inner, char>>* = nullptr) {
        // Calls the other overload with a default delimiter
        return convert(sv, ",", nullptr); // Pass nullptr for the dummy SFINAE arg
    }

    // Overload 2: Takes string_view and a custom delimiter
    static std::vector<T_Inner> convert(std::string_view sv, std::string_view element_delimiter,
        typename std::enable_if_t<!std::is_same_v<T_Inner, char>>* = nullptr) {
        std::vector<T_Inner> result_vec;
        std::vector<std::string_view> elements_sv = split(sv, element_delimiter);
        for (const auto& elem_sv : elements_sv) {
            result_vec.push_back(Converter<T_Inner>::convert(elem_sv));
        }
        return result_vec;
    }
};


template <typename T>
T inline convert(std::string_view sv) {
    return Converter<T>::convert(sv);
}


template <typename T> struct is_vector_specialization : std::false_type {};
template <typename U, typename Alloc> struct is_vector_specialization<std::vector<U, Alloc>> : std::true_type {};

template <typename T,
          typename = std::enable_if_t<is_vector_specialization<T>::value>>
T inline convert(std::string_view sv, std::string_view element_delimiter) {
    // This will correctly dispatch to the two-argument convert in Converter<std::vector<T::value_type>>
    // T::value_type is a member type defined by std::vector
    return Converter<T>::convert(sv, element_delimiter);
}