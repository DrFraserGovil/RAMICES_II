#include "Ring.h"


//! Initialises itself into a primordial state
Ring::Ring(int index, double mass,const GlobalParameters & param): Param(param), Width(param.Galaxy.Radius / param.Galaxy.RingCount), Radius((index + 0.5)*param.Galaxy.Radius / param.Galaxy.RingCount), Gas(GasReservoir::Primordial(mass,param))
{
	RadiusIndex = index;
	
}

double Ring::Mass()
{
	
}
