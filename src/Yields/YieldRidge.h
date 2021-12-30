#pragma once

#include "../Parameters/GlobalParameters.h"
struct YieldPoint
{
	double Mass;
	double Yield;
	YieldPoint(double m, double y): Mass(m), Yield(y)
	{

	}
	YieldPoint()
	{
		YieldPoint(0,0);
	}
};



class YieldRidge
{
	public:
		SourceID Source;
		double Z;
		std::vector<YieldPoint> Points;
		YieldRidge();
		YieldRidge(SourceID source, double z, int nPoints);

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
	YieldBracket(YieldRidge r1, YieldRidge r2): UpperRidge(r1), LowerRidge(r2)
	{
		isEnclosed = true;
		hasSingle = false;
	}
	YieldBracket(YieldRidge r1): UpperRidge(r1), LowerRidge(r1)
	{
		isEnclosed = true;
		hasSingle = true;
	}
	
	double Interpolate(double mass, double z);
};
