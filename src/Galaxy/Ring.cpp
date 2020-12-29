#include "Ring.h"

Ring::Ring(Options * opts)
{
	Radius = 0.0;
	Width = 0.0;
	Gas = GasReservoir(opts);
}

Ring::Ring(Options * opts, int id, double width, Galaxy * parent)
{
	Opts = opts;
	Width = width;
	Radius = ((double)id + 0.5)*width;
	
	double rm = Radius - Width/2;
	double rp = Radius + Width/2;
	Area = PI * (rp*rp - rm * rm); 
	Parent = parent;
	Gas = GasReservoir(opts);
	
	
	//get initial annulus mass from exponential surface density
	double primordialTotalMass = Opts->Galaxy.PrimordialMass;
	double R0 = parent->GasScaleLength(0.0);
	double annulusMass = RingMass(primordialTotalMass, R0);
	 
	Gas.SetPrimordial(annulusMass);
	SurfaceDensity = annulusMass / Area;

}

void Ring::Accrete(double t, double newGalaxyMass, double newR)
{
	

	double annulusMass = RingMass(newGalaxyMass,newR);
	
	
	double massDeficit = annulusMass - Gas.Mass;
	
	
	//NEED TO DO REAL ACCRETION HERE
	
	Gas.Mass = annulusMass;
	Gas.ColdMass += massDeficit;
	
	SurfaceDensity = Gas.Mass / Area;
}

double Ring::RingMass(double GalaxyMass, double ScaleLength)
{
	//simple lambda to make the algebra nicer
	auto integral = [](double x) { return -(x + 1.0) * exp(-x); };
	
	double Rp = Opts->Galaxy.MaxRadius;
	double x = Rp/ScaleLength;
	double nonInfFactor = integral(x) - integral(0.0);
	
	double Minf = GalaxyMass / nonInfFactor;
	
	double xPlus = (Radius + Width/2) / ScaleLength;
	double xMinus = (Radius - Width/2) / ScaleLength;
	
	double spatialIntegral = integral(xPlus) - integral(xMinus);
	return Minf * spatialIntegral;
}

double Ring::Mass()
{
	double stellarMass = 0;
	double remnantMass = 0;
	double gasMass = Gas.Mass;
	
	return stellarMass + remnantMass + gasMass;
}
