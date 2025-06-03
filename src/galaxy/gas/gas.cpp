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
	double m = Mass();
	if (m== 0)
	{
		return 0;
	}
	return internalMassOf[id]/m;
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


//Factories
Gas Gas::WithComposition(double mass,const std::vector<double> & composition)
{
	if (mass < 0)
	{
		LOG(ERROR) << "Cannot create a gas object with negative mass (" << mass << " < 0)";
		throw std::logic_error("Unphysical quantity encountered");
	}
	auto g = Gas();
	for (int el = 0; el < Element::Count; ++el)
	{
		g[el] = composition[el] * mass;
	}
	g.NeedsRecomputing = true;
	return g;
}

Gas Gas::WithMass(const std::vector<double> & massArray)
{
	auto g = Gas();
	for (int el = 0; el < Element::Count; ++el)
	{
		g[el] = massArray[el];
	}
	g.NeedsRecomputing = true;
	return g;
}

Gas Gas::Primordial(double mass)
{
	return Gas::WithComposition(mass,Settings.Abundance.PrimordialAbundances);
}

Gas Gas::Empty()
{
	auto g =  Gas();
	return g;
}
