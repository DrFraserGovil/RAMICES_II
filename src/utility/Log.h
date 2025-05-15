/* 
 * File:   Log.h
 * Author: Alberto Lepe <dev@alepe.com>
 *
 * Created on December 1, 2015, 6:00 PM
 */
// Modifications added by JFG

#pragma once
#include <unistd.h> // For isatty()
#include <cstdio>   // For fileno() and stderr
#include <iostream>
#include <exception>


enum LogLevel 
{
    ERROR,  //Uh uh, stuff broke
    WARN,  //Problems arose, but were handled
    INFO,  //Nice to know
    DEBUG, //Ludicrously indepth stuff.
};


struct ConfigObject
{
    bool ShowHeaders;
    LogLevel Level;
    bool TerminalOutput;
    bool AppendNewline;
    ConfigObject(int level = 2,bool header = true,bool newline = true);
    ConfigObject(LogLevel level ,bool header = true,bool newline=true);

    void SetLevel(LogLevel level);
    void SetLevel(int level);
    void SetHeader(bool value);
    void SetNewline(bool value);
};
extern ConfigObject LogConfig;
class LoggerCore
{
    public:
        LoggerCore(LogLevel level,int line,std::string function,std::string file)
        {

            StreamActive = false;
            Level = level;
            Insert = "";
            if (Level <= 1)
            {
                Insert = "Line " + std::to_string(line) + " of " + file + " in function " + function;
                Insert += "\n       \t";
            }
        };
        ~LoggerCore()
        {
            if (StreamActive)
            {
                endMessage();
            }
        }
        template<class T>
        LoggerCore &operator<<(const T &msg)
        {
            if (!StreamActive)
            {
                StreamActive = true;
                Header();
                std::cout << Insert;
            }
            std::cout << msg;

            return *this;
        } 
    private:
        LogLevel Level;
        bool StreamActive;
        std::string Insert;
        void Header()
        {
            std::string label;
            std::string fmt;
            switch(Level) {
                case DEBUG: fmt = "\033[34m";label = "[DEBUG] "; break;
                case INFO: fmt="\033[37m";label = "[INFO]  "; break;
                case WARN: fmt="\033[33m";label = "[WARN]  "; break;
                case ERROR: fmt="\033[31m";label = "[ERROR] "; break;
            } 
            if (LogConfig.TerminalOutput)
            {
                std::cout << fmt;
            }
            if (LogConfig.ShowHeaders)
            {
                std::cout << label;
            }
        }
        void endMessage()
        {
            if (LogConfig.TerminalOutput)
            {
                std::cout << "\033[0m";
            }
            if (LogConfig.AppendNewline)
            {
                std::cout << "\n"; 
            }
        }
};



/*
    The logger is wrapped in a macro interface to allow a non-trivial optimisation.
    If the level check evaluates to false, then the <<'d inputs are completely ignored and are not executed.
    That means that potentially expensive function calls can be omitted -- or prevents excessive string formatting inside tight loops that might otherwise be useful during a [DEBUG] call.
*/
#define LOG(level) \
    if (!(level <= LogConfig.Level)) {} \
    else (LoggerCore(level,__LINE__,__func__,__FILE__))
