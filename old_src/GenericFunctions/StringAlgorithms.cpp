#include "StringAlgorithms.h"
std::vector<std::string> split(const std::string& s, char delimiter)
{
	//dumb brute-force string splitter based on a delimiter
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (getline(tokenStream, token, delimiter))
	{
		if( token.length() > 0) // empty rows not allowed
		{
			tokens.push_back(token);
		}
	}
	return tokens;
}
