#include "SimpleYield.h"


SimpleYield::SimpleYield(const GlobalParameters & param, YieldProcess yieldprocess): Param(param), Process(param.Yield.ProcessTypes[yieldprocess])
{
	if (param.Meta.Verbosity > 0)
	{
		std::cout << "\t" << Param.Yield.ProcessNames[Process] << " yield grid initialising...." << std::flush;
	}
	switch(yieldprocess)
	{
		case NSM:
		{
			NSM_Initialise();
			break;
		}
		case SNIa:
		{
			SNIa_Initialise();
			break;
		}
		default:
		{
			throw std::runtime_error("You have tried to initialise a yield grid for which there is no rule to create - ID = " +std::to_string(Process) + "...I am forced to quit");
		}
	}
	
	
	if (param.Meta.Verbosity > 0)
	{
		std::cout << "complete" << std::endl;
	}
}

void SimpleYield::operator()(GasReservoir & scatteringReservoir, double nObjects) const
{
	RemnantInject(scatteringReservoir,nObjects);
}

void SimpleYield::RemnantInject(GasReservoir & scatteringReservoir, double nObjects) const
{
	GasStream TempStream(Process);
	for (int e = 0; e < ElementCount; ++e)
	{
		ElementID elem = (ElementID)e;
		double amountInjected = nObjects * Grid[elem] / 1e9;
		TempStream.Cold(elem) = amountInjected * (1.0 - hotInjectionFraction);
		TempStream.Hot(elem) = amountInjected * hotInjectionFraction;
	}
	//~ std::cout << "Sending " <<TempStream.Mass() << " from " << nObjects << std::endl;
	scatteringReservoir.Absorb(TempStream);
	
	//~ std::cout << "Injected " << scatteringReservoir[Process].Cold(Europium) << "  " << scatteringReservoir[Process].Hot(Europium) << std::endl;
}
void SimpleYield::SNIa_Initialise()
{
	hotInjectionFraction = Param.Thermal.HotInjection_SNIa;
	
	Grid = std::vector<double>(ElementCount,0.0);
	
	Grid[Metals] = Param.Yield.SNIa_TypicalMass;
	Grid[Iron] = 0.77;
	Grid[Oxygen] = 0.133;
	Grid[Magnesium] = 0.0158;
	Grid[Carbon] = 0.0508;
	Grid[Silicon] = 0.142;
	Grid[Calcium] = 0.0181;
	//~ Grid[Europium] = 1e-3;
}
void SimpleYield::NSM_Initialise()
{
	hotInjectionFraction = Param.Thermal.HotInjection_NSM;
}
