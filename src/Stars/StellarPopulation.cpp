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




StellarPopulation::StellarPopulation(InitialisedData & data, int parentRing): Param(data.Param), IMF(data.IMF), SLF(data.SLF), CCSNYield(data.CCSNYield), AGBYield(data.AGBYield), ECSNYield(data.ECSNYield), Data(data)
{
	BirthRadius = parentRing;
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

int StellarPopulation::FormStars(double formingMass, int timeIndex, GasReservoir & formingGas)
{
	double NStarsFormed = IMF.FormationCount(formingMass);
	double formingMetallicity = formingGas.ColdGasMetallicity();
	double budget = 0;
	
	BirthIndex = timeIndex;
	Metallicity = formingMetallicity;
	BirthGas = formingGas.Composition();
	double c = BirthGas[Remnant].ColdMass();
	double d= BirthGas[Accreted].ColdMass();

	
	int prevIndex = timeIndex;
	for (int i = Param.Stellar.MassResolution -1; i >= 0; --i)
	{
		double m = Param.Stellar.MassGrid[i];
		
		double nStars = NStarsFormed * IMF.Weighting(i);
		double nOrig = nStars;
		int truncated = (int)nStars;
		
		double roundingFactor = nStars - truncated;
		double diceRoll = (double)rand() / RAND_MAX;
		if (diceRoll > roundingFactor)
		{
			nStars = truncated;
		}
		else
		{
			nStars = truncated + 1;
		}		
		budget +=  nStars * m/1e9;
		
		int deathIndex = timeIndex + SLF(i,formingMetallicity);
		
		//check for monotonicity
		if (deathIndex < prevIndex)
		{
			deathIndex = prevIndex;
		}
		Distribution[i] = IsoMass(nStars,i,formingMetallicity, timeIndex,deathIndex);
		prevIndex = deathIndex;
	}
	
	//the remaining mass gets turned into immortal stars, al of which are assumed to have the minimum mortal mass
	double mInf = Param.Stellar.ImmortalMass;
	
	double effectiveImmortalCount = std::max(0.0,formingMass - budget)*1e9 / mInf;
	
	ImmortalStars = IsoMass(effectiveImmortalCount,mInf,formingMetallicity,timeIndex,1e10);
	
	internal_MassCounter += formingMass;
	DepletionIndex = Param.Stellar.MassResolution -1;
	
	if (std::isnan(internal_MassCounter))
	{
		std::cout << "Encountered critical errror in stellar formation routine " << std::endl;
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
void StellarPopulation::Death(int time, std::vector<GasReservoir> & temporalYieldGrid, RemnantPopulation & remnants, StarEvents & eventRate)
{
	if (IsLifetimeMonotonic)
	{
		MonotonicDeathScan(time,temporalYieldGrid, remnants, eventRate);
	}
	else
	{
		FullDeathScan(time);
	}
}
void StellarPopulation::MonotonicDeathScan(int time, std::vector<GasReservoir> & temporalYieldGrid, RemnantPopulation & remnants, StarEvents & eventRate)
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
				newRem = CCSNYield(temporalYieldGrid[birthID],nStars,massID,z,BirthGas);
				eventRate.CCSN += nStars;
			}
			else if (starMass >= Param.Yield.ECSN_MassCut)
			{
				//~ std::cout << "ECSN" << std::endl;
				double ecsnStars = Param.Yield.ECSN_Fraction * nStars;
				double ccsnStars = nStars - ecsnStars;
				//~ std::cout << "Out of " << nStars << ", " << ecsnStars << " are ECSN and " << ccsnStars << " are CCSN " << std::endl;
				newRem = ECSNYield(temporalYieldGrid[birthID],ecsnStars,massID,z,BirthGas);
				remnants.Feed(time,newRem); // double feed!
				eventRate.ECSN += ecsnStars;
				//~ std::cout << "ECSN = " << eventRate.ECSN << std::endl;
				eventRate.CCSN += ccsnStars;
				newRem = CCSNYield(temporalYieldGrid[birthID],ccsnStars,massID,z,BirthGas);
			}
			else
			{
				newRem = AGBYield(temporalYieldGrid[birthID],nStars,massID,z,BirthGas);
				eventRate.AGBDeaths += nStars;
			}
			remnants.Feed(time,newRem);
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

std::string StellarPopulation::CatalogueHeaders()
{
	std::string s= "Radius, TrueAge, BirthIndex, BirthRadius, MeasuredAge, Mass, Metallicity";
	for (int i = 1; i < ElementCount; ++i)
	{
		s += ", " + Param.Element.ElementNames[i] + "H";
	}
	for (int i = 0; i < PropertyCount; ++i)
	{
		s+= ", " + PropertyNames[i];
	}
	return s;
}
std::string StellarPopulation::CatalogueEntry(std::vector<int> ns, int m, double currentRadius, double birthRadius) const
{
	int nManualEntries = 7;
	std::vector<double> values(nManualEntries+PropertyCount+ElementCount - 1,0.0);
	values[0] = currentRadius;
	values[1] = Age;
	values[2] = BirthIndex;
	values[3] = birthRadius;
	values[4] = values[1];
	values[5] = Param.Stellar.MassGrid[m];
	values[6] = Metallicity;
	int offset = nManualEntries;
	
	double hContent = 1e-99;
	for (int p = 0; p < ProcessCount; ++p)
	{
		hContent += BirthGas[p].Cold(Hydrogen);
	}
	double hSol = Param.Element.SolarAbundances[Hydrogen];
	for (int i = 1; i < ElementCount; ++i)
	{
		double eContent = 1e-99;
		for (int p = 0; p < ProcessCount; ++p)
		{
			eContent+=BirthGas[p].Cold((ElementID)i);
		}
		//~ std::cout << "Evaluating " << Param.Element.ElementNames[i] << " = " << eContent << " h content = " << hContent << std::endl;
		double logVal = log10(eContent/hContent) - log10(Param.Element.SolarAbundances[(ElementID)i]/hSol);
		//~ std::cout << "Inferred abundances: " << logVal << std::endl;
		values[offset] = logVal;
		++offset;
	}
	int elemOffset = offset;
	std::vector<double> typicalErrors(values.size(),0.0);
	//~ typicalErrors[3] = 1;
	
	int eOffset = nManualEntries;
	//~ for (int i = 0; i < PropertyCount; ++i)
	//~ {
		//~ typicalErrors[eOffset] = abs(0.02 * values[offset]);
		//~ ++eOffset; 
	//~ }
	//~ for (int i = 1; i < ElementCount; ++i)
	//~ {
		//~ typicalErrors[eOffset] = 0.04;
		//~ ++eOffset;
	//~ }
	//~ typicalErrors[eOffset + Iron -1] = 0.04;
	
	
	std::string output = "";
	for (int entry = 0; entry < ns.size(); ++entry)
	{
		int n = ns[entry];

		for (int i = 0; i < PropertyCount; ++i)
		{
			
			const InterpolantPair & e = Distribution[m].Isochrone.Data[entry];
			
			values[elemOffset + i] = e[(IsochroneProperties)i];
			++offset; 
		}
		
		
		for (int star = 0; star < n; ++star)
		{
			std::string line = "";
			for (int k = 0; k < values.size(); ++k)
			{
				
				
				double r = Data.NormalDist();
				double error = typicalErrors[k] * r;
				double val = values[k] + error;
				//~ std::cout << "Given " << values[k] << " and typical value " << typicalErrors[k] << " and random value " << r << " I scattered with error " << error << " giving " <<val << std::endl;
				if (k > 0)
				{
					line += ", ";
				}
				line += std::to_string(val);
			}
			output += line + "\n";
		}
	}
	return output;
}
