#include <stdio.h>
#include <iostream>


#include "Parameters/GlobalParameters.h"

GlobalParameters Params;

int main(int argc, char** argv)
{
		
	
	Params.Initialise(argc,argv);
	
	std::cout << Params.Galaxy.PrimordialHotFraction <<"\n";
	std::cout << Params.Thermal.GasCoolingTimeScale <<std::endl;
	
	//~ //initialise main galaxy object
	//~ Galaxy g = Galaxy(&options);

	//~ g.Evolve();
	

	return 0;
}
