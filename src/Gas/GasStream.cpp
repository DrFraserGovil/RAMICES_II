#include "GasStream.h"

GasStream::GasStream()
{
	Source = Primordial;
	NeedsRecomputing = true;
}
GasStream::GasStream(SourceProcess source)
{
	Source = source;
	NeedsRecomputing = true;
}
GasStream::GasStream(SourceProcess source,  const Gas & hot,  const Gas & cold): internal_Hot(hot), internal_Cold(cold)
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


double & GasStream::Hot(ElementID el)
{
	Dirty();
	return internal_Hot[el];
}
double & GasStream::Cold(ElementID el)
{
	Dirty();
	return internal_Cold[el];
}

const Gas & GasStream::Hot() const
{
	internal_Hot.Mass();
	return internal_Hot;
}
const Gas & GasStream::Cold() const
{
	internal_Cold.Mass();
	return internal_Cold;
}

const double & GasStream::Hot(ElementID el) const
{
	return internal_Hot[el];
}
const double & GasStream::Cold(ElementID el) const
{
	return internal_Cold[el];
}

void GasStream::Absorb(const GasStream & input)
{
	for (int i = 0; i < ElementCount; ++i)
	{
		ElementID e = (ElementID)i;
		internal_Cold[e] += input.Cold(e);
		internal_Hot[e] += input.Hot(e);
	}
	NeedsRecomputing = true;
}

void GasStream::Absorb(const GasStream & input,double fraction)
{
	for (int i = 0; i < ElementCount; ++i)
	{
		ElementID e = (ElementID)i;
		internal_Cold[e] += input.Cold(e) * fraction;
		internal_Hot[e] += input.Hot(e) * fraction;
	}
	NeedsRecomputing = true;
}

void GasStream::Deplete(double amountToLose)
{
	//~ NeedsRecomputing = true;
	double lossFraction = amountToLose/Mass();
	//~ std::cout << lossFraction << std::endl;
	for (int i = 0; i < ElementCount; ++i)
	{
		ElementID e = (ElementID)i;
		if (internal_Cold[e] > 0)
		{
			internal_Cold[e] *= (1.0 - lossFraction);
		}
		if (internal_Hot[e] > 0)
		{
			internal_Hot[e] *= (1.0 - lossFraction);	
		}	
	}
	NeedsRecomputing = true;
}
void GasStream::Deplete(double amountToLose_Cold, double amountToLose_Hot)
{
	double coldLoss = amountToLose_Cold/ColdMass();
	double hotLoss = amountToLose_Hot/HotMass();
	for (int i = 0; i < ElementCount; ++i)
	{
		ElementID e = (ElementID)i;
		if (internal_Cold[e] > 0)
		{
			internal_Cold[e] *= (1.0 - coldLoss);
		}
		if (internal_Hot[e] > 0)
		{
			internal_Hot[e] *= (1.0 - hotLoss);	
		}	
	}
	NeedsRecomputing = true;
}


void GasStream::Absorb(const Gas & input, double hotFrac)
{
	double coldFrac = (1.0 - hotFrac);
	for (int i = 0; i < ElementCount; ++i)
	{
		
		internal_Cold[(ElementID)i] += coldFrac * input[(ElementID)i];
		internal_Hot[(ElementID)i] += hotFrac * input[(ElementID)i];
	}
	NeedsRecomputing = true;
}

void GasStream::ComputeMasses()
{
	internal_ColdMass = internal_Cold.Mass();
	internal_HotMass = internal_Hot.Mass();
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

double GasStream::ColdMass() const
{
	return internal_ColdMass;
}
double GasStream::HotMass() const
{
	return internal_HotMass;
}

void GasStream::Dirty()
{
	NeedsRecomputing = true;
}

void GasStream::Heat(double amountToHeat)
{
	
	//skip if empty!
	if (ColdMass() == 0)
	{
		return;
	}
	
	double moveFraction = amountToHeat/ColdMass();
	for (int i = 0; i < ElementCount; ++i)
	{
		ElementID elem = (ElementID)i;
		double mass = Cold(elem) * moveFraction;
		Cold(elem) -= mass;
		Hot(elem) += mass;
	}
	Dirty();

}

void GasStream::Cool(double amountToCool)
{
	
	//skip if empty!
	if (HotMass() == 0)
	{
		return;
	}
	double moveFraction = amountToCool/HotMass();
	for (int i = 0; i < ElementCount; ++i)
	{
		ElementID elem = (ElementID)i;
		double mass = Hot(elem) * moveFraction;
		Cold(elem) += mass;
		Hot(elem) -= mass;
	}
	Dirty();
}
