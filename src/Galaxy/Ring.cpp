#include "Ring.h"


//! Initialises itself into a primordial state
Ring::Ring(int index, double mass,const IMF_Functor & imf, SLF_Functor & slf, const GlobalParameters & param): Param(param), Width(param.Galaxy.Radius / param.Galaxy.RingCount), Radius((index + 0.5)*param.Galaxy.Radius / param.Galaxy.RingCount), Gas(GasReservoir::Primordial(mass,param)), Stars(param,index,imf,slf), IMF(imf)
{
	RadiusIndex = index;
	
}

double Ring::Mass()
{
	
}

void Ring::MakeStars()
{
	Stars.Form(Gas);
}
void Ring::KillStars(int time)
{
	Stars.Death(time);
}

void Ring::Cool()
{
	Gas.PassiveCool(Param.Meta.TimeStep);
}
