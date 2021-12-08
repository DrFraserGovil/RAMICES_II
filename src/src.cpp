#include <stdio.h>
#include <iostream>


#include "Parameters/GlobalParameters.h"
//~ #include "Galaxy/Galaxy.h"
#include "Gas/GasReservoir.h"

GlobalParameters Params;

void printRes(GasReservoir r)
{
	
	for (int c = 0; c < 2; ++c)
	{
		SourceProcess source = (SourceProcess)c;
		auto stream = r.Component(source);
		std::cout << "From process " << c << ": \n";
		for (int i = 0; i < ElementCount; ++i)
		{
			std::cout << "\tElement " << i << " Cold = " << stream.Cold.Species[i] << "  Hot = " << stream.Hot.Species[i] << std::endl;
		}
	}
}

int main(int argc, char** argv)
{
		
	
	Params.Initialise(argc,argv);
	
	std::cout << Params.Galaxy.PrimordialHotFraction <<"\n";
	std::cout << Params.Thermal.GasCoolingTimeScale <<std::endl;
	
	//~ initialise main galaxy object
	//~ Galaxy g = Galaxy(Params);

	//~ g.Evolve();
	GasReservoir r = GasReservoir::Primordial(10,Params);
	GasStream g(Accreted);
	Gas p2 = Gas::Primordial(20.0);
	g.StreamIn(p2,1);
	r.Absorb(g);
	printRes(r);
	r.Deplete(2);
	printRes(r);
	
	std::cout << r.Mass() << std::endl;
	return 0;
}
