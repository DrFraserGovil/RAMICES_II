#pragma once

//~ #include "../Stars/RemnantPopulation.h"
#include "../Parameters/GlobalParameters.h"
#include "../Gas/GasReservoir.h"
#include "YieldRidge.h"
//~ #include <ostream>

struct RemnantOutput
{
	RemnantType Type;
	double Mass;
};
struct Interpolator
{
	int UpperID;
	int LowerID;
	double LinearFactor;
	double Interpolate(double lower, double upper)
	{
		double val = lower + LinearFactor*(upper - lower);
		if (val < 0 && (lower > 0 || upper > 0))
		{
			val = 0;
		}
		return val;
	}
};

class YieldGrid
{
	public:
		const SourceProcess Process;
		YieldGrid(const GlobalParameters & param, YieldProcess Process);
		
		RemnantOutput operator()(GasReservoir & scatteringReservoir, double Nstars, int mass, double z, const std::vector<GasStream> & birthGas) const;
		
	private:
		const GlobalParameters & Param;
		std::vector<std::vector<std::vector<double>>> Grid;

		double hotInjectionFraction;
		
		void CCSN_Initialise();
		
		void AGB_Initialise();
		void ECSN_Initialise();
		void InitialiseLargeGrid(int mSize, int zSize);
		//~ GasStream TempStream;
		//allows the grid size to be truncated for CCSN etc.
		int MassOffset;
		
		RemnantOutput StellarInject( GasReservoir & scatteringReservoir, double Nstars, int mass, double z, const std::vector<GasStream> & birthGas) const;
		
		void LoadOrfeoYields();
		void LoadMarigoYields();
		void LoadLimongiYields();
		void LoadMaederYields();
		std::vector<std::vector<YieldRidge>> RidgeStorage;
		int RemnantLocation;
		
		Interpolator MetallicityInterpolation(double z) const;
		
		void ElementProduction(ElementID element, double synthesisFraction, double ejectaMass,std::vector<GasStream> & output, const std::vector<GasStream> & birthStreams) const;
		void ElementDestruction(ElementID element, double synthesisFraction, double ejectaMass, std::vector<GasStream> & output, const std::vector<GasStream> & birthStreams) const;
		
		// Creation properties
		
		void CreateGrid();
		YieldBracket GetBracket(int id, double mass, double z, bool overhang);
		//~ std::vector<int> SourcePriority;
		std::vector<std::vector<int>> SourcePriority;
		void SaveGrid(std::string name);
		void PurityEnforce();
};
