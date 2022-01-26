#include "RemnantPopulation.h"



RemnantPopulation::RemnantPopulation(InitialisedData & data) : Param(data.Param), SNIaYield(data.SNIaYield), NSMYield(data.NSMYield)
{
	int N = Param.Meta.SimulationSteps;
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
void RemnantPopulation::Feed(int timeIndex, RemnantOutput rem)
{
	switch (rem.Type)
	{
		case DormantDwarf:
		{
			DormantWDMass += rem.Mass;
			break;
		}
		case CODwarf:
		{
			double activeWD = rem.Mass * Param.Yield.SNIa_ActiveFraction;
			DormantWDMass += rem.Mass - activeWD;
			double fSN = Param.Yield.SNIa_LongFraction;
			ShortSNIaBuffer[timeIndex] += activeWD * (1.0 - fSN);
			LongSNIaBuffer[timeIndex] += activeWD * fSN;
			break;
		}
		case NeutronStar:
		{
			double activeNS = rem.Mass * Param.Yield.NSM_ActiveFraction;
			DormantNSMass += rem.Mass - activeNS;
			NSMBuffer[timeIndex] += activeNS;
			break;
		}
		case BlackHole:
		{
			BlackHoleMass += rem.Mass;
			break;
		}
		default:
		{
			throw std::runtime_error("You are attempting to feed in a remnant population of an unknown type (" + Param.Yield.ProcessNames[rem.Type] + ") Error!");
		}
		
	}
}
void RemnantPopulation::Decay(int currentTime, GasReservoir & scatteringReservoir, StarEvents & EventRate)
{
	double deltaT = Param.Meta.TimeStep;
	for (int t = 0; t <= currentTime; ++t)
	{
		double timeSince = (currentTime - t) * deltaT;
		if (timeSince >= Param.Yield.SNIa_DelayTime)
		{
			
			double oldSNIa_short = ShortSNIaBuffer[t];
			double oldSNIa_long = LongSNIaBuffer[t];
			double decayShort = exp(-deltaT/ Param.Yield.SNIa_ShortScale);
			double decayLong = exp(-deltaT/Param.Yield.SNIa_LongScale);
			
			
			ShortSNIaBuffer[t] = oldSNIa_short*decayShort;
			LongSNIaBuffer[t] = oldSNIa_long * decayLong;
			
			double snIa_released = oldSNIa_short * (1.0 - decayShort) + oldSNIa_long * (1.0-decayLong);
			int snIa_amount = snIa_released / Param.Yield.SNIa_TypicalMass;
			
			//~ std::cout << "SNIa decay! " << snIa_amount << "  " << ShortSNIaBuffer[t] << "  " << LongSNIaBuffer[t] << "  " << decayShort << "  " << decayLong << std::endl;
			EventRate.SNIa += snIa_amount;
			SNIaYield(scatteringReservoir,snIa_amount,t);
			
		}
	}
}

MassReport RemnantPopulation::Mass()
{
	MassReport rep;
	rep.WD = DormantWDMass;
	rep.NS = DormantNSMass;
	rep.BH = BlackHoleMass;
	
	for (int time = 0; time < Param.Meta.SimulationSteps; ++time)
	{
		rep.WD += ShortSNIaBuffer[time] + LongSNIaBuffer[time];
		rep.NS += NSMBuffer[time];
		//~ std::cout << "The mass in the WD buffer is " << ShortSNIaBuffer[time] << std::endl;
	}
	rep.Total = rep.WD + rep.NS+ rep.BH;
	return rep;
	
}
