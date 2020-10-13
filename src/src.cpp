#include <stdio.h>
#include <iostream>


#include "Options.h"
#include "GenericFunctions/Logger.h"
#include "GenericFunctions/CommandParser.h";
#include "ChemicalAssemblers/YieldGrid.h"
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
	
	
	YieldGrid demosthenes = YieldGrid(&options);

	shutDownLogger();
	return 0;
}
