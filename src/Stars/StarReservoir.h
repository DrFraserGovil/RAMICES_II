#pragma once

#include <vector>
#include "../Parameters/GlobalParameters.h"
#include "../Gas/GasReservoir.h"
class StarReservoir
{
	public:
		StarReservoir(const GlobalParameters & Param);
		double Mass();
		
		void Birth(GasReservoir & gas);
		
		
	private:
		std::vector<double> StarPopulations;
		const GlobalParameters & Param;
};
