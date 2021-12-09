#include "GasStream.h"

GasStream::GasStream()
{
	Source = Unknown;
	NeedsRecomputing = true;
}
GasStream::GasStream(SourceProcess source)
{
	Source = source;
	NeedsRecomputing = true;
}
GasStream::GasStream(SourceProcess source,  const Gas & hot,  const Gas & cold): Hot(hot), Cold(cold)
{
	Source = source;
	NeedsRecomputing = true;
}
GasStream::GasStream(SourceProcess source, const Gas & input, double hotFrac)
{
	Source = source;
	NeedsRecomputing = true;
	Absorb(input, hotFrac);
}

void GasStream::Absorb(const GasStream & input)
{
	for (int i = 0; i < ElementCount; ++i)
	{
		ElementID e = (ElementID)i;
		Cold[e] += input.Cold[e];
		Hot[e] += input.Hot[e];
	}
	NeedsRecomputing = true;
}
void GasStream::Deplete(double amountToLose)
{
	double lossFraction = amountToLose/Mass();
	
	double coldLossFraction = amountToLose/ColdMass();
	double hotLossFraction = amountToLose/ HotMass();
	//~ std::cout << lossFraction << std::endl;
	for (int i = 0; i < ElementCount; ++i)
	{
		ElementID e = (ElementID)i;
		if (Cold[e] > 0)
		{
			Cold[e] *= (1.0 - coldLossFraction);
		}
		if (Hot[e] > 0)
		{
			Hot[e] *= (1.0 - hotLossFraction);	
		}	
	}
	NeedsRecomputing = true;
}
void GasStream::Absorb(const Gas & input, double hotFrac)
{
	double coldFrac = (1.0 - hotFrac);
	for (int i = 0; i < ElementCount; ++i)
	{
		
		Cold[(ElementID)i] += coldFrac * input[(ElementID)i];
		Hot[(ElementID)i] += hotFrac * input[(ElementID)i];
	}
	NeedsRecomputing = true;
}

void GasStream::ComputeMasses()
{
	internal_ColdMass = Cold.Mass();
	internal_HotMass = Hot.Mass();
	internal_TotalMass = internal_ColdMass + internal_HotMass;
	NeedsRecomputing = false;
}

double GasStream::Mass()
{
	if (NeedsRecomputing)
	{
		ComputeMasses();
	}
	return internal_TotalMass;
}
double GasStream::HotMass()
{
	if (NeedsRecomputing)
	{
		ComputeMasses();
	}
	return internal_HotMass;
}
double GasStream::ColdMass()
{
	if (NeedsRecomputing)
	{
		ComputeMasses();
	}
	return internal_ColdMass;
}
void GasStream::Dirty()
{
	NeedsRecomputing = true;
}
