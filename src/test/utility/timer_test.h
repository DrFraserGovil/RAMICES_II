#pragma once
#include "../../utility/Timer.h"
#include <thread>

TEST_CASE("Timer State Handling","[timer][utility]")
{
	Timer T;

	SECTION("Test ordering")
	{
		REQUIRE_THROWS(T.measure());
		REQUIRE_THROWS(T.stop());
		T.start();
		REQUIRE_NOTHROW(T.measure());
		REQUIRE_NOTHROW(T.stop());

	}

	SECTION("Sequential calls")
	{
		T.start();
		REQUIRE_NOTHROW(T.start());
		double firstCall = T.measure();
		REQUIRE_NOTHROW(T.stop());
		double secondCall = T.measure();
		REQUIRE_NOTHROW(T.measure());
		double pausedCall = T.measure();
		REQUIRE(secondCall > firstCall);
		REQUIRE(secondCall == pausedCall);
	}
	SECTION("Bounding the timer")
	{
		auto t = {1,50,100,300};
		for (int wait : t)
		{
			T.start();
			std::this_thread::sleep_for(std::chrono::milliseconds(wait)); // Sleep for 50ms
			T.stop();
			REQUIRE(T.measure() >= 0.001*wait);
		}
	}
}