#include "Log.h"
#include "fileparser.h"
std::mutex GlobalLogMutex;
bool isTerminal() {
    return isatty(fileno(stdout));
}
ConfigObject::ConfigObject(int level,bool header,bool newline)
{
	SetLevel(level);
	SetHeader(header);
	SetNewline(newline);
	TerminalOutput = isTerminal();
}
ConfigObject::ConfigObject(LogLevel level,bool header,bool newline)
{
	SetLevel(level);
	SetHeader(header);
	SetNewline(newline);
	TerminalOutput = isTerminal();
}




void ConfigObject::SetLevel(int level)
{
	Level = LogLevelConvert(level);
}
void ConfigObject::SetLevel(LogLevel level)
{
	Level = level;
}
void ConfigObject::SetHeader(bool val)
{
	ShowHeaders = val;
}
void ConfigObject::SetNewline(bool val)
{
	AppendNewline = val;
}

void ConfigObject::Initialise(int level, bool header,std::string welcomeFile)
{
	SetNewline(true);
	SetLevel(level);
	
	
	SetHeader(false);//set header to false temporarily
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
	SetHeader(header);
}

ConfigObject LogConfig;

