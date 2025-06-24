#include "Log.h"
#include "MakeString.h"
namespace  GlobalLog
{
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
			default: throw std::runtime_error("Invalid logger argument");
		} 
		if (Config.TerminalOutput)
		{
			Buffer << fmt;
		}
		if (Config.ShowHeaders)
		{
			Buffer << label;
		}
	}

	void LoggerCore::endMessage()
	{
		if (Config.TerminalOutput)
		{
			Buffer << "\033[0m";
		}
		std::string linebreak = "\n";
		if (Config.ShowHeaders)
		{
		linebreak += "\t";
		}

		auto message = split(Buffer.view(),"\n");
		std::string buffer = "";

		{
			std::unique_lock<std::mutex> lock(GlobalLog::StreamMutex);
			std::cout << message[0];
			for (int i = 1; i < message.size(); ++i)
			{
				std::cout << linebreak << message[i];
			}

			if (Config.AppendNewline)
			{
				std::cout << "\n"; 
			}
			auto nlines = message.size();


			GlobalLog::PreviousLines[Level] =0;
			for (int i = 0; i < LogLevel::MAXLEVEL; ++i)
			{ 
				GlobalLog::PreviousLines[i] += nlines; //do this inside the mutex so line ordering is correct
			}
		}
	}

	//!*not* thread safe on its own
	void LoggerCore::Erase(int nLines)
	{
		if (GlobalLog::Config.TerminalOutput)
		{	
			std::unique_lock<std::mutex> lock(GlobalLog::StreamMutex);
			for (int i = 0; i < nLines; ++i)
			{
				std::cout << ANSI::CURSOR_UP << ANSI::CURSOR_TO_COL1 << ANSI::CLEAR_LINE;
			}
			std::cout << std::flush;//only do because deletion is expected to be 'instant', not buffered
			for (int i = 0; i < LogLevel::MAXLEVEL;++i)
			{
				int n = GlobalLog::PreviousLines[i];
				GlobalLog::PreviousLines[i] = std::max(0,n-nLines);
			}
		}
	}

	//!Thread safe!
	void LoggerCore::ErasePrevious()
	{
		{
			std::unique_lock<std::mutex> lock(GlobalLog::StreamMutex);
			size_t erase = GlobalLog::PreviousLines[Level];
			size_t block = 0;
			for (int i = 0; i < Level; ++i)
			{
				if (PreviousLines[i] < erase && (block == 0 || PreviousLines[i] < block))
				{
					block = PreviousLines[i];
				}
			}
			if (block > 0)
			{
				size_t safe = 0;
				for (int i = Level +1; i < LogLevel::MAXLEVEL; ++i)
				{
					if (PreviousLines[i] > safe && PreviousLines[i] < block)
					{
						safe = PreviousLines[i];
					}
				}
				erase = safe;
			}

			Erase(erase);
		}
	}
	
} // namespace  GlobalLog