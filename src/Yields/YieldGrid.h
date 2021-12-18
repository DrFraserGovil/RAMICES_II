#pragma once
#include "../Parameters/GlobalParameters.h"
#include "../Gas/GasReservoir.h";
//~ #include <ostream>
class YieldGrid
{
	public:
		const SourceProcess Process;
		YieldGrid(const GlobalParameters & param, SourceProcess Process);
		
		void operator()(int mass, double z);
		void In
	private:
		bool IsNet;
		const GlobalParameters & Param;
		std::vector<std::vector<std::vector<double>>> Grid;

		double hotInjectionFraction;
		
		void CCSN_Initialise();
		void NSM_Initialise();
		void AGB_Initialise();
		void SNIa_Initialise();
		void InitialiseLargeGrid(int mSize, int zSize);
		
		//allows the grid size to be truncated for CCSN etc.
		int MassOffset;
		
		void NetInject( std::vector<std::vector<Gas>> & GrossOutputStream, int Nstars, int mass, double z, int birthIndex, GasReservoir & birthReservoir);
};
