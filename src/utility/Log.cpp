#include "Log.h"

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
	switch(level){
		case 0: 
			Level=ERROR; break;
		case 1:
			Level=WARN;break;
		case 2:
			Level=INFO;break;
		case 3:
			Level=DEBUG;break;
		default:
			throw std::runtime_error(std::to_string(level) + "is not a valid logging level");break;
	}
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
ConfigObject LogConfig;

