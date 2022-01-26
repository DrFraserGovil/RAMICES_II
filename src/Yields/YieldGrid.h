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
		return lower + LinearFactor*(upper - lower);
	}
};

class YieldGrid
{
	public:
		const SourceProcess Process;
		YieldGrid(const GlobalParameters & param, YieldProcess Process);
		
		RemnantOutput operator()(GasReservoir & scatteringReservoir, double Nstars, int mass, double z, int birthIndex, GasReservoir & birthReservoir) const;
		
	private:
		const GlobalParameters & Param;
		std::vector<std::vector<std::vector<double>>> Grid;

		double hotInjectionFraction;
		
		void CCSN_Initialise();
		
		void AGB_Initialise();
		
		void InitialiseLargeGrid(int mSize, int zSize);
		//~ GasStream TempStream;
		//allows the grid size to be truncated for CCSN etc.
		int MassOffset;
		
		RemnantOutput StellarInject( GasReservoir & scatteringReservoir, double Nstars, int mass, double z, int birthIndex, GasReservoir & birthReservoir) const;
		
		void LoadOrfeoYields();
		void LoadMarigoYields();
		void LoadLimongiYields();
		void LoadMaederYields();
		std::vector<std::vector<YieldRidge>> RidgeStorage;
		int RemnantLocation;
		
		Interpolator MetallicityInterpolation(double z) const;
		
		void ElementProduction(ElementID element, double synthesisFraction, double ejectaMass,std::vector<GasStream> & output, const std::vector<GasStream> & birthStreams,bool wordy) const;
		void ElementDestruction(ElementID element, double synthesisFraction, double ejectaMass, std::vector<GasStream> & output, const std::vector<GasStream> & birthStreams,bool wordy) const;
		
		// Creation properties
		
		void CreateGrid();
		YieldBracket GetBracket(int id, double mass, double z, bool overhang);
		std::vector<int> SourcePriority;
		void SaveGrid(std::string name);
		void PurityEnforce();
};
