#include <iostream>
#include "utility/Log.h"
int main(int argc, char ** argv)
{
    LOGCFG.SetLevel(3);
    LOGCFG.headers = true;
    LOG(INFO) << "Welcome to RAMICES III";
    LOG(WARN) << "Welcome to RAMICES III";
    return (0);
}