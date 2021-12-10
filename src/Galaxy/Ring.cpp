#include "Ring.h"


Ring::Ring(int index, double mass, const GlobalParameters & param): Param(param)
{
	RadiusIndex = 0;
	Gas = GasReservoir::Primordial(mass,param);
	
}

