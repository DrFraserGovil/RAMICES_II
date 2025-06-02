#include "gas.h"

using namespace Element;
Gas::Gas() : internalMassOf(Element::Count,0.0)
{
	internal_Mass = 0;
}

double Gas::Mass()
{
	if (NeedsRecomputing)
	{
		ComputeMass();
	}
	return internal_Mass;
}

double Gas::Metallicity()
{
	double totalMass = Mass();
	double nonMetal = internalMassOf[Hydrogen] + internalMassOf[Helium];
	return 1.0 - nonMetal/totalMass;
}



double & Gas::operator[](Element::Species id)
{
	NeedsRecomputing= true;//returning a reference means the object may have been modified
	return internalMassOf[id];
}

double & Gas::operator[](int id)
{
	NeedsRecomputing= true;//returning a reference means the object may have been modified
	return internalMassOf[id];
}
double Gas::MassOf(Element::Species id)
{
	return internalMassOf[id];
}
double Gas::FractionOf(Element::Species id)
{
	return internalMassOf[id]/Mass();
}

void Gas::ComputeMass()
{
	internal_Mass = 0;
	for (int i =0; i < Element::Count; ++i)
	{
		internal_Mass += internalMassOf[i];
	}
	NeedsRecomputing = false;
}

Gas Gas::Primordial(double mass)
{
	Gas g;
	for (int el = 0; el < Element::Count; ++el)
	{
		g[el] = Settings.Abundance.PrimordialAbundances.Value()[el] * mass;
	}
	g.NeedsRecomputing = true;
	return g;
}

