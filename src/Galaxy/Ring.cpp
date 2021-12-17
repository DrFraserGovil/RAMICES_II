#include "Ring.h"


//! Initialises itself into a primordial state
Ring::Ring(int index, double mass,const IMF_Functor & imf, SLF_Functor & slf, const GlobalParameters & param): Param(param), Width(param.Galaxy.Radius / param.Galaxy.RingCount), Radius((index + 0.5)*param.Galaxy.Radius / param.Galaxy.RingCount), Gas(GasReservoir::Primordial(mass,param)), Stars(param,index,imf,slf), IMF(imf)
{
	RadiusIndex = index;
	//~ PreviousEnrichment.resize(Param.Meta.SimulationSteps);
	//~ PreviousEnrichment[0] = Gas;
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

void Ring::SaveChemicalHistory(int t, std::stringstream & absoluteStreamCold, std::stringstream & logarithmicStreamCold, std::stringstream & absoluteStreamHot, std::stringstream & logarithmicStreamHot)
{
	std::string basic = "";
	if (t == 0 && RadiusIndex == 0)
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
	basic += std::to_string(t) + ", " + std::to_string(RadiusIndex);
	
	absoluteStreamCold << basic;
	logarithmicStreamCold  << basic;
	absoluteStreamHot   << basic;
	logarithmicStreamHot   << basic;
	
	const std::vector<GasStream> & target = Gas.GetHistory(t);
	
	std::vector<std::vector<double>> coldAbundances(ProcessCount + 1, std::vector<double>(ElementCount,0.0));
	std::vector<std::vector<double>> hotAbundances(ProcessCount + 1, std::vector<double>(ElementCount,0.0));

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

			coldAbundances[0][e] += cold;
			hotAbundances[0][e] += hot;
			coldAbundances[p+1][e] = cold/processCold;
			hotAbundances[p+1][e] = hot/processHot;
		}
	} 
	for (int e = 0; e < ElementCount; ++e)
	{
		coldAbundances[0][e] /= coldMass;
		hotAbundances[0][e] /= hotMass;
	}
	
	for (int p = 0; p < ProcessCount + 1; ++p)
	{
		double logHydrogen_cold = log10(coldAbundances[p][Hydrogen] / Param.Element.SolarAbundances[Hydrogen]);
		double logHydrogen_hot = log10(hotAbundances[p][Hydrogen] / Param.Element.SolarAbundances[Hydrogen]);
		for (int e = 0; e < ElementCount; ++e)
		{
			absoluteStreamCold << ", " << coldAbundances[p][e];
			absoluteStreamHot << ", " << hotAbundances[p][e];
			
			
			double logValueCold = log10(coldAbundances[p][e] / Param.Element.SolarAbundances[e]) - logHydrogen_cold;
			double logValueHot = log10(hotAbundances[p][e]/Param.Element.SolarAbundances[e]) - logHydrogen_hot;
			
			if (std::isnan(logValueCold) || std::isinf(logValueCold))
			{
				logarithmicStreamCold << ", - ";
			}
			else
			{
				logarithmicStreamCold << ", " << logValueCold;
			}
			if (std::isnan(logValueHot) || std::isinf(logValueHot))
			{
				logarithmicStreamHot << ", - ";
			}
			else
			{
				logarithmicStreamHot << ", " << logValueHot;
			}
		}
	}
	absoluteStreamCold << "\n";
	logarithmicStreamCold  << "\n";
	absoluteStreamHot   << "\n";
	logarithmicStreamHot   << "\n";
}
