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

//test.cpp

void testPrint(LogLevel level)
{
    GlobalLog::Config.SetLevel(level);
    LOG(INFO) << "This is an initial info, which cannot be deleted!";
    LOG(INFO) << "This resets the `info' memory, so the previous line is forgotten";
    LOG(ERROR) << "This is an error";
    LOG(WARN) << "This is a warning\nIt is split across lines -- these will all be deleted together";
    LOG(DEBUG) << "This is noisy debugging";
}

int main(int argc, char**argv)
{
    testPrint(DEBUG); //prints everything
    
    
    // sleep(1);
    LOG(WARN).ErasePrevious(); //erases both the DEBUG + WARN
    sleep(1);
    LOG(DEBUG).ErasePrevious(); //no DEBUG to delete -- nothing happens
    sleep(1);

    LOG(INFO).ErasePrevious(); //The ERROR takes priority -- nothing is deleted


    sleep(1);
    LOG(ERROR).ErasePrevious(); //Deletes the error
    sleep(1);
    LOG(INFO).ErasePrevious(); //Deletes the *second* info block
    sleep(1);
    LOG(INFO).ErasePrevious(); //Does NOT delete the first INFO block 
    sleep(1);
    // This is because the internal memory cannot remember it. 
    LOG(INFO).Erase(1); //manually delete the line -- not recommended!
}