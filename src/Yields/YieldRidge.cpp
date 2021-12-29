#include "YieldRidge.h"

YieldRidge::YieldRidge(): Source(Unknown), Z(-10)
{
	
}
YieldRidge::YieldRidge(SourceID source, double z, int nPoints): Source(source), Z(z)
{
	Points.resize(nPoints);
}


double YieldBracket::Interpolate(double mass, double z)
{
	int lowZMass = 0;
	int highZMass = 0;
	while (lowZMass < LowerRidge.Points.size() -1 && LowerRidge.Points[lowZMass+1].Mass <= mass)
	{
		++lowZMass;
	}
	while (highZMass < UpperRidge.Points.size() -1 && UpperRidge.Points[highZMass+1].Mass <= mass)
	{
		++highZMass;
	}
	
	double lowZLowMass = LowerRidge.Points[lowZMass].Mass;
	double lowZHighMass = LowerRidge.Points[lowZMass+1].Mass;
	double lowZInterp = (mass - lowZLowMass)/(lowZHighMass - lowZLowMass);
	
	double lowZValue = LowerRidge.Points[lowZMass].Yield + lowZInterp * (LowerRidge.Points[lowZMass+1].Yield - LowerRidge.Points[lowZMass].Yield);
	
	double highZLowMass = UpperRidge.Points[highZMass].Mass;
	double highZHighMass = UpperRidge.Points[highZMass+1].Mass;
	double highZInterp = (mass - highZLowMass)/(highZHighMass - highZLowMass);
	double highZValue = UpperRidge.Points[highZMass].Yield + highZInterp * (UpperRidge.Points[highZMass+1].Yield - UpperRidge.Points[highZMass].Yield);
	
	double logZInterp;
	if (hasSingle)
	{
		logZInterp = 0;
	}
	else
	{
		double upZ = log10(UpperRidge.Z);
		double downZ = log10(LowerRidge.Z);
		logZInterp = (log10(z) - downZ)/(upZ - downZ);
	}
	return lowZValue + logZInterp * (highZValue - lowZValue);
}
