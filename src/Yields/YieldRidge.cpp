#include "YieldRidge.h"

YieldRidge::YieldRidge(): Source(Unknown), Z(-10)
{
	
}
YieldRidge::YieldRidge(SourceID source, double z, int nPoints): Source(source), Z(z)
{
	Points.resize(nPoints);
}

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}
double YieldBracket::Interpolate(double mass, double z)
{
	int lowZMass = 0;
	int highZMass = 0;
	while (lowZMass < LowerRidge.Points.size() -2 && LowerRidge.Points[lowZMass+1].Mass <= mass)
	{
		++lowZMass;
	}
	while (highZMass < UpperRidge.Points.size() -2 && UpperRidge.Points[highZMass+1].Mass <= mass)
	{
		++highZMass;
	}
	
	double decayScaleLength = 3;
	
	
	double lowZLowMass = LowerRidge.Points[lowZMass].Mass;
	double lowZHighMass = LowerRidge.Points[lowZMass+1].Mass;
	
	double lowerDecayFactor = 1;
	double dist = mass - lowZLowMass;
	if (lowZLowMass > mass)
	{
		double d = abs(mass - lowZLowMass);
		lowerDecayFactor = exp(-d/decayScaleLength); 
		dist = lowZHighMass - lowZLowMass - d*lowerDecayFactor;
	}
	if (lowZHighMass < mass)
	{
		double d = abs(mass - lowZHighMass);
		lowerDecayFactor = exp(-d/decayScaleLength); 
		dist = lowZHighMass - lowZLowMass + d*lowerDecayFactor;
	}
	double lowZInterp = dist /(lowZHighMass - lowZLowMass);
	
	double lowZValue = LowerRidge.Points[lowZMass].Yield + lowZInterp * (LowerRidge.Points[lowZMass+1].Yield - LowerRidge.Points[lowZMass].Yield);
	//~ lowZValue *= lowerDecayFactor;
	if (sgn(LowerRidge.Points[lowZMass].Yield) == sgn(LowerRidge.Points[lowZMass+1].Yield) && sgn(LowerRidge.Points[lowZMass].Yield) != sgn(lowZValue))
	{
		lowZValue = 0;
	}
	
	double highZLowMass = UpperRidge.Points[highZMass].Mass;
	double highZHighMass = UpperRidge.Points[highZMass+1].Mass;
	
	
	
	double upperDecayFactor = 1;
	dist = (mass - highZLowMass);
	if (highZLowMass > mass)
	{
		double d = abs(mass - highZLowMass);
		upperDecayFactor = exp(-d/decayScaleLength); 
		dist = highZHighMass - highZLowMass - d*upperDecayFactor;
	}
	if (highZHighMass < mass)
	{
		double d = abs(mass - highZHighMass);
		upperDecayFactor = exp(-d/decayScaleLength); 
		dist = highZHighMass - highZLowMass + d*upperDecayFactor;
	}
	
	double highZInterp = dist /(highZHighMass - highZLowMass);
	double highZValue = UpperRidge.Points[highZMass].Yield + highZInterp * (UpperRidge.Points[highZMass+1].Yield - UpperRidge.Points[highZMass].Yield);
	
	if (sgn(UpperRidge.Points[highZMass].Yield) == sgn(UpperRidge.Points[highZMass+1].Yield) && sgn(UpperRidge.Points[highZMass].Yield) != sgn(highZValue))
	{
		highZValue = 0;
	}
	
	double logZInterp;
	double upZ = 0;
	double downZ = 0;
	if (hasSingle)
	{
		logZInterp = 0;
	}
	else
	{
		upZ = log10(UpperRidge.Z) ;
		downZ = log10(LowerRidge.Z) ;
		logZInterp = (log10(z) - downZ)/(upZ - downZ);
	}
	
	double interpolatedValue =  lowZValue + logZInterp * (highZValue - lowZValue);
	
	return interpolatedValue;
}
