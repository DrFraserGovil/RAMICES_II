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
	ID = id;
	Width = width;
	Radius = ((double)id + 0.5)*width;
	
	double rm = Radius - Width/2;
	double rp = Radius + Width/2;
	Area = PI * (rp*rp - rm * rm); 
	InnerRadius = rm;
	OuterRadius = rp;
	Parent = parent;
	Gas = GasReservoir(opts);
	
	
	//get initial annulus mass from exponential surface density
	double primordialTotalMass = Opts->Galaxy.PrimordialMass;
	double R0 = parent->GasScaleLength(0.0);
	double annulusMass = RingMass(primordialTotalMass, R0);
	 
	Gas.SetPrimordial(annulusMass);
	SurfaceDensity = annulusMass / Area;

	Stars = StellarPopulation(opts);
}

GasRequest Ring::AccretionRequest(double t, double newGalaxyMass, double newR)
{
	//Accretion requests calculate how much material is to be grabbed from the ISM and from neighbouring rings
	//this function does not actually perform the transfer - that happens at a Galaxy level
	GasRequest Request;
	
	double targetMass = RingMass(newGalaxyMass,newR);
	double massDeficit = targetMass - Gas.Mass;// + lowerRingRequest;

	if (massDeficit < 0)
	{
		Request.IGM = 0.0;
		Request.Disc = 0.0;
	}
	else
	{
		//simple lambda to make the algebra nicer
		double a = Opts->Galaxy.InflowParameterA;
		double b = Opts->Galaxy.InflowParameterB;
		double Rmax = Opts->Galaxy.MaxRadius;
		auto bFunc = [a,b,Rmax](double x) { return a/(4*Rmax) * pow(x,4) + (b-1)/3*pow(x,3); };
		
		//see Bilitewski & Schonrich 2012 for derivation of this
		double bilitewskiRatio = 2.0/(OuterRadius * OuterRadius - InnerRadius * InnerRadius) * (bFunc(InnerRadius) - bFunc(OuterRadius));
		
		//~ if (bilitewskiRatio < 1.0)
		//~ {
			//~ bilitewskiRatio = 0;
		//~ }
		
		double igmGrab = std::max(0.0,massDeficit / (1.0 + bilitewskiRatio));
		double discGrab = std::max(0.0,massDeficit - igmGrab);
	

		//check that this does not empty out the rings
		if ( (ID < Opts->Simulation.NRings-1) && (discGrab > Parent->Rings[ID+1].Gas.Mass) )
		{
			double dilution = 0.999; //prevents total emptying of the ring, acts as a div0 buffer without altering physics
			double mMass = dilution * Parent->Rings[ID+1].Gas.Mass;
			discGrab = mMass;
			igmGrab = std::max(0.0,massDeficit - mMass);
		} 
		//final ring has no disc to accrete to
		if (ID == Opts->Simulation.NRings - 1)
		{
			igmGrab = massDeficit;
			discGrab = 0;
		}

		Request.IGM = igmGrab;
		Request.Disc = discGrab;
	}
	
	return Request;
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

void Ring::FormStars(double t)
{
	double FormedMass = StarFormationRate();
	double FeedbackMass = FormedMass * Opts->Galaxy.FeedbackFactor;
	
	//removes the gas
	Parent->PushIGM(ID,FeedbackMass, 0.0);
	Gas.Deplete(FormedMass,0.0);
	
	
	//fake star formation
	StarSet s= StarSet(Opts,FormedMass,0,1);
	Stars.Stars.push_back(s);
}

double Ring::StarFormationRate()
{
	double cut = Opts->Galaxy.SchmidtCut;
	
	double sfrDensityRate = 0.0;

	if (SurfaceDensity > cut)
	{
		sfrDensityRate = pow(SurfaceDensity,1.4);
	}
	else
	{
		sfrDensityRate = pow(SurfaceDensity,4) / pow(cut,2.6);
	}

	double deltaMassFormed = sfrDensityRate * Area * Opts->Simulation.TimeStep * Opts->Galaxy.SchmidtPrefactor;

	//checks that sfr + feedback will not empty gas, if so - use feedback to limit SFR
	double deltaMassAll = (1.0 + Opts->Galaxy.FeedbackFactor) * deltaMassFormed;
	double maxFormingMass = Gas.ColdMass * Opts->Galaxy.MaxSFRFraction;
	if (deltaMassAll > maxFormingMass)
	{
		deltaMassFormed = maxFormingMass / (1.0 + Opts->Galaxy.FeedbackFactor);
	}

	return deltaMassFormed;
	
}

void Ring::UpdateInternalProperties()
{
	SurfaceDensity = Gas.Mass / Area;
}

std::vector<std::string> Ring::PropertyHeaders()
{
	std::vector<std::string> headers = {"Time", "Radius", "Area", "TotalMass", "ColdGasMass", "HotGasMass", "StellarMass", "RemnantMass", "SFRRate"};
	return headers;
}

std::vector<double> Ring::ReportProperties(double t)
{
	std::vector<double> properties = {t, Radius, Area, Mass(), Gas.ColdMass, Gas.HotMass, Stars.Mass(), 0.0,0.0};
	return properties;
}
