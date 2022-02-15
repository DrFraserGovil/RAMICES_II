#pragma once

#include <vector>
#include "../Parameters/InitialisedData.h"
#include "../Gas/GasReservoir.h"
#include "RemnantPopulation.h"
#include "IsochroneTracker.h"
#include "StellarPopulation.h"
#include <sstream>
#include "IMF.h"
#include "SLF.h"
#include "StarEvents.h"
class StarReservoir
{
	public:
		StarReservoir(int parentRing, InitialisedData & data);
		std::vector<StellarPopulation> Population;
		double AliveMass();
		MassReport DeadMass();
		
		
		void Observations();
		void Form(GasReservoir & gas);
		void Death(int currentTime, GasReservoir & birthGas);
		void PrintStatus(int t);
		const std::vector<GasStream> & YieldsFrom(int t);
		void SaveEventRate(int t, std::stringstream & output);
		void StealFrom(const StellarPopulation & mark, double fraction);
	private:
		
		RemnantPopulation Remnants;
		std::vector<StellarPopulation> MigratedPopulation;
		InitialisedData & Data;
		const GlobalParameters & Param;
		double SFR_GasLoss(double surfaceArea);
		const int ParentRing;
		double ParentArea;
		double Temp_Mass;
		const IMF_Functor & IMF;
		SLF_Functor & SLF;
		int PopulationIndex;
		
		GasReservoir YieldOutput;
		std::vector<StarEvents> EventRate;
};
