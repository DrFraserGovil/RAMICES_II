#pragma once
#include <vector>

#include "../Options.h"
#include "Star.h"

class StellarPopulation
{
	public:
		std::vector<StarSet> Stars;
	
		
		StellarPopulation();
		StellarPopulation(Options * opts);
		
		double Mass();
	private:
		Options * Opts;
		
};
