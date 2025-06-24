#include "Log.h"


LoggerCore::LoggerCore(LogLevel level,int callingLine,const std::string & callingFunction,std::string callingFile)
{

	StreamActive = false;
	Level = level;
	Insert = "";
	if (Level <= 1)
	{
		Insert = "Line " + std::to_string(callingLine) + " of " + callingFile + " in function " + callingFunction;
		Insert += "\n";
	}
}

LoggerCore::~LoggerCore()
{
	if (StreamActive)
	{
		endMessage();
	}
}

void LoggerCore::Header()
{
	std::string label;
	std::string fmt;
	switch(Level) {
		case DEBUG: fmt = ANSI::BLUE_FONT;label = "[DEBUG] "; break;
		case INFO: fmt=ANSI::WHITE_FONT;label = "[INFO]  "; break;
		case WARN: fmt=ANSI::PURPLE_FONT;label = "[WARN]  "; break;
		case ERROR: fmt=ANSI::RED_FONT;label = "[ERROR] "; break;
	} 
	if (LogConfig.TerminalOutput)
	{
		Buffer << fmt;
	}
	if (LogConfig.ShowHeaders)
	{
		Buffer << label;
	}
}

void LoggerCore::endMessage()
{
	if (LogConfig.TerminalOutput)
	{
		Buffer << "\033[0m";
	}
	std::string linebreak = "\n";
	if (LogConfig.ShowHeaders)
	{
	   linebreak += "\t";
	}

	auto message = split(Buffer.view(),"\n");
	std::string buffer = "";

	{
		std::unique_lock<std::mutex> lock(GlobalLogMutex);
		std::cout << message[0];
		for (int i = 1; i < message.size(); ++i)
		{
			std::cout << linebreak << message[i];
		}

		if (LogConfig.AppendNewline)
		{
			std::cout << "\n"; 
		}
	}


}