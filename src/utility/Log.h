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
#include <sstream>
#include "strings.h"
#include "LogHelpers.h"
#include "ansiCodes.h"


/*!
    The executor of the \ref LOG functionality.
*/
class LoggerCore
{
    public:
        LoggerCore(LogLevel level,int callingLine,const std::string & callingFunction,std::string callingFile);
        ~LoggerCore();
        
        template<class T>
        LoggerCore &operator<<(const T &msg)
        {
            if (!StreamActive)
            {
                StreamActive = true;
                Header();
                Buffer << Insert;
            }
            Buffer << msg;
            return *this;
        } 
    private:
        std::stringstream Buffer;
        LogLevel Level;
        bool StreamActive;
        std::string Insert;
        static int PreviousLogLines;
        void Header();
        
        void endMessage();
};



/*!
    @brief The main log interface. Pipe output to it as you would std::cout.
    
    @details LOG is a specialised macro-interface to the LoggerCore object.  If the level check evaluates to false, then the <<'d inputs are completely ignored and are not executed, useful for skipping `expensive' operations during a ::DEBUG run. Defined in @fileinfo{path}

    @param level A ::LogLevel object, if greater than the LogConfig::Level value, nothing happens (and the expansion is ignored) 
    @returns If the level is suitable, a temporary LoggerCore object, which functions as a specialised Stream object, accepting values passed via '<<'. Otherwise, does nothing, and does not evaluate any subsequent pipe commands
*/
#define LOG(level) \
    if (!(level <= LogConfig.Level)) {} \
    else (LoggerCore(level,__LINE__,__func__,__FILE__))

