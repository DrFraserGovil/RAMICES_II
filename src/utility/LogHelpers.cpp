#include "LogHelpers.h"

#include "fileparser.h"

bool isTerminal() {
    return isatty(fileno(stdout));
}

namespace GlobalLog
{
	
	ConfigObject::ConfigObject()
	{
		SetLevel(INFO);
		ShowHeaders = true;
		AppendNewline = true;
		TerminalOutput = isTerminal();
	}

	void ConfigObject::SetLevel(int level)
	{
		Level = LogLevelConvert(level);
	}
	

	void ConfigObject::Initialise(int level, bool header,std::string welcomeFile)
	{
		AppendNewline =true;
		SetLevel(level);
		
		
		ShowHeaders = false;//set header to false temporarily
		//force in some special colours!
		auto fmt = "\033[38;5;228m";
		if (!TerminalOutput)
		{
			fmt = "";
		}

		try
		{
			forLineIn(welcomeFile,[&](auto line)
			{
				LOG(INFO) << fmt << line;	
			});

			
			
		}
		catch(...)
		{
			LOG(ERROR) << "Failed to locate the welcome file. This is usually an indicator of a malformed ResourceDirectory. Please ensure System::ResourceDirectory points to a valid file location";
			throw std::runtime_error("Invalid resource directory");
		}
		ShowHeaders = true;
		LOG(DEBUG) << "Logging system initialised";
		
	}


	//See the LogHelpers.h file for the definitions of these global variables
	ConfigObject Config; 
	std::vector<size_t> PreviousLines = std::vector<size_t>(LogLevel::MAXLEVEL,0);
	std::mutex StreamMutex;
}