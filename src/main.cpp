#include <iostream>
#include "utility/Log.h"
#include "utility/parallel.h"
#include "utility/Timer.h"
#include "utility/Random.h"

#include "settings/SimulationSettings.h"
#include "population/lifetime.h"
// int main(int argc, char ** argv)
// {
//     Settings.Initialise(argc,argv);
    
//     Stellar::Lifetime L;
    
//     return (0);
// }


void testPrint(LogLevel level)
    {
        LogConfig.SetLevel(level);
        LOG(DEBUG) << "This is detailed debugging";
        LOG(INFO) << "This is progress information";
        LOG(WARN) << "This is a warning that something went wrong\nBut was recovered.";
        LOG(ERROR) << "Something has gone very badly wrong";
    }

    int main(int argc, char**argv)
    {
        Settings.Initialise(argc,argv);
        LogConfig.SetLevel(INFO);
        testPrint(DEBUG);
        testPrint(WARN);
    }