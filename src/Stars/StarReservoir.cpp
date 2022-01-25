#include "StarReservoir.h"

StarReservoir::StarReservoir(int parentRing, InitialisedData & data) : Data(data),Param(data.Param), ParentRing(parentRing), IMF(data.IMF), SLF(data.SLF), Remnants(data), YieldOutput(data.Param)
{
	StellarPopulation empty(Data);
		
	for (int i = 0; i < Param.Meta.SimulationSteps+1; ++i)
	{
		Population.push_back(empty);
	}
	
	//Compute the parent ring surface area....needed for SFR so computing here is efficient!
	const double pi = 3.141592654;
	double width = Param.Galaxy.RingWidth[parentRing];
	double r = Param.Galaxy.RingRadius[parentRing];
	ParentArea = 2 * pi * r * width;
	Temp_Mass = 0;
	EventRate.resize(Param.Meta.SimulationSteps-1);
	PopulationIndex = 0;
}

double integratedSchmidt(double s0, double prefactor,double power, double t)
{
	if (power != 1)
	{
		double timeFactor = (power - 1)*prefactor* t;
		return pow( 1.0/pow(s0,power - 1) + timeFactor, 1.0/(1 - power));
	}
	else
	{
		return s0 * exp( - prefactor*t);
	}
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
	double gasDensity = integratedSchmidt(density,prefactor,power,Param.Meta.TimeStep);
	
	return density - gasDensity;
	
}

void StarReservoir::Form(GasReservoir & gas)
{
	double z = gas.Metallicity();
	
	double initMass = gas.ColdMass();
	
	//~ double initialTotalMass = gas.Mass() + Mass();
	
	////////   density version (old)
	//~ double gasSurfaceDensity = gas.ColdMass() / ParentArea;
	//~ double gasLossMass = std::max(0.0,ParentArea * SFR_GasLoss(gasSurfaceDensity));
	
	//////// mass version (new)
	
	
	double gasLossMass = SFR_GasLoss(gas.ColdMass());
	
	double heatFrac = Param.Stellar.FeedbackFactor;
	
	
	double starMassFormed = 1.0/(1 + heatFrac) * gasLossMass;
	double feedbackMass = gasLossMass - starMassFormed;
	gas.Deplete(starMassFormed,0.0);	
	gas.Heat(feedbackMass); 
	
	EventRate[PopulationIndex].StarMassFormed += starMassFormed;
	

	
	int newStarCount = Population[PopulationIndex].FormStars(starMassFormed,PopulationIndex,z);
	
	
	EventRate[PopulationIndex].NStarsFormed += newStarCount;
	
	if (gas.ColdMass() < 0)
	{
		std::cout << "ERROR!" << gas.ColdMass() << "  " << gas.HotMass() << std::endl;
		exit(3);
	}
	++PopulationIndex;
}

double StarReservoir::AliveMass()
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
	std::string ringName = "Ring" + std::to_string(ParentRing) + "_stars.dat";
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

void StarReservoir::Death(int currentTime, GasReservoir & birthGas)
{
	YieldOutput.WipeMemoryUpTo(currentTime);
	for (int i = 0; i < currentTime+1; ++i)
	{
		
		if (Population[i].Active())
		{
			
			Population[i].Death(currentTime, YieldOutput,Remnants, birthGas, EventRate[currentTime]);
		}
		
	}
	Remnants.Decay(currentTime,YieldOutput, EventRate[currentTime]);
}

const std::vector<GasStream> & StarReservoir::YieldsFrom(int t)
{
	return YieldOutput.GetHistory(t);
}

MassReport StarReservoir::DeadMass()
{
	return Remnants.Mass();
}

void StarReservoir::SaveEventRate(int t, std::stringstream & output)
{
	if (t == 0 && ParentRing == 0)
	{
		EventRate[0].AddHeaders(output);
	}
	output << t * Param.Meta.TimeStep<< ", " << Param.Galaxy.RingRadius[ParentRing] << ", ";
	EventRate[t].Save(output,Param.Meta.TimeStep);
}
