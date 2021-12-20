#pragma once
#include "../Parameters/GlobalParameters.h"
#include "../Gas/GasReservoir.h"

class SimpleYield
{
	public:
		const SourceProcess Process;
		SimpleYield(const GlobalParameters & param, SourceProcess Process);
		
		void operator()(GasReservoir & scatteringReservoir, int nObjects, int birthIndex)  const;
	
	private:
		
		double hotInjectionFraction;
		void NSM_Initialise();
		void SNIa_Initialise();
		
		void RemnantInject( GasReservoir & scatteringReservoir, int Nstars, int birthIndex) const;
		std::vector<double> Grid;
		const GlobalParameters & Param;
};
