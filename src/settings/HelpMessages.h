#pragma once
#include <vector>
#include <tuple>
#include <string>
#include <iostream>
#include "../utility/MakeString.h"
#include "../utility/strings.h"
struct HelpMessages
{
	std::string Name;
	std::vector<std::tuple<std::string,std::string,std::string,std::string>> Messages;

	template<class T>
	void AddMessage(std::string flag,T defaultValue,std::string name,std::string description)
	{
		Messages.push_back(std::make_tuple(flag,name,description,MakeString(defaultValue)));
		// Messages.push_back(std::make_tuple<std::string,std::string,std::string,std::string>(flag,"","",MakeString(defaultValue)));	
	}

	void scanSizes(std::pair<int,int> & lengths)
	{
		for (auto message : Messages)
		 {
			auto keyLength = std::get<0>(message).size();
			auto nameLength = std::get<1>(message).size();

			if (keyLength > lengths.first)
			{
				lengths.first = keyLength;
			}
			if (nameLength > lengths.second)
			{
				lengths.second = nameLength;
			}
		 }
	}

	void print(std::pair<int,int> & lengths)
	{
		int maxKeyLength = lengths.first;
		int maxNameLength = lengths.second;
		 

		std::cout << Name << ":\n\n";

		sort(Messages.begin(), Messages.end(),
         [](const std::tuple<std::string,std::string,std::string,std::string>& a,
            const std::tuple<std::string,std::string,std::string,std::string>& b) {
             return std::get<0>(a) < std::get<0>(b);
         });

		 
		 int bufferSpace = 4;
		 int minSize = 10;
		 int nameStartPos = std::max(minSize,maxKeyLength + bufferSpace);
		 int descStartPos =  std::max(minSize,maxNameLength + bufferSpace);
		 auto newLineBuffer =  std::string(3+nameStartPos+descStartPos,' ');
		 for (auto message : Messages)
		 {
			auto key = std::get<0>(message);
			auto name = std::get<1>(message);
			auto keyNameGap =  std::string(nameStartPos-key.size(),' '); 
			auto nameDescGap = std::string(descStartPos-name.size(),' ');
			std::cout << "  -" << key;
			std::cout << keyNameGap << name;

			auto descLines = split(std::get<2>(message),"\n");
			std::cout << nameDescGap  << descLines[0] << "\n";
			for (int i = 1; i < descLines.size(); ++i)
			{
				std::cout << newLineBuffer << descLines[i] << "\n";
			}

			std::cout << newLineBuffer << "default(" << std::get<3>(message) << ")\n"; 
			// std::cout <<  std::get<1>(message) << " " << std::get<2>(message) << " " << std::get<3>(message) << "\n";
		 }
	}
};