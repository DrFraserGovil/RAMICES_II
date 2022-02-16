#pragma once
#include "../Parameters/GlobalParameters.h"
#include "../Gas/GasReservoir.h"

class SimpleYield
{
	public:
		const SourceProcess Process;
		SimpleYield(const GlobalParameters & param, YieldProcess Process);
		
		void operator()(GasReservoir & scatteringReservoir, double nObjects)  const;
	
	private:
		
		double hotInjectionFraction;
		void NSM_Initialise();
		void SNIa_Initialise();
		
		void RemnantInject( GasReservoir & scatteringReservoir, double Nstars) const;
		std::vector<double> Grid;
		const GlobalParameters & Param;
};
