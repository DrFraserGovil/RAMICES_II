#ifndef LOGGER

#define LOGGER
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <regex>
#include "FileHandler.h"
#include "Options.h"

bool internal_LogToFile = false;
bool internal_LogToTerminal = true;
int internal_LoggingLevel = 2;
std::fstream logStream;

class LogStream 
{
	public:
		int Level;
		
		LogStream(int level)
		{
			Level = level;
		}
	
	
	    template<typename T> LogStream& operator<<(const T& mValue)
	    {
			if (Level <=internal_LoggingLevel)
			{
				if (internal_LogToTerminal)
				{
					std::cout << mValue;
				}
				if (internal_LogToFile)
				{
					logStream << mValue;
				}
			}
	    };
};


inline LogStream& log(int level) 
{ 
	
	switch(level)
	{
		case 1: 
			static LogStream l1(1);
			return l1; 
			break;
		case 2:
			static LogStream l2(2); 
			return l2; 
			break;
		case 3:
			static LogStream l3(3);
			return l3;
			break;
		default:
			static LogStream l(1);
			return l;
			break;
	}	
}



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

#endif



