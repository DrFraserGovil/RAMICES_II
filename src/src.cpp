#include <stdio.h>
#include <iostream>


#include "Parameters/GlobalParameters.h"
#include "Galaxy/Galaxy.h"

GlobalParameters Params;

int main(int argc, char** argv)
{
		
	
	Params.Initialise(argc,argv);
	

	//~ initialise main galaxy object
	Galaxy g = Galaxy(Params);

	g.Evolve();
	
	return 0;
}
