#pragma once

#include "Log.h"
#include "StringSplit.h"
#include "stringViewConverters.h"
template <typename Func>
void forLineIn(const std::string& fileName, Func lineProcessor) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
		LOG(ERROR) << "Could not find the file '" + fileName + "'.\n\nPlease provide a valid filepath.";
		throw std::runtime_error("Could not open file");
    }

    std::string fileLine;
    while (std::getline(file, fileLine)) {
        lineProcessor(fileLine);
    }
    file.close();
}

template <typename Func>
void forSplitLineIn(const std::string& fileName, std::string delimiter, Func vectorProcessor) 
{
	forLineIn(fileName,
		[&](std::string & line)
		{
			vectorProcessor(split(line,delimiter));
		}
	);
}


// Helper function to create a tuple from a vector of string_views
// This uses std::index_sequence and a fold expression for compile-time unpacking
template <typename... Ts, std::size_t... Is>
std::tuple<Ts...> ImplicitTupleConverter(const std::vector<std::string_view>& sv_vec,std::index_sequence<Is...>)
{
    return std::make_tuple(convert<Ts>(sv_vec[Is])...);
}

// Main helper to handle size checks and dispatch to the impl function
template <typename... Ts>
std::tuple<Ts...> inline tupleFromStringViews(const std::vector<std::string_view>& sv_vec)
{
    constexpr std::size_t expected_size = sizeof...(Ts);

    if (sv_vec.size() < expected_size) 
	{
        LOG(ERROR) << "Tuple conversion error: Not enough tokens (" << sv_vec.size()<< ") to fill tuple of size " << expected_size;
        throw std::logic_error("Tuple conversion: Not enough tokens in line.");
    }
    if (sv_vec.size() > expected_size)
	{
        LOG(WARN) << "Tuple conversion warning: Too many tokens (" << sv_vec.size() << ") for tuple of size " << expected_size<< ". Extra tokens will be ignored.";
        // You could also throw an error here if strict adherence to column count is required.
    }

    // Now, call the implementation which will do the actual conversions
    return ImplicitTupleConverter<Ts...>(sv_vec, std::index_sequence_for<Ts...>{});
}

template <typename... Ts, typename Func>
void forLineTupleIn(const std::string& fileName, std::string_view delimiter, Func tupleProcessor)
{
    forLineIn(fileName,
        // Important: `line` is taken by value here (`std::string line`).
        // This ensures the `std::string` object (which split's string_views refer to)
        // lives for the entire duration that `sv_vec` and the converted `parsed_tuple` are used,
        // including inside the `tupleProcessor` lambda.
        [&, delimiter](std::string & line)
        {
            std::vector<std::string_view> sv_vec = split(line, delimiter);
            // Convert the vector of string_views into the desired tuple
            std::tuple<Ts...> parsed_tuple = tupleFromStringViews<Ts...>(sv_vec);
            // Pass the fully typed tuple to the user's lambda
            tupleProcessor(parsed_tuple);
        }
    );
}