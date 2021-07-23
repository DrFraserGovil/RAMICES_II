#include "StellarPopulation.h"

StellarPopulation::StellarPopulation()
{
	Stars.resize(0);
}

StellarPopulation::StellarPopulation(Options * options)
{
	Opts = options;
}

double StellarPopulation::Mass()
{
	double m = 0;
	for (int i = 0; i < Stars.size(); ++i)
	{
		m += Stars[i].Mass * Stars[i].NStars;
	}
	return m;
}
