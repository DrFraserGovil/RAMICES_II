#include "Logger.h"
bool internal_LogToFile = false;
bool internal_LogToTerminal = true;
int internal_LoggingLevel = 2;
std::fstream logStream;


void asciiArt()
{
	forLineInFile("Resources/ascii.dat",
	
		log(1) << FILE_LINE + "\n";
	);
}



void initialiseLogger(Options* opts,std::vector<std::string> prelog)
{
	internal_LoggingLevel = opts->Simulation.LoggingLevel;
	internal_LogToTerminal = opts->Simulation.LogToTerminal;
	if (opts->Simulation.LogToFile)
	{
		internal_LogToFile = true;
		std::string fileName = opts->Simulation.FileRoot + "Output.log";
		logStream.open(fileName,std::fstream::out );
	}
	
	asciiArt();
	
	log(1) << "\nRAMICES II Simulation Initialised.\n\nLogger Initialised\n";
	
	for (std::string entry : prelog)
	{
		log(1) << entry + "\n";
	}
	
	
}

void shutDownLogger()
{	
	log(1) << "\nShutting down logging capabilities. \nSimulation Terminated. Have a nice day :) \n";
	
	logStream.close();
}
