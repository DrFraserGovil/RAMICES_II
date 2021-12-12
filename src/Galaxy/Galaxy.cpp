#include "Galaxy.h"
double pi = 3.141592654;
Galaxy::Galaxy(const GlobalParameters & param): Param(param), IGM(GasReservoir::Primordial(param.Galaxy.IGM_Mass,param)), IMF(param)
{
	double ringWidth = Param.Galaxy.Radius / Param.Galaxy.RingCount;
	double initialScaleLength = GasScaleLength(0);
	for (int i = 0; i < Param.Galaxy.RingCount; ++i)
	{
		double ri = (i + 0.5)*ringWidth;
		double predictedDensity = PredictSurfaceDensity(ri,ringWidth,Param.Galaxy.PrimordialMass,initialScaleLength);
		double predictedMass = 2*pi * ri * ringWidth * predictedDensity;
		Rings.push_back(Ring(i,predictedMass,IMF,Param));
	}
	
	
}


void Galaxy::Evolve()
{
	double t = 0;
	SaveState(t);

	for (int timestep = 0; timestep < Param.Meta.SimulationSteps; ++timestep)
	{
		Infall(t);
		FormStars();
		t += Param.Meta.TimeStep;
		SaveState(t);
	}
	
}
double Galaxy::InfallMass(double t)
{
	double delta = Param.Meta.TimeStep;
	//I have analytically integrated Mdot between t-Delta and t, just for an additional layer of accuracy at early times
	double bFast = Param.Galaxy.InfallTime1;
	double bSlow = Param.Galaxy.InfallTime2;
	double fastInfall = Param.Galaxy.InfallMass1 * exp(-t/bFast) * (exp(delta / bFast) - 1.0);
	double slowInfall = Param.Galaxy.InfallMass2 * exp(-t/bSlow) * ( exp(delta/bSlow) - 1.0);
	
	return fastInfall + slowInfall;
}
double Galaxy::GasMass()
{
	double m = 0;
	for (int i = 0; i < Rings.size(); ++i)
	{
		m+=Rings[i].Gas.Mass();
	}
	return m;
}
double Galaxy::StarMass()
{
	return 0;
}
double Galaxy::RelicMass()
{
	return 0;
}
double Galaxy::Mass()
{
	return GasMass() + StarMass() + RelicMass();
}
double Galaxy::GasScaleLength(double t)
{
	
	double t0 = Param.Galaxy.ScaleLengthDelay;
	double tg = Param.Galaxy.ScaleLengthTimeScale;
	double tf = Param.Galaxy.ScaleLengthFinalTime;
	double R0 = Param.Galaxy.MinScaleLength;
	double Rf = Param.Galaxy.MaxScaleLength;

	double N = 1.0/(atan((tf - t0)/tg) - atan(-t0/tg));
	return R0 + N*(Rf - R0) * (atan( (t - t0)/tg) - atan(-t0/tg));
}


double bilitewskiRatio(double a, double b, int n, int N)
{
	double denominator = 1.0/(2*n + 1);
	double nsq = n*n;
	double firstTerm = (-2 * a)/(4 * N) * denominator * (4*n*nsq + 6*nsq + 4*n + 1);
	double secondTerm = 2*(1 - b)/3 * denominator * (3 *nsq + 3*n + 1);
	
	return firstTerm + secondTerm;
	
}

void Galaxy::InsertInfallingGas(int ring, double amount)
{
	double a_factor  = Param.Galaxy.InflowParameterA;
	double b_factor = Param.Galaxy.InflowParameterB;
	double remainingMass;
	if ( ring < Rings.size() - 1)
	{
		double ratio = bilitewskiRatio(a_factor,b_factor,ring,Rings.size());
		double inflowMass = ratio/(1 + ratio) * amount;
		
		//check that we do not remove more gas than is actually present
		inflowMass = std::min(inflowMass, Rings[ring+1].Gas.Mass());
				
		Rings[ring].Gas.TransferFrom(Rings[ring+1].Gas,inflowMass);
		
		//if some part of the budget was missed because of the std::min above, then make up the deficit from the IGM
		remainingMass = amount -inflowMass;
	}
	else
	{
		remainingMass = amount;
	}
	GasStream igm = IGM.AccretionStream(remainingMass);
	Rings[ring].Gas.Absorb(igm);
	
}

void Galaxy::Infall(double t)
{
	double Rd = GasScaleLength(t);
	double oldGas = GasMass();
	double predictedInfall = InfallMass(t);
	double newGas = oldGas + predictedInfall;
	
	for (int i = 0; i < Rings.size(); ++i)
	{
		double r = Rings[i].Radius;
		double w = Rings[i].Width;
		double sigma = PredictSurfaceDensity(r,w,newGas,Rd);
		double newMass = sigma * 2 * pi * r*w;
		double oldMass = Rings[i].Gas.Mass();
		double delta = newMass - oldMass;
		
		if (delta > 0)
		{
			InsertInfallingGas(i,delta);
		}
		else
		{
			IGM.TransferFrom(Rings[i].Gas,abs(delta));
			//~ Rings[i].Gas.Deplete(abs(delta));
		}
		
	}	
}

double mass_integrand(double x)
{
	return -exp(-x) * (x + 1);
}

double Galaxy::PredictSurfaceDensity(double radius, double width, double totalGasMass, double scaleLength)
{
	double r = radius;
	double w = width;
	double truncatedFactor =  (1 - (1 + Param.Galaxy.Radius/scaleLength)*exp(-Param.Galaxy.Radius/scaleLength)); // from the finite size of the galaxy -- not an infinite exponential!
	double prefactor = totalGasMass/(2 * pi * r * w)  / truncatedFactor;
	double upRadius = (r+w/2)/scaleLength;
	double downRadius = (r - w/2)/scaleLength;
	return prefactor * (mass_integrand(upRadius) - mass_integrand(downRadius));
}



void Galaxy::FormStars()
{
	for (int i = 0; i < Rings.size(); ++i)
	{
		Rings[i].MakeStars();
	}
}

void Galaxy::SaveState(double t)
{
	SaveState_Mass(t);
}
void Galaxy::SaveState_Mass(double t)
{
	std::stringstream output;
	if (t == 0)
	{
		output << MassHeaders() << "\n";
	}
	
	for (int i = 0; i < Rings.size(); ++i)
	{
		double Ms = Rings[i].Stars.Mass();
		double Mc = Rings[i].Gas.ColdMass();
		double Mh = Rings[i].Gas.HotMass();
		double Mr = 0;
		double Mt = Ms + Mc + Mh + Mr;
		double Migm = IGM.Mass();
		std::vector<double> vals = {Rings[i].Radius, Mt,Ms,Mc,Mh,Mr,Migm};
		output << t;
		for (int j = 0; j < vals.size(); ++j)
		{
			output << ", " << vals[j];
		}
		output << "\n";
	}	
	JSL::writeStringToFile(Param.Output.GalaxyMassFile,output.str());
	
}
std::string Galaxy::MassHeaders()
{
	return "Time, Radius, TotalMass, StellarMass, ColdGasMass, HotGasMass, RelicMass,IGMMass";
}
