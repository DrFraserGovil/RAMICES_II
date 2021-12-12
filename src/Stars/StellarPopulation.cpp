#include "StellarPopulation.h"

IsoMass::IsoMass()
{
	Weight = 0;
	TimeIndexCreated = 0;
	TimeIndexDestroyed = 1e10;
}
IsoMass::IsoMass(int n, int birth, int death)
{
	Weight = n;
	TimeIndexCreated = birth;
	TimeIndexDestroyed = death;
}


StellarPopulation::StellarPopulation(const GlobalParameters & param): Param(param)
{
	Distribution.resize(Param.Meta.SimulationSteps);
}
