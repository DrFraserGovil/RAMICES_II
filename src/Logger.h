#pragma once
#include <string>
#include <fstream>
#include <iostream>

#include "Options.h"

int internal_LogToFile = false;
int internal_LogToTerminal = true;
int internal_LoggingLevel = 1;
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
	    }
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



void initialiseLogger(Options* opts)
{
	internal_LoggingLevel = opts->Simulation.LoggingLevel;
	internal_LogToTerminal = opts->Simulation.LogToTerminal;
	if (opts->Simulation.LogToFile)
	{
		internal_LogToFile = true;
		std::string fileName = opts->Simulation.FileRoot + "Output.log";
		logStream.open(fileName,std::fstream::out );
	}
	std::string strip = "=========================================================";
	log(1) << strip + "\n\n\tRAMICES II SIMULATION INITIALISED.\n\n" + strip + "\n  Options Parsed and Logger Initialised\n";
}

void shutDownLogger()
{	
	log(1) << "\nShutting down logging capabilities. \nSimulation Terminated. Have a nice day :) \n";
	
	logStream.close();
}
