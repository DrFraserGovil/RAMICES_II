#ifndef FILEHANDLER
#define FILEHANDLER

#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <dirent.h>
#include "StringAlgorithms.h"

#define forLineInFile(macroFileName, ...)\
{								\
	do 							\
	{							\
		std::ifstream macroFile(macroFileName);	\
		if (!macroFile.is_open())	\
		{							\
			std::cout << "\n\nERROR: Could not find the file '" << macroFileName << ".\n\nPlease provide a valid directory within the Output directory\n\n " << std::endl;	\
			exit(1);				\
		}							\
		std::string FILE_LINE;				\
		while (getline(macroFile,FILE_LINE))	\
		{							\
			__VA_ARGS__				\
		}							\
		macroFile.close();			\
	} while(0);						\
}									\

#define forLineVectorInFile(macroFileName, token,...)\
{								\
	do 							\
	{							\
		std::ifstream macroFile(macroFileName);	\
		if (!macroFile.is_open())	\
		{							\
			std::cout << "\n\nERROR: Could not find the file '" << macroFileName << ".\n\nPlease provide a valid directory within the Output directory\n\n " << std::endl;	\
			exit(1);				\
		}							\
		std::string FILE_LINE;				\
		while (getline(macroFile,FILE_LINE))	\
		{							\
			std::vector<std::string> FILE_LINE_VECTOR = split(FILE_LINE,token);	\
			__VA_ARGS__;				\
		}							\
		macroFile.close();			\
	} while(0);						\
}		

struct mkdirReturn
{
	bool Successful;
	std::string Message;
};

mkdirReturn mkdirSafely(std::string dirLoc);
#endif


