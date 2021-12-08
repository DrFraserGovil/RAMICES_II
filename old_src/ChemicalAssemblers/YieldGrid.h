#pragma once
#include <vector>
#include <math.h>
#include <fstream>
#include <algorithm>
#include "../GenericFunctions/FileHandler.h"
#include "../Options.h"
#include "../GenericFunctions/Logger.h"

#include "YieldRidge.h"
#include "StellarYield.h"
class YieldGrid
{
	public:		
		YieldGrid();
		YieldGrid(Options * opts);
		
		double Synthesis(int ElementCode, double M, double Z);
		
	private:
		Options * Opts;
		std::vector<StellarYield> Element;
		StellarYield RelicMass;
		StellarYield RelicType;
		
		void ReUseYields();
		void CalculateYields();
		
		void LoadMarigoYields();
		void LoadOrfeoYields();
		void LoadLimongiYields();
		void LoadMaederYields();
		void SaveYields();
};
