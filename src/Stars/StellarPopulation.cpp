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
		//~ double nOrig = nStars;
		//~ int truncated = (int)nStars;
		
		//~ double roundingFactor = nStars - truncated;
		//~ double diceRoll = (double)rand() / RAND_MAX;
		//~ if (diceRoll > roundingFactor)
		//~ {
			//~ nStars = truncated;
		//~ }
		//~ else
		//~ {
			//~ nStars = truncated + 1;
		//~ }		
		budget +=  nStars * m/1e9;
		
		int deathIndex = timeIndex + SLF(i,formingMetallicity);
		// int deathIndexDown;
		// if (i ==0){
		// 	deathIndexDown = deathIndex;
		// }
		// else{
		// 	deathIndexDown = timeIndex + SLF(i-1,formingMetallicity);
		// }
		
		//check for monotonicity
		if (deathIndex < prevIndex)
		{
			deathIndex = prevIndex;
		}
		// if(deathIndexDown<prevIndexDown){
		// 	deathIndexDown = prevIndexDown;
		// }
		// if(deathIndex > deathIndexDown){
		// 	deathIndexDown = deathIndex;
		// }
		Distribution[i] = IsoMass(nStars,i,formingMetallicity, timeIndex,deathIndex);
		prevIndex = deathIndex;
		//prevIndexDown = deathIndexDown;
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
	
			RemnantOutput newRem;
			
			if (starMass >= Param.Yield.CCSN_MassCut)
			{
				double stellarMassReleased = nStars * starMass;
				double gasMassReclaimed = stellarMassReleased / 1e9;
				internal_MassCounter -= gasMassReclaimed;
				Distribution[DepletionIndex].Count = 0;
				//~ std::cout << "CCSN Event: n = " << nStars << " m = " << Param.Stellar.MassGrid[massID] << "  z = " << z << std::endl;
				newRem = CCSNYield(temporalYieldGrid[birthID],nStars,massID,z,BirthGas);
				eventRate.CCSN += nStars;
				--DepletionIndex;
			}
			else if (starMass >= Param.Yield.ECSN_MassCut)
			{
				double stellarMassReleased = nStars * starMass;
				double gasMassReclaimed = stellarMassReleased / 1e9;
				internal_MassCounter -= gasMassReclaimed;
				Distribution[DepletionIndex].Count = 0;
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
				--DepletionIndex;
			}
			else
			{

				int deathIndex = Distribution[DepletionIndex].DeathIndex;
				int deathIndexDown = Distribution[DepletionIndex-1].DeathIndex;
				
				double depletionFraction;
				if(deathIndex == deathIndexDown || deathIndex == deathIndexDown-1){
					depletionFraction = 1.0;
					Distribution[DepletionIndex].Count = 0;
					// std::cout<< time<< " " << deathIndex<< " " << DepletionIndex<< " "<<deathIndexDown<< " " <<nStars*depletionFraction<< " " << nStars<< " " << depletionFraction<< " " << Distribution[DepletionIndex].Count<<"\n";
					--DepletionIndex;
				}
				else{
					depletionFraction = 1.0/(deathIndexDown-time);
					++ Distribution[DepletionIndex].DeathIndex;
					Distribution[DepletionIndex].Count -=nStars*depletionFraction; 
					
					// if(depletionFraction != 1.0){
					// 	std::cout<< time<< " " << deathIndex<< " " << deathIndexDown<< " " <<nStars*depletionFraction<< " " << nStars<< " " << depletionFraction<< " " << Distribution[DepletionIndex].Count<<"\n";
					// }
				}

				//std::cout<< time<< " " << deathIndex<<  " "<<deathIndexDown<< " "<< birthID <<" " << DepletionIndex<< " " <<nStars*depletionFraction<< " " << nStars<< " " << depletionFraction<< " " << Distribution[DepletionIndex].Count<<"\n";

				double starsPerStep = nStars*depletionFraction;

				double stellarMassReleased = starsPerStep * starMass;
				double gasMassReclaimed = stellarMassReleased / 1e9;
				internal_MassCounter -= gasMassReclaimed;

				//std::cout<<time<< " "<<internal_MassCounter<<"\n";


				newRem = AGBYield(temporalYieldGrid[birthID],starsPerStep,massID,z,BirthGas);
				eventRate.AGBDeaths += starsPerStep;

			// 	//if (newRem.Mass <=0.0){
			// 		std::cout<< time<< " "<<newRem.Mass<<"rem\n";
			// //	}
			}
			remnants.Feed(time,newRem);
		}	
		else{
			--DepletionIndex;
		}
		
	}

	bool remainingStarsOutliveSimulation = Distribution[DepletionIndex-1].DeathIndex > Param.Meta.SimulationSteps;
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


	std::string s= "GuidingRadius, TrueAge, BirthIndex, BirthRadius, MeasuredAge, Mass, Metallicity, observedRadius, X_kpc, Y_kpc, Z_kpc, vx_kms, vy_kms, vz_kms, Jr_kpckms, Jz_kpkms, Lz_kpckms";
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


bool RejectStars(double mass, double Age, const coord::PosVelCar &PS, double Mv, double solar_radius)
{

	double x_solar = solar_radius;
	double y_solar = 0.0;
	double z_solar = 0.0;

	//xx give ps_cyl instead of calculating it again?
	double r = std::sqrt(PS.x*PS.x + PS.y*PS.y);
	double phi = std::atan2(PS.y, PS.x);

	double maxDistance = pow(10, (4.0 - Mv)/5);
	double minDistance = pow(10, (2.0 - Mv)/5);
	
	// std::cout<< "maxDistance = " << maxDistance << " minDistance = " << minDistance << "\n";

	double distance3d = std::sqrt( (PS.x - x_solar)*(PS.x - x_solar) + (PS.y - y_solar)*(PS.y - y_solar) + (PS.z - z_solar)*(PS.z - z_solar) );

	if (distance3d < maxDistance && distance3d > minDistance)
	{
		double distance2d = std::sqrt( (PS.x - x_solar)*(PS.x - x_solar) + (PS.y - y_solar)*(PS.y - y_solar) );
		double phi_degree = std::asin(PS.z/distance2d) *180.0/M_PI;
		double phiCut_degree = 10.0;
		if(phi_degree>phiCut_degree){
			// std::cout<< "accepted\n";
			return true;
		}
	}
	// std::cout<< "rejected\n";	
	return false;


}

// ns: numberSynthesised; m: massBin
std::string StellarPopulation::CatalogueEntry(std::vector<int> ns,
											  int m,
											  double currentRingRadius,
											  double birthRadius, 
											  double age, 
											  const potential::PtrPotential& pot, 
											  const units::InternalUnits& unit, 
											  std::vector<double> Mv_vec,
											  int& numberAccepted) const
{
	int nManualEntries = 17;
	std::vector<double> values(nManualEntries+PropertyCount+ElementCount - 1,0.0);
	
	double currentGuidingRadius = currentRingRadius - Param.Galaxy.RingWidth[0]/2.0;
	currentGuidingRadius += Param.Galaxy.RingWidth[0]*rand()/RAND_MAX;
	values[0] = currentGuidingRadius;
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
	for (int i = 1; i < ElementCount; ++i)
	{
		typicalErrors[eOffset] = 0.02;
		++eOffset;
	}
	//~ typicalErrors[eOffset + Iron -1] = 0.04;
	for (int i = 0; i < PropertyCount; ++i)
	{
		typicalErrors[eOffset] = 0.0;
		++eOffset; 
	}
	//~ for (int i = 1; i < ElementCount; ++i)
	//~ {
		//~ typicalErrors[eOffset] = 0.04;
		//~ ++eOffset;
	//~ }
	//~ typicalErrors[eOffset + Iron -1] = 0.04;
	
	
	// set-up  dynamics:
	double beta = 0.33;

	double scalelength_sigmaz = 5; //kpc
	double scalelength_sigmaR = 7.5; //kpc

	double sigmaz10_R0 = 43; //km s
	double sigmaR10_R0 = 28; //km s

	double sigmaz_min = 7; //km s
	double sigmaR_min = 7; //km s

	double sigmaz10 = sigmaz10_R0 * std::exp(- birthRadius/scalelength_sigmaz);
	double sigmaR10 = sigmaR10_R0 * std::exp(- birthRadius/scalelength_sigmaR);


	double sigmaz = std::max(sigmaz_min, sigmaz10*std::pow((age/10.0), beta) );
	double sigmaR = std::max(sigmaR_min, sigmaR10*std::pow((age/10.0), beta) );

	double nu_z =1;
	double nu_R =1;

	// double dist_z = std::exp(-(nu_z * J_z)/sigmaz);
	//function that samples a random numbers from a distribution accordings to dist_z:
	// std::uniform_real_distribution<double> dist_uni(0,1);
	// std::mt19937_64 generator;

	std::string output = "";
	for (int entry = 0; entry < ns.size(); ++entry)
	{

		// mvVector = Distribution[m].Isochrone.Value(entry, (IsochroneProperties)i);

		//sample dynamics separately for each entry
		double genZ = (double)rand()/RAND_MAX;
		double genR = (double)rand()/RAND_MAX;

		double Jz = - std::log(genZ)*sigmaz/nu_z;
		double JR = - std::log(genR)*sigmaR/nu_R;

		double RadiusGuiding = currentGuidingRadius;

		// add vcirc and calculate Lz as vcirc*RadiusObserved, then sample R according to JR around RadiusObserved
		double vcirc = 245 *RadiusGuiding/(RadiusGuiding+0.5);
		double Lz = vcirc * RadiusGuiding;

		actions::Actions acts;
		acts.Jr = JR * unit.from_Kpc_kms;
		acts.Jz = Jz * unit.from_Kpc_kms;
		acts.Jphi = Lz * unit.from_Kpc_kms;


		//sample angles
		actions::Angles angles;
		angles.thetar = 2*M_PI*(double)rand()/RAND_MAX;
		angles.thetaz = 2*M_PI*(double)rand()/RAND_MAX;
		angles.thetaphi = 2*M_PI*(double)rand()/RAND_MAX;


		actions::ActionMapperTorus mapper(*pot, acts);

		double Mv = Mv_vec[entry];
		coord::PosVelCyl phasespace = mapper.map(actions::ActionAngles(acts, angles));


		double maxDistance = pow(10, (4.0 - Mv)/5);
		
		double deltaAzimuth = 2.0 * atan(maxDistance / Param.Catalogue.SolarRadius);
		double scalingFactor = deltaAzimuth/(2.0 * M_PI);

		phasespace.phi = phasespace.phi*scalingFactor;
		coord::PosVelCar phasespace_cart = coord::toPosVelCar(phasespace);

		double mass = Param.Stellar.MassGrid[m];




		bool starObserved = RejectStars(mass, Age, phasespace_cart, Mv, Param.Catalogue.SolarRadius);

		if (starObserved){

			++numberAccepted;

			values[7] = phasespace.R;
			values[8] = phasespace_cart.x;
			values[9] = phasespace_cart.y;
			values[10] = phasespace_cart.z;
			values[11] = phasespace_cart.vx;
			values[12] = phasespace_cart.vy;
			values[13] = phasespace_cart.vz;
			values[14] = acts.Jr;
			values[15] = acts.Jz;
			values[16] = acts.Jphi;
			

			int n = ns[entry];

			for (int i = 0; i < PropertyCount; ++i)
			{
				
				//~ const InterpolantPair & e = Distribution[m].Isochrone.Data[entry];
				
				values[elemOffset + i] = Distribution[m].Isochrone.Value(entry,(IsochroneProperties)i);
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
	}
	// std::cout<< output;
	return output;
}

// double tan_SkyCut(double theta)
// {

// 	double order0 = 0.715;
// 	double order1 = -1.24 * cos(theta) + 1.915 * sin(theta);
// 	double order2 = -0.114 * cos(2*theta) - 0.2553*sin(2*theta);
	
// 	return order0 + order1 + order2;
// }

