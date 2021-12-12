#include "StarReservoir.h"

StarReservoir::StarReservoir(const GlobalParameters & param, int parentRing, const IMF_Functor & imf) : Param(param), ParentRing(parentRing), IMF(imf)
{
	StellarPopulation empty(IMF,Param);
		
	if (parentRing == 0)
	{
		empty.FormStars(10,0);
	}
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
	
}

double integratedSchmidt(double s0, double cutDensity, double prefactor,double power, double t)
{
	double timeFactor = (power - 1)*prefactor/pow(cutDensity,power) * t;
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
	}
	//integrates the SFR smoothly over the timestep (including Feedback losses), makes it impossible to losemore gas than you have
	double gasDensity = integratedSchmidt(density,sigmaCut,prefactor,power,Param.Meta.TimeStep);
	
	return density - gasDensity;
	
}

void StarReservoir::Birth(GasReservoir & gas)
{
	double initialTotalMass = gas.Mass() + Mass();
	double gasSurfaceDensity = gas.ColdMass() / ParentArea;
	double gasLossMass = std::max(0.0,ParentArea * SFR_GasLoss(gasSurfaceDensity));
	double heatFrac = Param.Stellar.FeedbackFactor;
	
	double starMassFormed = 1.0/(1 + heatFrac) * gasLossMass;
	double feedbackMass = gasLossMass - starMassFormed;
	gas.Deplete(starMassFormed,0.0);	
	gas.Heat(feedbackMass); 
	Temp_Mass += starMassFormed;
	
}

double StarReservoir::Mass()
{
	return Temp_Mass;
}
