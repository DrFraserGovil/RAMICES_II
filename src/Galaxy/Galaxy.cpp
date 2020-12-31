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
	//loop through first time to calculate required gas
	log(3) << "Beginning request loop at t = " + std::to_string(t) + "\n";
	double previousAsk = 0;
	for (int ringID = 0; ringID < nRings; ++ringID)
	{
		GasRequest request = Rings[ringID].AccretionRequest(t,newMass, newR);

		GasReservoir igm = PullIGM(request.IGM);
		
		igm.AddTo(&Rings[ringID].Gas);
		
		if (ringID < nRings - 1)
		{
			Rings[ringID].Gas.TakeFrom(&Rings[ringID+1].Gas,request.Disc);
		}


		Rings[ringID].UpdateInternalProperties();
		newMassSum += Rings[ringID].Gas.Mass;
	}
	

	GasMass = newMass;
}

GasReservoir Galaxy::PullIGM(double m)
{
	//simply copies the current IGM abundance into a new object and sets it to 100% cold gas. Does not alter the IGM.
	GasReservoir subset = IGM;
	subset.Mass = m;
	subset.ColdMass = m;
	subset.HotMass = 0;
	
	return subset;
}

void Galaxy::SaveState(double t)
{
	int width = 10;
	
	for (int id = 0; id < Opts->Simulation.NRings; ++id)
	{
		std::vector<double> GalaxySaver = {t,Rings[id].Radius, Rings[id].Mass(), Rings[id].SurfaceDensity, Rings[id].Gas.Mass,GasMass};
		
		for (auto const &saver : GalaxySaver)
		{
			GalaxyState << std::setw(width)  << std::to_string(saver)+",";
		}
		GalaxyState << "\n";
	}
}
