#pragma once
#include <chrono>

#include <exception>


class Timer
{
    private:
        std::chrono::high_resolution_clock::time_point start_time;
        std::chrono::high_resolution_clock::time_point stop_time;
        bool startCalled;
        bool stopCalled;

    public:
        Timer()
        {
            startCalled = false;
            stopCalled = false;
        }
        void start()
        {
            start_time = std::chrono::high_resolution_clock::now();
            startCalled = true;
            stopCalled = false;
        }

        void stop()
        {
            if (!startCalled)
            {
                throw std::logic_error("Cannot stop the timer without first calling Timer.start()");
            }
            stopCalled = true;
            stop_time = std::chrono::high_resolution_clock::now();
        }


        double measure()
        {
            if (!startCalled)
            {
                throw std::logic_error("Cannot measure a time without first calling Timer.start()");
            }
            if (!stopCalled)
            {
                stop();
            }
            using namespace std::chrono;
            return duration_cast<nanoseconds>
                (stop_time - start_time).count() / 1e9;
        }
};