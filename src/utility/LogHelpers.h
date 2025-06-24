#pragma once
#include <exception>
#include <mutex>
#include <string>
/*!
    An encoding for different levels of logs. Levels are hierarchical: WARN includes ERROR, and INFO includes WARN (and therefore, also ERROR).
	
	Levels are assumed to be numerically increasing. A lower level means more important.
*/
enum LogLevel 
{
    ERROR, //!< Level 0. Used to indicate points where the code is throwing errors. 
    WARN,  //!< Level 1. Used to indicate where problems were encountered, but a default assumption was made. Also used to indicate `are you sure about this?'
    INFO,  //!< Level 2. General progress information.
    DEBUG, //!< Level 3. High density of information, likely to bottleneck code. Used for debugging information
};

//! Convert integers to LogLevels. @param level an integer between 0 and 3 @throws runtime_error if level is out of bounds @returns The corresponding LogLevel 
inline LogLevel LogLevelConvert(int level)
{
	switch(level){
		case 0: 
			return ERROR; break;
		case 1:
			return WARN;break;
		case 2:
			return INFO;break;
		case 3:
			return DEBUG;break;
		default:
			throw std::runtime_error(std::to_string(level) + "is not a valid logging level");break;
	}
}

/*!
	\brief A packager for globally accessible variables for the LoggerCore object to refer to. 

	\details A single \c ConfigObject exists, the globally defined \ref LogConfig. Defined in @fileinfo{path}
*/
struct ConfigObject
{
    bool AppendNewline;//!<If true, all Log commands @default 
    bool ShowHeaders;
    LogLevel Level;
    bool TerminalOutput;
	ConfigObject() : ConfigObject(INFO,true,true){};
    ConfigObject(int level,bool header,bool newline);
    ConfigObject(LogLevel level ,bool header,bool newline);

    void SetLevel(LogLevel level);
    void SetLevel(int level);
    void SetHeader(bool value);
    void SetNewline(bool value);
    void Initialise(int level,bool header,std::string welcomeFile);
};


extern ConfigObject LogConfig;
extern std::mutex GlobalLogMutex;