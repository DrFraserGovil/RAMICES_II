#include "StarReservoir.h"

StarReservoir::StarReservoir(const GlobalParameters & param, int parentRing, const IMF_Functor & imf, SLF_Functor & slf) : Param(param), ParentRing(parentRing), IMF(imf), SLF(slf)
{
	StellarPopulation empty(IMF,SLF,Param);
		
	for (int i = 0; i < Param.Meta.SimulationSteps; ++i)
	{
		Population.push_back(empty);
	}
	
	//Compute the parent ring surface area....needed for SFR so computing here is efficient!
	const double pi = 3.141592654;
	double width = Param.Galaxy.Radius / Param.Galaxy.RingCount;
	double r = (parentRing + 0.5)*width;
	ParentArea = 2 * pi * r * width;
	Temp_Mass = 0;
	PopulationIndex = 0;
}

double integratedSchmidt(double s0, double cutDensity, double prefactor,double power, double t)
{
	double timeFactor = (power - 1)*prefactor* t;
	return pow( 1.0/pow(s0,power - 1) + timeFactor, 1.0/(1 - power));
}
double StarReservoir::SFR_GasLoss(double density)
{	
	double nBig = Param.Stellar.SchmidtMainPower;
	double nSmall = Param.Stellar.SchmidtLowPower;
	double sigmaCut = Param.Stellar.SchmidtDensityCut;
	double prefactor = Param.Stellar.SchmidtPrefactor * (1+Param.Stellar.FeedbackFactor);
	double power = nBig;
	if (density < sigmaCut)
	{
		power = nSmall;
		prefactor *= pow(sigmaCut,nBig - nSmall); // ensures the SFR is continuous
	}
	//integrates the SFR smoothly over the timestep (including Feedback losses), makes it impossible to losemore gas than you have
	double gasDensity = integratedSchmidt(density,sigmaCut,prefactor,power,Param.Meta.TimeStep);
	
	return density - gasDensity;
	
}

void StarReservoir::Form(GasReservoir & gas)
{
	double z = gas.Metallicity();
	double initMass = gas.ColdMass();
	//~ double initialTotalMass = gas.Mass() + Mass();
	double gasSurfaceDensity = gas.ColdMass() / ParentArea;
	double gasLossMass = std::max(0.0,ParentArea * SFR_GasLoss(gasSurfaceDensity));
	double heatFrac = Param.Stellar.FeedbackFactor;
	
	if (PopulationIndex > 100)
	{
		gasLossMass = 0;
	}
	
	double starMassFormed = 1.0/(1 + heatFrac) * gasLossMass;
	double feedbackMass = gasLossMass - starMassFormed;
	gas.Deplete(starMassFormed,0.0);	
	gas.Heat(feedbackMass); 
	
	
	Population[PopulationIndex].FormStars(starMassFormed,PopulationIndex,z);
	if (gas.ColdMass() < 0)
	{
		std::cout << "ERROR!" << gas.ColdMass() << "  " << gas.HotMass() << std::endl;
		exit(3);
	}
	++PopulationIndex;
}

double StarReservoir::Mass()
{
	double m = 0;
	for (int i = 0; i < Population.size(); ++i)
	{
		m += Population[i].Mass();
	}
	return m;
}

void StarReservoir::PrintStatus(int t)
{
	std::string ringName = Param.Output.RingDirectory.Value + "Ring" + std::to_string(ParentRing) + "_stars.dat";
	std::stringstream output;
	int N = Param.Stellar.MassResolution;
	if (t == 0)
	{
		JSL::initialiseFile(ringName);
		output << "0, 0," << Param.Stellar.ImmortalMass-2;
		for (int j = 0; j < N; ++j)
		{
			output << ", " << Param.Stellar.MassGrid[j];
		}
		output << "\n";
	}
	
	
	for (int i = 0; i < t; ++i)
	{
		output << t << ", " << t-i << ", " <<Population[i].Relic().Count;
		for (int j = 0; j < N; ++j)
		{
			output << ", " << Population[i][j].Count;
		}
		output << "\n";
	}
	JSL::writeStringToFile(ringName, output.str());
}

void StarReservoir::Death(int currentTime)
{
	for (int i = 0; i < currentTime; ++i)
	{
		if (Population[i].Active())
		{
			Population[i].Death(currentTime);
		}
	}
}
