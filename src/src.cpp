#include <stdio.h>
#include <iostream>


#include "Parameters/GlobalParameters.h"
//~ #include "Galaxy/Galaxy.h"
#include "Gas/GasReservoir.h"

GlobalParameters Params;

int main(int argc, char** argv)
{
		
	
	Params.Initialise(argc,argv);
	
	std::cout << Params.Galaxy.PrimordialHotFraction <<"\n";
	std::cout << Params.Thermal.GasCoolingTimeScale <<std::endl;
	
	//~ initialise main galaxy object
	//~ Galaxy g = Galaxy(Params);

	//~ g.Evolve();
	GasReservoir r = GasReservoir::Primordial(10,Params);

	for (int c = 0; c < ProcessCount; ++c)
	{
		SourceProcess source = (SourceProcess)c;
		auto stream = r.Component(source);
		std::cout << "From process " << c << ": \n";
		for (int i = 0; i < ElementCount; ++i)
		{
			std::cout << "\tElement " << i << " Cold = " << stream.Cold.Species[i] << "  Hot = " << stream.Hot.Species[i] << std::endl;
		}
	}


	return 0;
}
