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
    ERROR=0, //!< Level 0. Used to indicate points where the code is throwing errors. 
    WARN=1,  //!< Level 1. Used to indicate where problems were encountered, but a default assumption was made. Also used to indicate `are you sure about this?'
    INFO=2,  //!< Level 2. General progress information.
    DEBUG=3, //!< Level 3. High density of information, likely to bottleneck code. Used for debugging information

	MAXLEVEL //!<Used as an indicator of the 'allowed max log level' - used only for loop checks etc. Should never be assigned to
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



namespace GlobalLog
{
	/*!
	\brief A packager for globally accessible variables for the LoggerCore object to refer to. 

	\details A single \c ConfigObject exists, the globally defined \ref GlobalLog::Config. Defined in @fileinfo{path}
	*/
	struct ConfigObject
	{
		//! If true, all Log commands have a newline automatically appended to them. Default: TRUE
		bool AppendNewline;
		
		//! If true, all Log commands are preceeded by 'header info' about the type of log. Headers occur only once per LOG call, linebreaks are automatically aligned. Default: TRUE
		bool ShowHeaders; 
		
		//! Any calls to @ref LOG with a value higher than this are ignored. Default: INFO
		LogLevel Level; 

		//! Automatically detected at runtime-start. True if the output stream is a tty terminal capable of interpreting @ref ANSI commands.
		bool TerminalOutput; 

		//! Default initialiser. Initialises TerminalOutput, and sets Level=INFO, ShowHeaders=true and AppendNewline=true.
		ConfigObject();
		
		//! Convenient interface for setting Level from integer values
		void SetLevel(int level);

		/*!
			Used to initialise globally \ref Config, after the command line arguments have been parsed.
			@param level The (as an integer) to be assigned to the Level
			@param header The value to be assigned to ShowHeaders
			@param welcomeFile The file location of the RAMICES 'welcome.dat' file, which prints a cute little message
		*/ 
		void Initialise(int level,bool header,std::string welcomeFile);
		
	};


	//! The global ConfigObject used by @ref LOG and LoggerCore to determine what messages are printed, and any associated formatting 
	extern ConfigObject Config;

	//! Used to prevent log-interleaving, and make the LOG (mostly) thread-safe
	extern std::mutex StreamMutex;

	//! A tracker of the number of lines printed since each type of ::LogLevel. Used for LoggerCore::ErasePrevious()
	extern std::vector<size_t> PreviousLines;
}