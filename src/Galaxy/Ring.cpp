#include "Ring.h"


//! Initialises itself into a primordial state
Ring::Ring(int index, double mass,InitialisedData & data): Data(data), Param(data.Param), Width(data.Param.Galaxy.Radius / data.Param.Galaxy.RingCount), Radius((index + 0.5)*data.Param.Galaxy.Radius / data.Param.Galaxy.RingCount), Gas(GasReservoir::Primordial(mass,data.Param)), Stars(index,data)
{
	RadiusIndex = index;
	//~ PreviousEnrichment.resize(Param.Meta.SimulationSteps);
	//~ PreviousEnrichment[0] = Gas;
	Data.Log("\tRing " + std::to_string(index) + " initialised\n",3);
}

double Ring::Mass()
{
	
}

void Ring::MakeStars()
{
	Stars.Form(Gas);
}
void Ring::KillStars(int time)
{
	Stars.Death(time);
}

void Ring::Cool()
{
	Gas.PassiveCool(Param.Meta.TimeStep);
}
void Ring::UpdateMemory(int t)
{
	Gas.UpdateMemory(t);
}

void neatLog(double value, std::stringstream & stream)
{
	if (isinf(value) || isnan(value))
	{
		stream << ",-";
	}
	else
	{
		stream << ", " << value;
	}
}

void Ring::SaveChemicalHistory(int t, std::stringstream & absoluteStreamCold, std::stringstream & logarithmicStreamCold, std::stringstream & absoluteStreamHot, std::stringstream & logarithmicStreamHot)
{
	std::string basic = "";
	if (t == 0)
	{
		HotBuffer = std::vector<std::vector<double>>(ProcessCount + 1, std::vector<double>(ElementCount,0.0));
		ColdBuffer = std::vector<std::vector<double>>(ProcessCount + 1, std::vector<double>(ElementCount,0.0));
		
		if (RadiusIndex == 0)
		{
			std::string headers = "TimeIndex, RingIndex";
			for (int p = -1; p < ProcessCount; ++p)
			{
				std::string processName;
				if (p > -1)
				{
					 processName = Param.Element.ProcessNames[p];
				}
				else
				{
					processName = "Total";
				}
				for (int e = 0; e < ElementCount; ++e)
				{
					std::string elementName = Param.Element.ElementNames[e];
				
				
					headers += ", " + processName+ "_" + elementName;
				}
			}
			basic = headers + "\n";
		}
	}
	basic += std::to_string(t) + ", " + std::to_string(RadiusIndex);
	
	absoluteStreamCold << basic;
	logarithmicStreamCold  << basic;
	absoluteStreamHot   << basic;
	logarithmicStreamHot   << basic;
	
	const std::vector<GasStream> & target = Gas.GetHistory(t);
	
	

	double coldMass = 0;
	double hotMass = 0;
	for (int p = 0; p < ProcessCount; ++p)
	{
		double processCold = target[p].ColdMass();
		double processHot = target[p].HotMass();
		
		coldMass += processCold;
		hotMass += processHot;
		for (int e = 0; e < ElementCount; ++e)
		{
			ElementID elem = (ElementID)e;
			double cold = target[p].Cold(elem);
			double hot = target[p].Hot(elem);

			if (p == 0)
			{
				ColdBuffer[p][e] = 0;
				HotBuffer[p][e] = 0;
			}
			ColdBuffer[0][e] += cold;
			HotBuffer[0][e] += hot;
			ColdBuffer[p+1][e] = cold/processCold;
			HotBuffer[p+1][e] = hot/processHot;
		}
	} 
	for (int e = 0; e < ElementCount; ++e)
	{
		ColdBuffer[0][e] /= coldMass;
		HotBuffer[0][e] /= hotMass;
	}
	
	for (int p = 0; p < ProcessCount + 1; ++p)
	{
		for (int e = 0; e < ElementCount; ++e)
		{
			
			neatLog(ColdBuffer[p][e], absoluteStreamCold);
			neatLog(HotBuffer[p][e], absoluteStreamHot);
					
			double logValueCold = log10(ColdBuffer[p][e] / Param.Element.SolarAbundances[e]);
			double logValueHot = log10(HotBuffer[p][e]/Param.Element.SolarAbundances[e]);
			
			neatLog(logValueCold,logarithmicStreamCold);
			neatLog(logValueHot,logarithmicStreamHot);
	
		}
	}
	absoluteStreamCold << "\n";
	logarithmicStreamCold  << "\n";
	absoluteStreamHot   << "\n";
	logarithmicStreamHot   << "\n";
}
