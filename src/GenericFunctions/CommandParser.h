#ifndef COMMANDPARSER
#define COMMANDPARSER


#include <vector>
#include <string>
#include <iomanip>
#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h> 
#include "../Options.h"
#include "FileHandler.h"
#include "StringAlgorithms.h"
//See .cpp for initialisation values
typedef bool (*parseFunctions) (char* arg);


//designated "special functions" (see .cpp for more detail)
bool help(char* arg);
bool changeFileRoot(char* arg);


//Caller
struct ParserOutput
{
	Options options;
	bool SuccessfulParse;
	std::vector<std::string> PreLog;
};

ParserOutput parseCommandLine(int argc, char** argv);



#endif
