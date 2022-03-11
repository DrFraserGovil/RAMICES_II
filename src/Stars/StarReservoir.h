#pragma once

#include <vector>
#include "../Parameters/InitialisedData.h"
#include "../Gas/GasReservoir.h"
#include "RemnantPopulation.h"
#include "IsochroneTracker.h"
#include "StellarPopulation.h"
#include "IsochroneTracker.h"
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
		void Form(GasReservoir & gas, GasReservoir & igm);
		void Death(int currentTime);
		void PrintStatus(int t);
		const std::vector<GasStream> & YieldsFrom(int t);
		void SaveEventRate(int t, std::stringstream & output);
		void AssignMagnitudes();
			std::vector<GasReservoir> YieldOutput;
	private:
		
		RemnantPopulation Remnants;
		
		double SFR_GasLoss(double coldMass, double hotMass,double ejectFactor);
		const int ParentRing;
		double ParentArea;
		double Temp_Mass;
		const IMF_Functor & IMF;
		SLF_Functor & SLF;
		int PopulationIndex;
		
		std::vector<StarEvents> EventRate;
	
		
		
		InitialisedData & Data;
		const GlobalParameters & Param;
};
