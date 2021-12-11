#pragma once
#include <vector>
#include "../Parameters/GlobalParameters.h"

class StellarPopulation
{
	public:
		StellarPopulation(const GlobalParameters & param);
	
	private:
		const GlobalParameters & Param;
};
