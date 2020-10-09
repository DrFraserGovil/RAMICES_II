#ifndef LOGGER

#define LOGGER
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <regex>
#include "FileHandler.h"
#include "Options.h"

extern bool internal_LogToFile;
extern bool internal_LogToTerminal;
extern int internal_LoggingLevel;
extern std::fstream logStream;

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



void asciiArt();

void initialiseLogger(Options* opts,std::vector<std::string> prelog);

void shutDownLogger();
#endif



