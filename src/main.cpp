#include <iostream>
#include "utility/Log.h"
#include "utility/parallel.h"
#include "utility/Timer.h"
#include "utility/Random.h"

#include "settings/SimulationSettings.h"
#include "population/lifetime.h"
#include <thread>
// int main(int argc, char ** argv)
// {
//     Settings.Initialise(argc,argv);
    
//     Stellar::Lifetime L;
    
//     return (0);
// }


void testPrint(LogLevel level)
    {
        GlobalLog::Config.SetLevel(level);
        LOG(ERROR) << "Something has gone very badly wrong";
        LOG(INFO) << "This is progress information";
        LOG(WARN) << "This is a warning that something went wrong\nBut was recovered.";
        LOG(DEBUG) << "This is detailed debugging";
    }

    int main(int argc, char**argv)
    {
        Settings.Initialise(argc,argv);
        testPrint(DEBUG);

        sleep(2);
        LOG(INFO).ErasePrevious();
        sleep(2);
        LOG(WARN).ErasePrevious();
        sleep(2);
        LOG(INFO).ErasePrevious();
        testPrint(WARN);
    }