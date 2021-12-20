#include "GasReservoir.h"

GasReservoir::GasReservoir() : Param(GlobalParameters())
{
	Components.resize(ProcessCount);
	//make sure each stream has its own label correctly
	for (int i = 0; i < ProcessCount; ++i)
	{
		Components[i].Source = (SourceProcess)i;
	}
	ComponentHistory.resize(Param.Meta.SimulationSteps+1);
	for (int i = 0; i < Param.Meta.SimulationSteps; ++i)
	{
		ComponentHistory[i].resize(ProcessCount);
	}
}
GasReservoir::GasReservoir(const GlobalParameters & param): Param(param)
{
	Components.resize(ProcessCount);
	//make sure each stream has its own label correctly
	for (int i = 0; i < ProcessCount; ++i)
	{
		Components[i].Source = (SourceProcess)i;
	}
	ComponentHistory.resize(Param.Meta.SimulationSteps+1);
	for (int i = 0; i < Param.Meta.SimulationSteps; ++i)
	{
		ComponentHistory[i].resize(ProcessCount);
	}
}

GasReservoir GasReservoir::Primordial(double mass, const GlobalParameters & param)
{
	GasReservoir prim(param);
	
	Gas g = Gas::Primordial(mass);
	double fh = param.Galaxy.PrimordialHotFraction;
	GasStream primordialStream = GasStream(SourceProcess::Primordial,g,fh);
	
	prim.Absorb(primordialStream);
	return prim;
	
}
const GasStream & GasReservoir::operator[](SourceProcess source) const
{
	return Components[source];
}
GasStream & GasReservoir::operator[](SourceProcess source)
{
	Components[source].Dirty(); //non-constant reference so have to alert the Stream that somebody has been touching its stuff
	return Components[source];
}


double GasReservoir::Mass()
{
	double sum = 0;
	for (int i = 0; i < ProcessCount; ++i)
	{
		sum += Components[i].Mass();
	}
	return sum;
}
double GasReservoir::ColdMass()
{
	double sum = 0;
	for (int i = 0; i < ProcessCount; ++i)
	{
		sum += Components[i].ColdMass();
	}
	return sum;
}
double GasReservoir::HotMass()
{
	double sum = 0;
	for (int i = 0; i < ProcessCount; ++i)
	{
		sum += Components[i].HotMass();
	}
	return sum;
}

void GasReservoir::Absorb(const GasReservoir & givingGas)
{
	for (int i = 0; i < ProcessCount; ++i)
	{
		SourceProcess source = (SourceProcess)i;
		Absorb(givingGas[source]);
	}
}

void GasReservoir::Absorb(const std::vector<GasStream> & givingGas)
{
	for (int i = 0; i < ProcessCount; ++i)
	{
		SourceProcess source = (SourceProcess)i;
		Absorb(givingGas[source]);
	}
}
void GasReservoir::AbsorbMemory(int t, const GasStream & input)
{
	ComponentHistory[t][input.Source].Absorb(input);
}

void GasReservoir::Deplete(double amountToLose)
{
	double totalMass = Mass();
	
	amountToLose = std::min(amountToLose,totalMass);
	double contFraction = amountToLose / Mass();
	for (int i = 0; i < ProcessCount; ++i)
	{
		if (Components[i].Mass() > 0)
		{
			
			double componentContribution =  Components[i].Mass() * contFraction;
			Components[i].Deplete(componentContribution);
		}
	}
	
}
void GasReservoir::Deplete(double amountToLose_Cold, double amountToLose_Hot)
{
	double Mc = ColdMass();
	double Mh = HotMass();
	
	amountToLose_Cold = std::min(amountToLose_Cold,Mc);
	amountToLose_Hot = std::min(amountToLose_Hot,Mh);
	
	double coldLoss = amountToLose_Cold/Mc;
	double hotLoss = amountToLose_Hot/Mh;
	for (int i = 0; i < ProcessCount; ++i)
	{
		if (Components[i].Mass() > 0)
		{
			
			double coldMass =  Components[i].ColdMass() * coldLoss;
			double hotMass = Components[i].HotMass() * hotLoss;
			Components[i].Deplete(coldMass,hotMass);
		}
	}
	
}


void GasReservoir::Absorb(const GasStream & givingGas)
{	
	SourceProcess source = givingGas.Source;
	Components[source].Absorb(givingGas);
}

void GasReservoir::Heat(double amountToHeat)
{
	double heatingFraction = amountToHeat / ColdMass();
	for (int i = 0; i < ProcessCount; ++i)
	{
		double componentHeat = heatingFraction * Components[i].ColdMass();
		Components[i].Heat(componentHeat);
	}
}

void GasReservoir::PassiveCool(double dt)
{
	double tau = Param.Thermal.GasCoolingTimeScale;
	double coolingFraction = (1.0 - exp(-dt/tau)); //basic exponential cooling law
	for (int i = 0; i < ProcessCount; ++i)
	{
		double componentCool = coolingFraction * Components[i].HotMass();
		Components[i].Cool(componentCool);
	}
	

}

GasStream GasReservoir::AccretionStream(double amountToLose)
{
	double initMass = ColdMass();
	amountToLose = std::min(amountToLose,initMass); //Can't lose more than I have!
	GasStream output(Accreted);
	double lossFraction = amountToLose / initMass;
	for (int i = 0; i < ProcessCount; ++i)
	{
		
		for (int j = 0; j < ElementCount; ++j)
		{
			ElementID elem = (ElementID)j;
			double extract = lossFraction * Components[i].Cold(elem);
			Components[i].Cold(elem) -= extract;
			output.Cold(elem) += extract; 
		}	
	}
	output.ColdMass(); //force computation of masses
	return output;
}

void GasReservoir::TransferFrom(GasReservoir & givingGas, double massToMove)
{
	double lossFraction = std::min(1.0,massToMove/givingGas.Mass());
	
	
	
	for (int i = 0; i < ProcessCount; ++i)
	{
		SourceProcess proc = (SourceProcess)i;
		for (int j = 0; j < ElementCount; ++j)
		{
			ElementID elem = (ElementID)j;
			double currentCold = givingGas[proc].Cold(elem);
			double currentHot = givingGas[proc].Hot(elem);
			double extractCold = lossFraction * currentCold;
			
			double extractHot = lossFraction * currentHot;
			givingGas[proc].Cold(elem) -= extractCold;
			givingGas[proc].Hot(elem) -= extractHot;
			
			Components[proc].Cold(elem) += extractCold; 
			Components[proc].Hot(elem) += extractHot;		
		}	
	}

}

void GasReservoir::PrintSelf()
{
	for (int c = 0; c < ProcessCount; ++c)
	{
		SourceProcess source = (SourceProcess)c;
		auto stream = Components[source];
		bool headerPrinted = false;
		
		for (int i = 0; i < ElementCount; ++i)
		{
			ElementID elem = (ElementID)i;
			if (stream.Cold(elem)!= 0 || stream.Hot(elem) != 0)
			{
				if (!headerPrinted)
				{
					std::cout << "From process " << c << ": \n";
					headerPrinted = true;
				}
				std::cout << "\t" << Param.Element.ElementNames[elem] << " Cold = " << stream.Cold(elem) << "  Hot = " << stream.Hot(elem) << std::endl;
			}
		}
	}
}
	
double GasReservoir::Metallicity()
{
	//~ std::cout << "I am requesting a metallicity call" << std::endl;
	double Mz = 0;
	double M = 0;
	for (int i = 0; i < ProcessCount; ++i)
	{
		Components[i].Mass();
		const GasStream & stream = Components[i];
		M += stream.Cold().Mass();
		Mz += stream.Cold(Metals);
	}
	return Mz/M;
}
void GasReservoir::UpdateMemory(int t)
{
	ComponentHistory[t] = Components;
}
const std::vector<GasStream> & GasReservoir::GetHistory(int t)
{
	//force update to mass calculation, as cannot be done with consts!
	for (int  p =0; p < ProcessCount; ++p)
	{
		ComponentHistory[t][p].ColdMass();
	}
	return ComponentHistory[t];
}

void GasReservoir::WipeMemoryUpTo(int t)
{
	for (int time = 0; time < t; ++time)
	{
		for (int p = 0; p < ProcessCount; ++p)
		{
			for (int e = 0; e < ElementCount; ++e)
			{
				ElementID elem = (ElementID)e;
				ComponentHistory[time][p].Cold(elem) = 0;
				ComponentHistory[time][p].Hot(elem) = 0;
			}
		}
	}
}
