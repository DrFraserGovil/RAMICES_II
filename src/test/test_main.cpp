
#include "catch_amalgamated.hpp"

#include "utility/utility_main.h"
#include "settings/settings_main.h"

//physics begins - settings have been validated, so default initialise a spoofed settings object
#include "../settings/SimulationSettings.h"
#include "galaxy/galaxy_main.h"

int main(int argc, char** argv)
{
	auto defaultState = ArgSpoofer({});
	Settings.Initialise(defaultState.argc,defaultState.argv); //correctly initialises the global settings object into its default state. The code may choose to change these values at a later date, but they will default initialise to here
	return Catch::Session().run(argc,argv);
}