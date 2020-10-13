#pragma once
#include <vector>
#include <algorithm>
#include <fstream>
#include <math.h>
#include "YieldRidge.h"
#include "../Options.h"
#include "../GenericFunctions/Logger.h"
class StellarYield
{
	
	//yield grid for individual element
	public:
		StellarYield();
		StellarYield(Options * opts, int ElementalID);
		double GrabYield(double M, double Z);
		double GrabYield(int mIndex, int zIndex);
	
		std::vector<YieldRidge> Ridges;
		
		void PrepareGrids();
		void PrintRidges();
		int GridSize;
		double MFromIndex(int index);
		double ZFromIndex(int index);
	private:
		Options * Opts;
		
		std::vector<std::vector<double>> Yield; 
		int Element;
		
		int IndexFromM(double M);
		int IndexFromZ(double Z);
		
		void FilterRidges();
		void InterpolateGrid();
		void SmoothGrid();
		YieldBracket FindBracket(double m, double z);
};
