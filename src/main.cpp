#include <iostream>
#include "utility/Log.h"
#include "utility/parallel.h"
#include "utility/Timer.h"
#include "utility/Random.h"

#include "settings/SettingGroups.h"
int main(int argc, char ** argv)
{
   
   
    System S;
    S.Parse(argc,argv);
    LogConfig.SetLevel(S.Verbosity);
    
    LOG(INFO) << S.Verbosity;


    return (0);
}