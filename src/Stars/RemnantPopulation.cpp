#include "RemnantPopulation.h"


RemnantPopulation::RemnantPopulation(const InitialisedData & data) : Param(data.Param)
{
	int N = param.Meta.SimulationSteps;
	ShortSNIaBuffer = std::vector<double>(N);
	LongSNIaBuffer = std::vector<double>(N);

	NSMBuffer = std::vector<double>(N);
	BlackHoleMass = 0;
	DormantWDMass = 0;
	DormantNSMass = 0;	
}

void RemnantPopulation::Feed(int timeIndex, double bhMass, double wdMass, double nsMass)
{
	BlackHoleMass += bhMass;
	
	double activeWD = wdMass * Param.Yield.SNIa_ActiveFraction;
	double activeNS = nsMass * Param.Yield.NSM_ActiveFraction;
	DormantWDMass += wdMass - activeWD;
	DormantNSMass += nsMass - activeNS;
	
	NSMBuffer[timeIndex] = activeNS;
	double fSN = Param.Yield.SNIa_LongFraction;
	ShortSNIaBuffer[timeIndex] = activeWD * (1.0 - fSN);
	LongSNIaBuffer[timeIndex] = activeWD * fSN;
	
}

void RemnantPopulation::Decay(int currentTime);
