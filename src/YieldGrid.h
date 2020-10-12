#pragma once
#include <vector>
#include <math.h>
#include <fstream>
#include <algorithm>
#include "Logger.h"
#include "Options.h"

struct YieldPoint
{
	double Mass;
	double Yield;
	YieldPoint(double m, double y)
	{
		Mass = m;
		Yield = y;
	}
};



class YieldRidge
{
	public:
		double SourceID;
		double Z;
		std::vector<YieldPoint> Points;


		bool Merged;
		
		YieldRidge();
		YieldRidge(int id, double Z, int nPoints);
		
		double MassInterp(double mass, Options * opts);
};

struct YieldBracket
{
	bool isEnclosed;
	bool hasSingle;
	YieldRidge UpperRidge;
	YieldRidge LowerRidge;
	
	YieldBracket()
	{
		isEnclosed = false;
		hasSingle = false;
	}
	YieldBracket(YieldRidge r1, YieldRidge r2)
	{
		isEnclosed = true;
		UpperRidge = r1;
		LowerRidge = r2;
	}
	YieldBracket(YieldRidge r1)
	{
		isEnclosed = false;
		hasSingle = true;
		LowerRidge = r1;
		UpperRidge = r1;
	}
	
};

class StellarYield
{
	
	//yield grid for individual element
	public:
		StellarYield();
		StellarYield(Options * opts, int ElementalID);
		double GrabYield(double M, double Z);
	
	
		std::vector<YieldRidge> Ridges;
		
		void PrepareGrids();
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
		void FilterRidges();
		void InterpolateGrid();
		void SmoothGrid();
		YieldBracket FindBracket(double m, double z);
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
