#include <iostream>
#include "utility/Log.h"
int main(int argc, char ** argv)
{
    LogConfig.SetLevel(1);
    LOG(DEBUG) << "Welcome to RAMICES III";
    LOG(INFO) << "Welcome to RAMICES III";
    LOG(WARN) << "Welcome to RAMICES III";
    LOG(ERROR) << "Welcome to RAMICES III";
    LOG(DEBUG);
    return (0);
}