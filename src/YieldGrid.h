#pragma once
#include <vector>
#include <math.h>
#include <fstream>
#include "Logger.h"
#include "Options.h"

class YieldRidge
{
	public:
		double Z;
		std::vector<double> Masses;
		std::vector<double> Yields;
		double MinMass;
		double MaxMass;
		
		YieldRidge();
		YieldRidge(double Z, int nPoints);
};

class StellarYield
{
	
	//yield grid for individual element
	public:
		StellarYield();
		StellarYield(Options * opts, int ElementalID);
		double GrabYield(double M, double Z);
	
	
		std::vector<YieldRidge> Ridges;
		void Print();
	private:
		Options * Opts;
		int GridSize;
		std::vector<std::vector<double>> Yield; 
		int Element;
		
		int IndexFromM(double M);
		int IndexFromZ(double Z);
		double MFromIndex(int index);
		double ZFromIndex(int index);
};


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
};
