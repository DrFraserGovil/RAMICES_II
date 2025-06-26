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
#include "utility/Archiver.h"
using namespace Archiver;
int main(int argc, char** argv)
    {
        Archive A("test.archive",Read);
        
        auto output = A.GetTabular<std::string,int,std::vector<int>>("people.txt");

        std::cout << "Name\tAge\t#. of Accolades\n";
        for (auto line : output)
        {
            std::cout << std::get<0>(line) <<"\t" << std::get<1>(line) << "\t" << std::get<2>(line).size() << "\n";
        }
    }