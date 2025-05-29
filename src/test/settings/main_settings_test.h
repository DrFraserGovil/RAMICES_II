#pragma once
#include "../catch_amalgamated.hpp" 
#include "../../settings/SimulationSettings.h"

#include "../mock/MockFile.h"
#include "../mock/coutCatch.h"

TEST_CASE("Compile-time testing","[compilation][settings]")
{
	
	//as with some of the other concepts around the X-Macro objects, this stuff mainly exists to force the compiler to test if these objects exist

	try //add a try here because the actual runtime might fail (due to Validation errors) -- those are checked later!
	{
		SimulationSettings Settings;
		REQUIRE(std::is_same_v<decltype(Settings.System),SystemSettings>);
		REQUIRE(std::is_same_v<decltype(Settings.Thermal),ThermalSettings>);
		REQUIRE_FALSE(std::is_same_v<decltype(Settings.Yield),SystemSettings>);
	}
	catch(...){};
}

TEST_CASE("Container enforces string uniqueness","[settings][parameter]")
{
	REQUIRE_NOTHROW(SimulationSettings()); //just to catch if the default constructor throws

	SimulationSettings Settings;

	/*
		VS code might flag some errors here -- this is because SetTrigger (and GetTrigger) are compile-time exposed only when UNITTEST is a defined macro -- i.e. it is a test-only facility designed for this purpose.

		They exist purely to try and prod the container objects into having name collisions which are otherwise defined at compile time by the user
	*/


	REQUIRE_NOTHROW(Settings::Parameter<bool>(false,Settings.Thermal.FeedbackFactor.GetTrigger())); //trigger-guards are non-global, so can do this (but probably shouldn't)
	Settings.System.ParallelThreads.SetTrigger(Settings.Thermal.FeedbackFactor.GetTrigger());
	REQUIRE_THROWS(Settings.Validate());
}

TEST_CASE("SimulationSettings parses cmdline","[settings][parse]")
{
	SimulationSettings Settings;

	SpoofedStructure cmd({"-v","3","-thread","8","-feedback-heat","0.01"});
	REQUIRE_NOTHROW(Settings.Initialise(cmd.argc,cmd.argv));

	REQUIRE(Settings.System.Verbosity == 3);
	REQUIRE(Settings.System.ParallelThreads == 8);
	REQUIRE(Settings.Thermal.FeedbackFactor == 0.01);
}


TEST_CASE("SimulationSettings reads config files","[settings][configure]")
{
	MockFile f;
	f << "v_5\n";
	f << "feedback-heat_0.07\n";
	f << "thread_18\n";


	SpoofedStructure cmd({"-config",f.Name(),"-config-delimiter","_"});
	SimulationSettings Settings;
	REQUIRE_NOTHROW(Settings.Initialise(cmd.argc,cmd.argv));

	REQUIRE(Settings.System.ParallelThreads == 18);
	REQUIRE(Settings.Thermal.FeedbackFactor == 0.07);
	REQUIRE(Settings.System.Verbosity == 3); //Validate catches a v > 5, and resets it to 3 so this is a cheeky check
}

TEST_CASE("SimulationSettings reads config files and cmd-lines","[settings][parse][configure]")
{
	MockFile f;
	f << "v_1\n";
	f << "feedback-heat_0.07\n";
	f << "thread_18\n";

	SpoofedStructure cmd({"-config",f.Name(),"-config-delimiter","_","-v","3","-feedback-heat","0.5"});
	SimulationSettings Settings;
	REQUIRE_NOTHROW(Settings.Initialise(cmd.argc,cmd.argv));

	REQUIRE(Settings.System.ParallelThreads == 18); //this was not in the cmd line, so remains at the config value
	REQUIRE(Settings.Thermal.FeedbackFactor == 0.5); 
	REQUIRE(Settings.System.Verbosity == 3); 
}