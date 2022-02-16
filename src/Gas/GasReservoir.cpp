#include "GasReservoir.h"

GasReservoir::GasReservoir() : Param(GlobalParameters())
{
	Components.resize(ProcessCount);
	//make sure each stream has its own label correctly
	for (int i = 0; i < ProcessCount; ++i)
	{
		Components[i].Source = (SourceProcess)i;
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
}

GasReservoir GasReservoir::Primordial(double mass, const GlobalParameters & param)
{
	GasReservoir prim(param);
	
	Gas g = Gas::Primordial(mass);
	double fh = param.Galaxy.PrimordialHotFraction;
	GasStream primordialStream = GasStream(SourceProcess::Accreted,g,fh);
	
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

void GasReservoir::Absorb(const std::vector<GasStream> & givingGas, double fraction)
{
	for (int i = 0; i < ProcessCount; ++i)
	{
		SourceProcess source = (SourceProcess)i;
		
		
		Absorb(givingGas[source],fraction);
	}
	
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

void GasReservoir::Absorb(const GasStream & givingGas, double fraction)
{	
	SourceProcess source = givingGas.Source;
	Components[source].Absorb(givingGas,fraction);
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

void GasReservoir::PassiveCool(double dt, bool isIGM)
{
	//~ double tau = Param.Thermal.GasCoolingTimeScale;
	//~ double coolingFraction = (1.0 - exp(-dt/tau)); //basic exponential cooling law
	//~ for (int i = 0; i < ProcessCount; ++i)
	//~ {
		//~ double componentCool = coolingFraction * Components[i].HotMass();
		//~ Components[i].Cool(componentCool);
		
	//~ }
	
	double hotMass = HotMass();
	double gasMass = hotMass + ColdMass();
	//~ std::cout << "\t\tStarted with " << gasMass << " ( C = " << gasMass - hotMass << "  H = " << hotMass << ")" << std::endl;
	double n = Param.Thermal.CoolingPower;
	double dormantPower = pow(Param.Thermal.DormantHotFraction,n);
	if (isIGM)
	{
		dormantPower = 1e-99;
	}
	double ddt = dt/Param.Thermal.NumericalResolution;
	for (int i = 0; i < Param.Thermal.NumericalResolution; ++i)
	{
		double dMh = gasMass/ Param.Thermal.GasCoolingTimeScale * (dormantPower - pow(hotMass/gasMass,n));
		double delta = std::max(std::min(dMh * ddt,hotMass),hotMass-gasMass);
		hotMass += delta;
		hotMass = std::max(0.0,hotMass);
	}
	
	double cooledAmount = HotMass() - hotMass;
	if (cooledAmount > 0 && HotMass() > 0)
	{
		double cooledFraction = cooledAmount / HotMass();
		for (int i = 0; i < ProcessCount; ++i)
		{
			double componentCool = cooledFraction * Components[i].HotMass();
			Components[i].Cool(componentCool);
		}
	}
	else
	{
		double heatedAmount = -cooledAmount;

		double heatedFraction = std::min(1.0,heatedAmount/ColdMass());
		for (int i = 0; i < ProcessCount; ++i)
		{
			double componentHeat = heatedFraction * Components[i].ColdMass();
			Components[i].Heat(componentHeat);
		}
	}
	//~ std::cout << "\t\tEnded with " << Mass() << " ( C = " << ColdMass() << "  H = " << HotMass() << ")" << std::endl;
}

GasStream GasReservoir::AccretionStream(double amountToLose)
{
	double initMass = ColdMass();
	if (amountToLose > initMass)
	{
		std::cout << "You have just attempted to accrete gas from a reservoir exceeding the mass of the reservoir. This is likely because your IGM mass is too low." << std::endl;
		exit(5);
	}
	
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
void GasReservoir::TransferColdFrom(GasReservoir & givingGas, double massToMove)
{
	if (givingGas.ColdMass() < massToMove)
	{
		std::cout << "Error - you are trying to transfer more gas from one reservoir to another than is available" << std::endl;
		exit(6); 
	}
	double lossFraction = std::min(1.0,massToMove/givingGas.ColdMass());
	
	
	
	for (int i = 0; i < ProcessCount; ++i)
	{
		SourceProcess proc = (SourceProcess)i;
		for (int j = 0; j < ElementCount; ++j)
		{
			ElementID elem = (ElementID)j;
			double currentCold = givingGas[proc].Cold(elem);
			double extractCold = lossFraction * currentCold;
			
			givingGas[proc].Cold(elem) -= extractCold;			
			Components[proc].Cold(elem) += extractCold; 
		}	
	}

}
	
double GasReservoir::ColdGasMetallicity() const
{
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


const std::vector<GasStream> & GasReservoir::Composition() const
{
	return Components;
}


void GasReservoir::Wipe()
{
	for (int p = 0; p < ProcessCount; ++p)
	{
		for (int e = 0; e < ElementCount; ++e)
		{
			ElementID elem = (ElementID)e;
			Components[p].Cold(elem) = 0;
			Components[p].Hot(elem) = 0;
		}
	}
}
