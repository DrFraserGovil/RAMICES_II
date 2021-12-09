#include "Gas.h"

Gas::Gas()
{
	Species = std::vector<double>(ElementCount,0.0);
}
Gas::Gas(const std::vector<double> & input) : Species(input)
{
	//nope
}

double Gas::Mass()
{
	double sum = 0;
	for (int i = 0; i < ElementCount; ++i)
	{
		sum += Species[i];	
	}
}

Gas Gas::Empty()
{
	return Gas();
}
Gas Gas::Primordial(double mass)
{
	double primordialX = 0.75;
	double primordialY = 0.25;
	Gas g = Gas();
	g.Species[Hydrogen] = primordialX * mass;
	g.Species[Helium] = primordialY * mass;
	return g;
}

double & Gas::operator[](ElementID id)
{
	return Species[id];
}
const double & Gas::operator[](ElementID id) const
{
	return Species[id];
}
