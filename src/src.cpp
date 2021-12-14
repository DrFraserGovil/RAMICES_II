#include <stdio.h>
#include <iostream>


#include "Parameters/GlobalParameters.h"
#include "Galaxy/Galaxy.h"

GlobalParameters Params;

std::chrono::time_point<std::chrono::system_clock> startTime;

void Welcome()
{
	if (Params.Meta.Verbosity > 0)
	{
		std::string welcomeFile = Params.Meta.ResourceRoot.Value + Params.Meta.WelcomeFile.Value;
		forLineIn(welcomeFile,
			std::cout << FILE_LINE << "\n";
		);
		std::cout << "\nNew Simulation initiated on " << JSL::CurrentTime();
		std::cout <<"\nFile output is being sent to " << Params.Output.Root.Value << "\n";
	}
}

void Exit()
{
	if (Params.Meta.Verbosity > 0)
	{
		auto endTime = std::chrono::system_clock::now();
		std::cout << "\n\nSimulation complete. Computation duration was " << JSL::FormatClock(startTime,endTime);
		std::cout << "\nHave a nice day!";
	}
}
int main(int argc, char** argv)
{
	startTime = std::chrono::system_clock::now();
	
	Params.Initialise(argc,argv);
	
	Welcome();

	//~ initialise main galaxy object
	Galaxy g = Galaxy(Params);

	g.Evolve();
	
	Exit();
	return 0;
}
