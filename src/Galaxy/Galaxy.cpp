#include "Galaxy.h"

Galaxy::Galaxy(Options * opts)
{
	log(1) << "\nMain Galaxy object initialised\n";
	Opts = opts;
	
	double dr = Opts->Galaxy.MaxRadius / Opts->Simulation.NRings;
	
	GasMass = 0;
	for (int i = 0; i < Opts->Simulation.NRings; ++i)
	{
		Ring R = Ring(opts,i,dr,this);
		Rings.push_back(R);
		GasMass += R.Gas.Mass;
	}
	
	IGM = GasReservoir(Opts);
	double igmMass = 100.0;
	IGM.SetPrimordial(igmMass);
	
	
	//Demosthenes = YieldGrid(opts);
	

}

void Galaxy::OpenLogs()
{
	std::string root = Opts->Simulation.FileRoot;
	
	GalaxyState.open(root + Opts->Simulation.GalaxyStateFile,std::fstream::out);
}

void Galaxy::CloseLogs()
{
	GalaxyState.close();
}

double Galaxy::InfallRate(double t)
{
	double sum = 0.0;
	
	for (int i = 0; i < Opts->Galaxy.InfallMasses.size(); ++i)
	{
		double Mi = Opts->Galaxy.InfallMasses[i];
		double Bi = Opts->Galaxy.InfallTimeScales[i];
		
		sum += Mi/Bi * exp(-t/Bi);
	}

	return sum;
}


double Galaxy::GasScaleLength(double t)
{
	double t0 = Opts->Galaxy.ScaleLengthDelay;
	double tg = Opts->Galaxy.ScaleLengthGrowth;
	double tf = Opts->Galaxy.ScaleLengthTimeTether;
	
	double N = 1.0/(atan((tf - t0)/tg) - atan(-t0/tg));
	
	double R0 = Opts->Galaxy.MinScaleLength;
	double Rf = Opts->Galaxy.MaxScaleLength;
	
	return R0 + N*(Rf - R0) * (atan( (t - t0)/tg) - atan(-t0/tg));

}


void Galaxy::Evolve()
{
	OpenLogs();
	
	double dt = Opts->Simulation.TimeStep;
	int timeStep = 0;
	int nRings = Opts->Simulation.NRings;
	
	
	for (double t = 0; t < Opts->Simulation.FinalTime; t+=dt)
	{
		SaveState(t);
		
		UpdateGasMass(t);
		
		
		++timeStep;
	}
	
	SaveState(Opts->Simulation.FinalTime+dt);
	
	CloseLogs();
}

void Galaxy::UpdateGasMass(double t)
{
	double dt = Opts->Simulation.TimeStep;
	int nRings = Opts->Simulation.NRings;
	double totalInfall = InfallRate(t)*dt;
	double newR = GasScaleLength(t);
	double newMass = (GasMass + totalInfall);
	
	double newMassSum = 0.0;
	for (int ringID = 0; ringID < nRings; ++ringID)
	{
		Rings[ringID].Accrete(t,newMass, newR);
		newMassSum+= Rings[ringID].Gas.Mass;
	}
	GasMass = newMassSum;
}

void Galaxy::SaveState(double t)
{
	int width = 10;
	for (int id = 0; id < Opts->Simulation.NRings; ++id)
	{
		GalaxyState << std::setw(width) << t << "\t" << Rings[id].Radius << "\t" << Rings[id].Mass() << "\t" << Rings[id].SurfaceDensity << "\t" << Rings[id].Gas.Mass<< "\t" << GasMass <<"\n";
	}
}
