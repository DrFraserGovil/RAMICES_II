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
ConfigObject LogConfig;

