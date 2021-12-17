#pragma once

#include <vector>
#include "../Parameters/GlobalParameters.h"
#include "../Gas/GasReservoir.h"
#include "RemnantPopulation.h"
#include "StellarPopulation.h"
#include <sstream>
#include "IMF.h"
#include "SLF.h"
class StarReservoir
{
	public:
		StarReservoir(const GlobalParameters & Param, int parentRing, const IMF_Functor & imf, SLF_Functor & slf);
		
		double Mass();
		
		void Form(GasReservoir & gas);
		void Death(int currentTime);
		void PrintStatus(int t);
		
	private:
		std::vector<StellarPopulation> Population;
		RemnantPopulation Remnants;
		const GlobalParameters & Param;
		double SFR_GasLoss(double surfaceArea);
		const int ParentRing;
		double ParentArea;
		double Temp_Mass;
		const IMF_Functor & IMF;
		SLF_Functor & SLF;
		int PopulationIndex;
		
		
};
