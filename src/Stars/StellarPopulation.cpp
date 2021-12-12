#include "StellarPopulation.h"

IsoMass::IsoMass()
{
	Mass = 0;
	Count = 0;
	BirthIndex = 0;
	DeathIndex = 1e10;
}
IsoMass::IsoMass(int n, double m, int birth, int death)
{
	Mass = m;
	Count = n;
	BirthIndex = birth;
	DeathIndex = death;
}


StellarPopulation::StellarPopulation(const IMF_Functor & imf, const GlobalParameters & param): Param(param), IMF(imf)
{
	Distribution.resize(Param.Stellar.MassResolution);
}


void StellarPopulation::FormStars(double formingMass, int timeIndex)
{
	double NStarsFormed = IMF.FormationCount(formingMass);
	double budget = 0;
	
	for (int i = Param.Stellar.MassResolution -1; i >= 0; --i)
	{
		double m = Param.Stellar.MassGrid[i];
		double nStars = NStarsFormed * IMF.Weighting(i);
		budget +=  nStars * m/1e9;
		
		int deathIndex = timeIndex; ////////////////// HERE IS WHERE ILM GETS CALLED!
		
		Distribution[i] = IsoMass(nStars,m,timeIndex,deathIndex);
	}
	//the remaining mass gets turned into immortal stars, al of which are assumed to have the minimum mortal mass
	double mInf = Param.Stellar.ImmortalMass;
	double effectiveImmortalCount = std::max(0.0,formingMass - budget) / mInf;
	ImmortalStars = IsoMass(effectiveImmortalCount,mInf,timeIndex,1e10);
}
