#include <stdio.h>
#include <iostream>


#include "Parameters/GlobalParameters.h"
//~ #include "Galaxy/Galaxy.h"
#include "Gas/GasReservoir.h"

GlobalParameters Params;

void printRes(GasReservoir r)
{
	
	for (int c = 0; c < ProcessCount; ++c)
	{
		SourceProcess source = (SourceProcess)c;
		auto stream = r[source];
		bool headerPrinted = false;
		
		for (int i = 0; i < ElementCount; ++i)
		{
			ElementID elem = (ElementID)i;
			if (stream.Cold(elem)> 0 || stream.Hot(elem) > 0)
			{
				if (!headerPrinted)
				{
					std::cout << "From process " << c << ": \n";
					headerPrinted = true;
				}
				std::cout << "\t" << Params.Element.ElementNames[elem] << " Cold = " << stream.Cold(elem) << "  Hot = " << stream.Hot(elem) << std::endl;
			}
		}
	}
}

int main(int argc, char** argv)
{
		
	
	Params.Initialise(argc,argv);
	

	
	//~ std::cout << Params.Galaxy.PrimordialHotFraction <<"\n";
	//~ std::cout << Params.Thermal.GasCoolingTimeScale <<std::endl;
	
	//~ initialise main galaxy object
	//~ Galaxy g = Galaxy(Params);

	//~ g.Evolve();
	GasReservoir r = GasReservoir::Primordial(10,Params);


	
	r[Accreted].Hot(Hydrogen) = 10;
	printRes(r);
		
	double depleteAmount = 5;
	std::cout << " \nI am now removing " << 5 << " units from the reservoir...\n\n";
	r.Deplete(depleteAmount);
	
	printRes(r);
	
	std::cout << r.Mass() << std::endl;
	return 0;
}
