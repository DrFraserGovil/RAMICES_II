#include "StarReservoir.h"

StarReservoir::StarReservoir(int parentRing, InitialisedData & data) : Data(data),Param(data.Param), ParentRing(parentRing), IMF(data.IMF), SLF(data.SLF), Remnants(data)
{
	StellarPopulation empty(Data,parentRing);
	
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
	YieldOutput = std::vector<GasReservoir>(Param.Meta.SimulationSteps+1,GasReservoir(Param));
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
	//compute how much cold gas mass is lost to star formation (through stars + feedback)
	double initMass = gas.ColdMass();
	double gasSurfaceDensity = gas.Mass() / ParentArea;
	double gasLossMass = std::max(0.0,ParentArea * SFR_GasLoss(gasSurfaceDensity));
	gasLossMass = std::min(gas.ColdMass() * 0.5, gasLossMass);
		
	//compute how much goes to stars vs hot gas
	double heatFrac = Param.Stellar.FeedbackFactor;
	double starMassFormed = 1.0/(1.0 + heatFrac) * gasLossMass;
	double feedbackMass = gasLossMass - starMassFormed;
	
	//move the gas around + form the stars
	gas.Deplete(starMassFormed,0.0);	
	gas.Heat(feedbackMass); 
	int newStarCount = Population[PopulationIndex].FormStars(starMassFormed,PopulationIndex,gas);
	
	//some accounting for event rate tracking
	EventRate[PopulationIndex].StarMassFormed += starMassFormed;
	EventRate[PopulationIndex].NStarsFormed += newStarCount;
	
	
	//check that nothing went horribly wrong
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

void StarReservoir::Death(int currentTime)
{
	
	for (int i = 0; i < currentTime+1; ++i)
	{
		YieldOutput[i].Wipe();
		if (Population[i].Active())
		{
			Population[i].Death(currentTime, YieldOutput,Remnants, EventRate[currentTime]);
		}
	}
	Remnants.Decay(currentTime,YieldOutput, EventRate[currentTime]);
	
	//to prevent const errors!
	//~ std::cout << "At end of death step for " << currentTime << " in ring " << ParentRing << std::endl;
	//~ for (int t = 0; t < currentTime + 1; ++t)
	//~ {
		//~ std::cout << t << std::endl;
		//~ for (int p = 0; p < ProcessCount; ++p)
		//~ {
			//~ std::cout << "\t" << p << "  " << YieldOutput[t][(SourceProcess)p].ColdMass() << std::endl;
				//~ double v = YieldOutput[t][(SourceProcess)p].ColdMass();
		//~ }
		//~ std::cout << "Remnant contribution: " << YieldOutput[t][Remnant].ColdMass() << std::endl;
	
	//~ }
}

const std::vector<GasStream> & StarReservoir::YieldsFrom(int t)
{
	return YieldOutput[t].Composition();
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


void StarReservoir::AssignMagnitudes()
{
	double minMv = 100;
	double maxMv = -100;
	for (int i = 0; i < PopulationIndex; ++i)
	{
		
		double age = (Param.Meta.SimulationSteps - 2 - i) * Param.Meta.TimeStep;
		//~ if (age == 0)
		//~ {
			//~ age = Param.Meta.TimeStep / 2; //just to prevent zero age line!
		//~ }
		double z = Population[i].Metallicity;
		Population[i].Age = age;
		for (int j = 0; j < Population[i].Distribution.size(); ++j)
		{
			if (Population[i].Distribution[j].Count > 0)
			{
				int m_ID = Population[i].Distribution[j].MassIndex;
				Population[i].Distribution[j].Isochrone = Data.Isochrones.GetProperties(m_ID,z,age);
				
			
			}
		}
		
	}	
}

