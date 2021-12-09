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
		auto stream = r[source];
		std::cout << "From process " << c << ": \n";
		for (int i = 0; i < ElementCount; ++i)
		{
			ElementID elem = (ElementID)i;
			std::cout << "\tElement " << i << " Cold = " << stream.Cold[elem] << "  Hot = " << stream.Hot[elem] << std::endl;
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
	r[Accreted].Hot[Hydrogen] = 10;
	
	printRes(r);
	r.Deplete(2);
	printRes(r);
	
	std::cout << r.Mass() << std::endl;
	return 0;
}
