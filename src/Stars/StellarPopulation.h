#pragma once
#include <vector>
#include "../Parameters/GlobalParameters.h"

//! A simple struct for tracking the number of stars of a given mass
class IsoMass
{
	public:
		int Weight;
		int TimeIndexCreated;
		int TimeIndexDestroyed;
		IsoMass();
		IsoMass(int n, int birth, int death);
};

class StellarPopulation
{
	public:
		StellarPopulation(const GlobalParameters & param);
	
	private:
		const GlobalParameters & Param;
		std::vector<IsoMass> Distribution;
};
