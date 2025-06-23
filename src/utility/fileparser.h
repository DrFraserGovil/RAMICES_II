#pragma once
#include <fstream>
#include "Log.h"
#include "strings.h"
#include "convert.h"
template <typename Func>
void forLineIn(const std::string& fileName, Func lineProcessor) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
		LOG(ERROR) << "Could not find the file '" + fileName + "'.\nPlease provide a valid filepath.";
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
            std::tuple<Ts...> parsed_tuple = convertTuple<Ts...>(sv_vec);
            // Pass the fully typed tuple to the user's lambda
            tupleProcessor(parsed_tuple);
        }
    );
}