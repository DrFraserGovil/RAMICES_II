#include "StellarPopulation.h"
#include <stdexcept>
IsoMass::IsoMass()
{
	MassIndex = 0;
	Count = 0;
	BirthIndex = 0;
	DeathIndex = 1e10;
}
IsoMass::IsoMass(double n, int m, double z, int birth, int death)
{
	MassIndex = m;
	Count = n;
	Metallicity = z;
	BirthIndex = birth;
	DeathIndex = death;
}




StellarPopulation::StellarPopulation(InitialisedData & data): Param(data.Param), IMF(data.IMF), SLF(data.SLF), CCSNYield(data.CCSNYield), AGBYield(data.AGBYield)
{
	Distribution.resize(Param.Stellar.MassResolution);
	internal_MassCounter = 0;
	IsDepleted = false;
	IsLifetimeMonotonic = true;
}

IsoMass & StellarPopulation::Relic()
{
	return ImmortalStars;
}

const IsoMass & StellarPopulation::Relic() const
{
	return ImmortalStars;
}

const IsoMass & StellarPopulation::operator [](int i) const
{
	if (i < Distribution.size() && i >= 0)
	{
		return Distribution[i];
	}
	else
	{
		throw std::runtime_error("You just tried to access a member of a Stellar Population which does not exist!");
	}
}
IsoMass & StellarPopulation::operator [](int i)
{
	if (i < Distribution.size() && i >= 0)
	{
		return Distribution[i];
	}
	else
	{
		throw std::runtime_error("You just tried to access a member of a Stellar Population which does not exist!");
	}
}

int StellarPopulation::FormStars(double formingMass, int timeIndex,double formingMetallicity)
{
	//~ std::cout << "I am here to form " << formingMass << std::endl;
	double NStarsFormed = IMF.FormationCount(formingMass);
	//~ std::cout << NStarsFormed <<std::endl;
	double budget = 0;
	
	int prevIndex = timeIndex;
	for (int i = Param.Stellar.MassResolution -1; i >= 0; --i)
	{
		//~ std::cout << "Mass " << i << "  " << formingMetallicity << std::endl;
		double m = Param.Stellar.MassGrid[i];
		double nStars = NStarsFormed * IMF.Weighting(i);
		//~ std::cout << i << "  " << m << "  " << nStars << std::endl;
		budget +=  nStars * m/1e9;
		
		//~ std::cout << "Calc. death index" <<std::endl;
		int deathIndex = timeIndex + SLF(i,formingMetallicity);
		//~ std::cout << deathIndex << std::endl;
		
		//check for monotonicity
		if (deathIndex < prevIndex)
		{
			//~ IsLifetimeMonotonic = false;
			//~ std::cout << "t = " << timeIndex << "  death index = " << deathIndex << std::endl;
			//~ std::cout << "z = " << formingMetallicity << std::endl;
			//~ std::cout << "lifetime = " << SLF(i,formingMetallicity) << " steps = " << SLF(i,formingMetallicity) * Param.Meta.TimeStep << "Gyr" << std::endl;
			//~ std::cout << "m = " << m << std::endl;
			//~ std::cout << "Previous index = " << prevIndex << std::endl;
			//~ std::cout << "Non-monotonic lifetime generated?" << std::endl;
			//~ exit(5);
			deathIndex = prevIndex;
		}
		//~ std::cout << deathIndex << std::endl;
		Distribution[i] = IsoMass(nStars,i,formingMetallicity, timeIndex,deathIndex);
		//~ std::cout << "Assigned to distribution" <<std::endl;
		prevIndex = deathIndex;
	}
	
	//the remaining mass gets turned into immortal stars, al of which are assumed to have the minimum mortal mass
	double mInf = Param.Stellar.ImmortalMass;
	
	double effectiveImmortalCount = std::max(0.0,formingMass - budget)*1e9 / mInf;
	//~ std::cout << "Immortal shenanigans" << effectiveImmortalCount << std::endl;
	ImmortalStars = IsoMass(effectiveImmortalCount,mInf,formingMetallicity,timeIndex,1e10);
	
	internal_MassCounter += formingMass;
	DepletionIndex = Param.Stellar.MassResolution -1;
	
	if (std::isnan(internal_MassCounter))
	{
		std::cout << "Encountered critical errror " << std::endl;
	}
	return NStarsFormed;
}

double StellarPopulation::Mass()
{
	
	if (std::isnan(internal_MassCounter))
	{
		std::cout << "Encountered critical errror " << std::endl;
		exit(5);
	}
	
	return internal_MassCounter;
}
bool StellarPopulation::Active()
{
	return !IsDepleted;
}
void StellarPopulation::Death(int time, GasReservoir & temporalYieldGrid, RemnantPopulation & remnants, GasReservoir & birthGas, StarEvents & eventRate)
{
	if (IsLifetimeMonotonic)
	{
		MonotonicDeathScan(time,temporalYieldGrid, remnants,birthGas, eventRate);
	}
	else
	{
		FullDeathScan(time);
	}
}
void StellarPopulation::MonotonicDeathScan(int time, GasReservoir & temporalYieldGrid, RemnantPopulation & remnants, GasReservoir & birthGas, StarEvents & eventRate)
{
	while ( (Distribution[DepletionIndex].DeathIndex <= time || Distribution[DepletionIndex].Count == 0) && DepletionIndex >= 0)
	{

		//recover population information
		double nStars = Distribution[DepletionIndex].Count;
		int massID = Distribution[DepletionIndex].MassIndex;
		
		double starMass = Param.Stellar.MassGrid[massID];
	
		int birthID = Distribution[DepletionIndex].BirthIndex;
		double z = Distribution[DepletionIndex].Metallicity;
		 
		 
		 if (nStars > 0)
		 {
			double stellarMassReleased = nStars * starMass;
			Distribution[DepletionIndex].Count = 0;
			double gasMassReclaimed = stellarMassReleased / 1e9;
			internal_MassCounter -= gasMassReclaimed;
	
			RemnantOutput newRem;
			
			if (starMass >= Param.Yield.CCSN_MassCut)
			{
				//~ std::cout << "CCSN Event: n = " << nStars << " m = " << Param.Stellar.MassGrid[massID] << "  z = " << z << std::endl;
				newRem = CCSNYield(temporalYieldGrid,nStars,massID,z,birthID,birthGas);
				eventRate.CCSN += nStars;
			}
			else if (starMass >= Param.Yield.ECSN_MassCut)
			{
				newRem.Type = NeutronStar;
				newRem.Mass = nStars * starMass;
			}
			else
			{
				newRem = AGBYield(temporalYieldGrid,nStars,massID,z,birthID,birthGas);
				eventRate.AGBDeaths += nStars;
			}
			remnants.Feed(birthID,newRem);
		}
		
		--DepletionIndex;
	}
	
	
	bool remainingStarsOutliveSimulation = Distribution[DepletionIndex].DeathIndex > Param.Meta.SimulationSteps;
	if (DepletionIndex < 0 || remainingStarsOutliveSimulation)
	{
		IsDepleted = true;
	}
	
	
	//~ GasStream output(CCSN,creation,Param.Thermal.HotInjection_CCSN);

}
void StellarPopulation::FullDeathScan(int time)
{
	throw std::runtime_error("You are calling death on a non-monotonic population, but this functionality does not exist yet!");
}
