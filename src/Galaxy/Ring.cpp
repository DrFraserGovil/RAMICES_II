#include "Ring.h"

const double pi = 3.141592654;
//! Initialises itself into a primordial state
Ring::Ring(int index, double mass,InitialisedData & data): Data(data), Param(data.Param), Width(data.Param.Galaxy.RingWidth[index]), Radius(data.Param.Galaxy.RingRadius[index]), Gas(GasReservoir::Primordial(mass,data.Param)), Stars(index,data)
{
	RadiusIndex = index;
	Area = 2 * pi * Radius * Width;
	//~ PreviousEnrichment.resize(Param.Meta.SimulationSteps);
	//~ PreviousEnrichment[0] = Gas;
	Data.Log("\tRing " + std::to_string(index) + " initialised\n",3);
}

double Ring::Mass()
{
	MassReport Mrr = Stars.DeadMass();
	return Gas.Mass() + Stars.AliveMass() + Mrr.Total /1e9; 
}


void Ring::TimeStep(int t)
{
	MetCheck("Start of internal loop");
	Cool();
	//~ std::cout << "\tCooled " << Mass() << std::endl;
	MakeStars();
	//~ std::cout << "\tFormed "  << Mass()<< std::endl;
	UpdateMemory(t);
	KillStars(t);
	//~ std::cout << "\tKilled "  << Mass() << std::endl;
	
	
	UpdateMemory(t);
	//~ std::cout << "\tSaved "  << Mass() << std::endl;
	
	MetCheck("End of internal loop");
}

void Ring::MakeStars()
{
	//~ std::cout << "Ring " << RadiusIndex << " forming stars " << Gas.Mass() + Stars.AliveMass() << "  " << Gas.ColdMass() << "  " << Gas.HotMass() << std::endl;
	Stars.Form(Gas);
	//~ std::cout << "\t " << Gas.Mass() + Stars.AliveMass() << "  " << Gas.ColdMass() << "  " << Gas.HotMass() << std::endl;
}
void Ring::KillStars(int time)
{
	Stars.Death(time,Gas);
}

void Ring::Cool()
{
	Gas.PassiveCool(Param.Meta.TimeStep,false);
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
			std::string headers = "Time, RingIndex, RingRadius";
			for (int p = -1; p < ProcessCount; ++p)
			{
				std::string processName;
				if (p > -1)
				{
					 processName = Param.Yield.ProcessNames[p];
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
	basic += std::to_string(t*Param.Meta.TimeStep) + ", " + std::to_string(RadiusIndex) + ", " + std::to_string(Radius);
	
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
		ColdBuffer[0][e] /= (coldMass+1e-88);
		HotBuffer[0][e] /= (hotMass+1e-88);
	}
	

	for (int p = 0; p < ProcessCount + 1; ++p)
	{
		for (int e = 0; e < ElementCount; ++e)
		{
			double coldCorrect = coldMass;
			double hotCorrect = hotMass;
			if (p > 0)
			{
				coldCorrect = target[p-1].ColdMass();
				coldCorrect = target[p-1].HotMass();
			}
			neatLog(ColdBuffer[p][e] * coldCorrect, absoluteStreamCold);
			neatLog(HotBuffer[p][e] * hotCorrect, absoluteStreamHot);
					
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


void Ring::MetCheck(const std::string & location)
{
	if (Gas.Mass() < 0)
	{
		std::cout << "\n\nThe gas in Ring " << RadiusIndex << " has negative mass -- something has gone very wrong!" << std::endl;
		exit(5);
	}
	double z = Gas.Metallicity();
	if (z < 0)
	{
		std::cout << "\n\nThe gas in Ring " << RadiusIndex << " had a negative metallicity at " << location << "\n Critical Error!" << std::endl;
		std::cout << "The components of the ring are: \n";
		for (int p = 0; p < ProcessCount; ++p)
		{
			SourceProcess proc = (SourceProcess)p;
			for (int e = 0; e < ElementCount; ++e)
			{
				ElementID elem = (ElementID)e;
				std::cout << Gas[proc].Cold(elem) << "\t";
			}
			std::cout << "\n";
		}
		exit(5);
		
	}
	
}
