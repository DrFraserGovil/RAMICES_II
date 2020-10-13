#pragma once
#include <vector>
#include <algorithm>
#include "../Options.h"
#include "../GenericFunctions/Logger.h"

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


YieldRidge MergeRidges(std::vector<YieldRidge> candidates, Options * opts);
