#include "Gas.h"

Gas::Gas() : Species(ElementCount,0.0)
{
	//~ Species = std::vector<double>(ElementCount,0.0);
	//~ internal_Mass = 0;
	NeedsRecomputing = true;
}
Gas::Gas(const std::vector<double> & input) : Species(input)
{
	CheckMass();
}

double Gas::Mass()
{
	//~ std::cout << "called"<<std::endl;
	if (NeedsRecomputing)
	{
		CheckMass();
	}
	//~ std::cout << "returned " << std::endl;
	return internal_Mass;

}
double Gas::Mass() const
{
	return internal_Mass;
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

Gas Gas::CGM_polluted(double mass)
{
	double primordialY = 0.25;

	double cgm_mg = 1e-6;
	double cgm_fe = 2.5e-6;
	double cgm_eu = 1e-6;

	double primordialX = 1.0 - primordialY - cgm_eu - cgm_fe - cgm_mg;

	Gas g = Gas();
	g.Species[Hydrogen] = primordialX * mass;
	g.Species[Helium] = primordialY * mass;
	g.Species[Magnesium] = cgm_mg * mass;
	g.Species[Iron] = cgm_fe * mass;
	g.Species[Europium] = cgm_eu * mass;
	return g;
} 

double & Gas::operator[](ElementID id)
{
	NeedsRecomputing = true;
	return Species[id];
}
const double & Gas::operator[](ElementID id) const
{
	return Species[id];
}
void Gas::CheckMass()
{
	double basicMass = Species[Hydrogen] + Species[Helium] + Species[Metals];
	double elementWiseMass = -Species[Metals]; //avoid double counting
	for (int i = 0; i < ElementCount; ++i)
	{
		elementWiseMass += Species[i];
	}
	if (elementWiseMass > basicMass)
	{
		Species[Metals] += elementWiseMass - basicMass;
		basicMass= elementWiseMass;	
	}
	internal_Mass = basicMass;
	NeedsRecomputing = false;
}
