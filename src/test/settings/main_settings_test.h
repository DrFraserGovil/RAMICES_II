#pragma once
#include "../catch_extended.h" 
#include "../../settings/SimulationSettings.h"

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


	REQUIRE_NOTHROW(Setting::Parameter<bool>(false,Settings.Thermal.FeedbackFactor.GetTrigger())); //trigger-guards are non-global, so can do this (but probably shouldn't)
	Settings.System.ParallelThreads.SetTrigger(Settings.Thermal.FeedbackFactor.GetTrigger());
	REQUIRE_THROWS(Settings.Register());
}

TEST_CASE("SimulationSettings parses cmdline","[settings][parse]")
{
	SimulationSettings Settings;

	ArgSpoofer cmd({"-v","3","-thread","8","-feedback-heat","0.01"});
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


	ArgSpoofer cmd({"-config",f.Name(),"-config-delimiter","_"});
	SimulationSettings Settings;
	REQUIRE_NOTHROW(Settings.Initialise(cmd.argc,cmd.argv));

	REQUIRE(Settings.System.ParallelThreads == 18);
	REQUIRE(Settings.Thermal.FeedbackFactor == 0.07);
	REQUIRE(Settings.System.Verbosity == 3); //Validate catches a v > 5, and resets it to 3 so this is a cheeky check
}

TEST_CASE("SimulationSettings reads *unusual* config files","[settings][configure][edgecase]")
{
	//same test as above, but with some unusual additions to the config file, to ensure that it parses correctly
	MockFile f;
	f << "v_5\n";
	f << "\n";  //blank line
	f << "//This line is only a comment\n"; //full line comment
	f << "feedback-heat_0.07 //this is a test comment, it should be ignored\n"; //inline comment
	f << "thread_18"; //no terminating line break


	ArgSpoofer cmd({"-config",f.Name(),"-config-delimiter","_"});
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

	ArgSpoofer cmd({"-config",f.Name(),"-config-delimiter","_","-v","3","-feedback-heat","0.5"});
	SimulationSettings Settings;
	REQUIRE_NOTHROW(Settings.Initialise(cmd.argc,cmd.argv));

	REQUIRE(Settings.System.ParallelThreads == 18); //this was not in the cmd line, so remains at the config value
	REQUIRE(Settings.Thermal.FeedbackFactor == 0.5); 
	REQUIRE(Settings.System.Verbosity == 3); 
}

TEST_CASE("Warnings & Errors","[settings][configure][edgecase][warnings][errors]")
{
	MockFile f;
	f << "v_1\n";
	f << "feedback-heat_0.07\n";
	f << "thread_18\n";

	SimulationSettings Settings;
	ArgSpoofer cmd({"-config",f.Name(),"-config-delimiter","_"});
	ArgSpoofer cmdSafeDuplicate({"-config",f.Name(),"-config-delimiter","_","-thread","10"});


	SECTION("Duplicate arguments throw an error")
	{
		f << "thread_10\n";
		auto msg = REQUIRE_ERROR(Settings.Initialise(cmd.argc,cmd.argv));
		REQUIRE_THAT(msg,ContainsSubstring("thread"));//check it is throwing off duplicated thread 

		ArgSpoofer cmdBadDuplicate({"-v","5","-v","10"});
		msg = REQUIRE_ERROR(Settings.Initialise(cmdBadDuplicate.argc,cmdBadDuplicate.argv));
		REQUIRE_THAT(msg,ContainsSubstring("thread"));//check it is throwing off duplicated thread 
	}
	SECTION("Duplicated arguments are permitted if in different modes; with cmd overriding them")
	{
		REQUIRE_NO_WARN(Settings.Initialise(cmdSafeDuplicate.argc,cmdSafeDuplicate.argv));

		REQUIRE(Settings.System.ParallelThreads == 10); //not 18, the config value
	}

	SECTION("Throws warnings from bad cmd")
	{
		ArgSpoofer cmdNoDash({"-v","5","thread","10"}); //note thread, not -thread
		auto msg = REQUIRE_WARN(Settings.Initialise(cmdNoDash.argc,cmdNoDash.argv));
		REQUIRE_THAT(msg,ContainsSubstring("thread"));


		//check a warning is thrown if you give a parameter it doesn't recognise
		ArgSpoofer cmdFalseParam({"-nonExistentParameter","5"});
		msg = REQUIRE_WARN(Settings.Initialise(cmdFalseParam.argc,cmdFalseParam.argv));
		REQUIRE_THAT(msg,ContainsSubstring("nonExistentParameter"));
	}
}