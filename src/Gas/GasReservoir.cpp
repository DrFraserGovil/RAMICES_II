#include "GasReservoir.h"

GasReservoir::GasReservoir()
{
	Components.resize(ProcessCount);
	//make sure each stream has its own label correctly
	for (int i = 0; i < ProcessCount; ++i)
	{
		Components[i].Source = (SourceProcess)i;
	}
}

GasReservoir GasReservoir::Primordial(double mass, GlobalParameters & param)
{
	GasReservoir prim;
	
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
	Components[source].Dirty();
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
void GasReservoir::Absorb(const GasStream & givingGas)
{
	SourceProcess source = givingGas.Source;
	Components[source].Absorb(givingGas);
}