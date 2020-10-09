#include <stdio.h>
#include <iostream>

#include "Logger.h"
#include "Options.h"
#include "CommandParser.h";

int main(int argc, char** argv)
{
	Options options;
	
	ParserOutput parser = parseCommandLine(argc, argv);
	
	
	
	
	options = parser.options;
	
	initialiseLogger(&options,parser.PreLog);
	
	if (parser.SuccessfulParse == false)
	{
		return 1;
	}
	
	shutDownLogger();
	return 0;
}
