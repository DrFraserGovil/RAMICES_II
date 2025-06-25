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
			Insert = "Line " + std::to_string(callingLine) + " of " + callingFile + " in function " + callingFunction ;
			Insert += "\n";
		}
	}

	LoggerCore::~LoggerCore()
	{
		if (StreamActive) //only add the output to stream if "<<" was actually called
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
			Buffer << ANSI::RESET_FORMAT; //reset the font colors for all subsequent data
		}


		//now format the data so that linebreaks are suitably indented
		std::string linebreak = "\n";
		if (Config.ShowHeaders)
		{
			linebreak += "\t";
		}
		auto message = split(Buffer.view(),"\n");
		std::string buffer = "";

		//iterate across all lines in the entry 
		{
			std::unique_lock<std::mutex> lock(GlobalLog::StreamMutex); //lock the stream to prevent interleaving

			//the first line automatically includes the correct indentation -- the header accounts for that
			std::cout << message[0]; 

			//subequent lines need to indent (or not) based on the presence of the header.
			for (int i = 1; i < message.size(); ++i)
			{
				std::cout << linebreak << message[i];
			}

			if (Config.AppendNewline)
			{
				std::cout << "\n"; 
			}

			//save the data to the 'erase' memory banks
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
				int pli = PreviousLines[i];
				if (pli < erase && pli > 0 && (block == 0 || pli < block))
				{
					block = PreviousLines[i];
				}
			}
			size_t safe = 0;
			if (block > 0)
			{
				for (int i = Level +1; i < LogLevel::MAXLEVEL; ++i)
				{
					int pli = PreviousLines[i];
					if (pli > safe && pli < block && pli > 0 )
					{
						safe = pli;
					}
				}
				erase = safe;
			}

			Erase(erase);
		}
	}
	
} // namespace  GlobalLog