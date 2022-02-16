#include <stdio.h>
#include <iostream>


#include "Parameters/GlobalParameters.h"
#include "Parameters/InitialisedData.h"
#include "Galaxy/Galaxy.h"

GlobalParameters Params;

std::chrono::time_point<std::chrono::system_clock> startTime;

void Welcome()
{
	if (Params.Meta.Verbosity > 0)
	{
		forLineIn(Params.Resources.WelcomeFile.Value,
			std::cout << FILE_LINE << "\n";
		);
		std::cout << "\nNew Simulation initiated on " << JSL::CurrentTime();
		std::cout <<"\nFile output is being sent to " << Params.Output.Root.Value << "\n";
		std::cout << "\nBeginning initialisation of data structures...." << std::endl;
	}
}

void Exit()
{
	if (Params.Meta.Verbosity > 0)
	{
		auto endTime = std::chrono::system_clock::now();
		std::cout << "\n\nSimulation complete. Computation duration was " << JSL::FormatClock(startTime,endTime);
		std::cout << "\nHave a nice day!\n";
	}
}
int main(int argc, char** argv)
{
	startTime = std::chrono::system_clock::now();
	std::cout << "init" << std::endl;
	Params.Initialise(argc,argv);
	
	Welcome();


	//initialise the precomputed portions of the data which will be available (nearly) globally
	
	InitialisedData Data(Params);
	
	//~ initialise main galaxy object
	Galaxy g = Galaxy(Data);
	int n = Params.Galaxy.RingCount;
	
	double amount = 1e-3;
	int pol = 10;
	int centre = 2*n/9;
	amount = amount * g.Rings[centre].Gas.Mass();
	//~ for (int i = 0; i < pol;++i)
	//~ {
		//~ if (centre + i < n)
		//~ {	
			//~ g.Rings[centre + i].Gas[Accreted].Cold(Europium) = amount ;
		//~ }
		//~ if (centre - i >= 0)
		//~ {
			//~ g.Rings[centre - i].Gas[Accreted].Cold(Europium) = amount ;
		//~ }
		//~ amount = amount / 10;
	//~ }
	
	Data.UrgentLog("Beginning main computation loop...\n");
	g.Evolve();
	
	auto endTime = std::chrono::system_clock::now();
	Data.UrgentLog("\tMain Computation loop complete in " + JSL::FormatClock(startTime,endTime) + ".\n");
	
	if (Params.Meta.StellarSynthesisActive)
	{
		Data.UrgentLog("Beginning Synthesis of mock catalogue\n");
		g.SynthesiseObservations();
	}
	
	
	Exit();
	return 0;
}
