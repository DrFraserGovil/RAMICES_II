#pragma once
#include <chrono>


class Timer
{
    private:
        std::chrono::high_resolution_clock::time_point start_time;
        std::chrono::high_resolution_clock::time_point stop_time;

    public:
        void start()
        {
            start_time = std::chrono::high_resolution_clock::now();
        }

        void stop()
        {
            stop_time = std::chrono::high_resolution_clock::now();
        }

        double measure()
        {
            using namespace std::chrono;
            return duration_cast<nanoseconds>
                (stop_time - start_time).count() / 1e9;
        }
};