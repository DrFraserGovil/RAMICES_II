#include <iostream>
#include "utility/Log.h"
#include "utility/parallel.h"
#include "utility/Timer.h"
#include "utility/Random.h"
int square(int index,bool wait)
{
    // if (wait)
    // {
    //     auto  R = Random();
    //     int r = R.UniformInteger(300,320);
    //     // std::this_thread::sleep_for(std::chrono::microseconds(r));
    // }
    LOG(DEBUG) << "hi from " << index;
    return index*index + 1;
}



int main(int argc, char ** argv)
{
    LogConfig.SetLevel(3);
    int nSquares = 1e2;
    Timer T;
    auto  R = Random(0);
    std::vector<int> vals(nSquares);
    T.start();
    for (int i =0 ; i < nSquares; ++i)
    {
        vals[i] = square(i,true);
    }
    T.stop();
    
    int r = R.UniformInteger(0,nSquares-1);
    LOG(INFO) << "Basic implementation " <<r << " " << vals[r] << " " << square(r,false) << " computed in " << T.measure();
    // auto n = {1,2}
    for (int n = 0; n < 10; ++n)
    {
    ParallelForPool Parallel(n);
    
   
    
    T.start();
    Parallel.For(nSquares,square,&vals,true);
    T.stop();
    int r = R.UniformInteger(0,nSquares-1);
    LOG(INFO) << "With " << n << " workers: " <<r << " " << vals[r] << " " << square(r,false) << " computed in " << T.measure();
    }
    // for (int i = 0; i < nSquares; i+=25)
    // {
    //     std::cout << i << " " << vals[i] << " " << square(i,false) << std::endl;
    // }
    // LOG(INFO) << ""

    return (0);
}