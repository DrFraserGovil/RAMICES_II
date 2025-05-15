/* 
 * File:   Log.h
 * Author: Alberto Lepe <dev@alepe.com>
 *
 * Created on December 1, 2015, 6:00 PM
 */
// Modifications added by JFG

#pragma once

#include <iostream>
#include <exception>


enum typelog {
    ERROR,  //Uh uh, stuff broke
    WARN,  //Problems arose, but were handled
    INFO,  //Nice to know
    DEBUG, //Ludicrously indepth stuff.
};

struct structlog {
    bool headers = false;
    typelog level = WARN;

    void SetLevel(int i);
};

extern structlog LOGCFG;

class LOG {
public:
    LOG() {}
    LOG(typelog type) {
        msglevel = type;
        if(LOGCFG.headers) {
            operator << (getLabel(type));
        }
    }
    ~LOG() {
        if(opened) {
            std::clog << "\033[0m\n";
        }
        opened = false;
    }
    template<class T>
    LOG &operator<<(const T &msg) {
        if(msglevel <= LOGCFG.level) {
           std::clog << msg;
            opened = true;
        }
        return *this;
    }
private:
    bool opened = false;
    typelog msglevel = DEBUG;
    inline std::string getLabel(typelog type) {
        std::string label;
        switch(type) {
            case DEBUG: label = "\033[34m[DEBUG] "; break;
            case INFO:  label = "\033[37m[INFO]  "; break;
            case WARN:  label = "\033[33m[WARN]  "; break;
            case ERROR: label = "\033[31m[ERROR] "; break;
        }
        return label;
    }
};


/*
    These macros provide an alternative interface. They add an additional level check, but give the benefit that they don't evaluate the arguments if the condition is not met, instead of evaluating and then ignoring them
*/ 
#define LOG_DEBUG if (DEBUG <= LOGCFG.level) LOG(DEBUG)
#define LOG_INFO if (INFO <= LOGCFG.level) LOG(INFO)
#define LOG_WARN if (WARN <= LOGCFG.level) LOG(WARN)
#define LOG_ERROR if (ERROR <= LOGCFG.level) LOG(ERROR) 




