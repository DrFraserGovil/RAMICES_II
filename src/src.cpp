#include <stdio.h>
#include <iostream>


#include "Options.h"
#include "GenericFunctions/Logger.h"
#include "GenericFunctions/CommandParser.h";
#include "ChemicalAssemblers/YieldGrid.h"
#include "Galaxy/GasReservoir.h"
#include "Galaxy/Galaxy.h"


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
	
	//initialise main galaxy object
	Galaxy g = Galaxy(&options);

	g.Evolve();
	
	
	
	shutDownLogger();
	return 0;
}
