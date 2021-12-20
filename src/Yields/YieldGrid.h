#pragma once
#include "../Parameters/GlobalParameters.h"
#include "../Gas/GasReservoir.h";
#include "../Stars/RemnantPopulation.h"
//~ #include <ostream>
class RemnantPopulation; // predeclaration for circular linkage

class YieldGrid
{
	public:
		const SourceProcess Process;
		YieldGrid(const GlobalParameters & param, SourceProcess Process);
		
		void operator()(GasReservoir & scatteringReservoir, RemnantPopulation & remnantReservoir, int Nstars, int mass, double z, int birthIndex, GasReservoir & birthReservoir) const;
		void operator()(GasReservoir & scatteringReservoir, int nObjects, int birthIndex)  const;
	private:
		bool IsBasic;
		const GlobalParameters & Param;
		std::vector<std::vector<std::vector<double>>> Grid;

		double hotInjectionFraction;
		
		void CCSN_Initialise();
		void NSM_Initialise();
		void AGB_Initialise();
		void SNIa_Initialise();
		void InitialiseLargeGrid(int mSize, int zSize);
		//~ GasStream TempStream;
		//allows the grid size to be truncated for CCSN etc.
		int MassOffset;
		
		void StellarInject( GasReservoir & scatteringReservoir, RemnantPopulation & remnantReservoir, int Nstars, int mass, double z, int birthIndex, GasReservoir & birthReservoir) const;
		void RemnantInject( GasReservoir & scatteringReservoir, int Nstars, int birthIndex) const;
};
