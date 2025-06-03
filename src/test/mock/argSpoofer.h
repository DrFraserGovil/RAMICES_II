#pragma once
#include <vector>
#include <string>
//generates a suitable argc/argv pair from an input vector<string>
//surprisingly difficult to generate -- need to ensure the object the pointers refer to remain in scope
struct ArgSpoofer
{
	int argc;
	char** argv;
	std::vector<char*> internalVector;
	std::vector<std::string> copyVector;
	ArgSpoofer(std::vector<std::string> input)
	{
		Digest(input);
	}

	void Digest(std::vector<std::string> input)
	{
		copyVector = input;
		for (const std::string& s : copyVector) {
			internalVector.push_back(const_cast<char*>(s.c_str()));
		}
		argc = internalVector.size();
		argv = internalVector.data(); 
	}
};
