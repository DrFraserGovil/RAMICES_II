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
	StreamIn(input, hotFrac);
}

void GasStream::Absorb(const GasStream & input)
{
	for (int i = 0; i < ElementCount; ++i)
	{
		Cold.Species[i] += input.Cold.Species[i];
		Hot.Species[i] += input.Hot.Species[i];
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
		if (Cold.Species[i] > 0)
		{
			Cold.Species[i] *= (1.0 - coldLossFraction);
		}
		if (Hot.Species[i] > 0)
		{
			Hot.Species[i] *= (1.0 - hotLossFraction);	
		}	
	}
	NeedsRecomputing = true;
}
void GasStream::StreamIn(const Gas & input, double hotFrac)
{
	double coldFrac = (1.0 - hotFrac);
	for (int i = 0; i < ElementCount; ++i)
	{
		Cold.Species[i] += coldFrac * input.Species[i];
		Hot.Species[i] += hotFrac * input.Species[i];
	}
	NeedsRecomputing = true;
}

void GasStream::ComputeMasses()
{
	internal_ColdMass = 0;
	internal_HotMass = 0;
	for (int i = 0; i < ElementCount; ++i)
	{
		internal_ColdMass += Cold.Species[i];
		internal_HotMass += Hot.Species[i];
	}
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
