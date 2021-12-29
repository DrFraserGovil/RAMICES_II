#include "YieldGrid.h"

YieldGrid::YieldGrid(const GlobalParameters & param, YieldProcess yieldsource): Param(param), Process(param.Yield.ProcessTypes[yieldsource])
{
	if (param.Meta.Verbosity > 0)
	{
		std::cout << "\t" << Param.Yield.ProcessNames[Process] << " yield grid initialising\n" << std::flush;
	}
	MassOffset = 0;
	
	switch(yieldsource)
	{
		case CCSN:
		{
			CCSN_Initialise();
			break;
		}
		case AGB:
		{
			AGB_Initialise();
			break;
		}
		default:
		{
			throw std::runtime_error("You have tried to initialise a yield grid for which there is no rule to create - ID = " +std::to_string(Process) + "...I am forced to quit");
		}
	}
	
	
}

RemnantOutput YieldGrid::operator()(GasReservoir & scatteringReservoir, int Nstars, int mass, double z, int birthIndex, GasReservoir & birthReservoir) const
{
	return StellarInject(scatteringReservoir, Nstars, mass, z, birthIndex, birthReservoir);
}


RemnantOutput YieldGrid::StellarInject( GasReservoir & scatteringReservoir,  int Nstars, int mass, double z, int birthIndex, GasReservoir & birthReservoir) const
{
	if (mass - MassOffset < 0)
	{
		throw std::runtime_error("You have called a yield injection on a star which is outside the scope of this yield grid - likely you have asked for the CCSN from a low mass star");
	}
	double logZ = std::max(log10(z),Param.Stellar.MinLogZ.Value);
	int closestMetallicityID = round((logZ - Param.Stellar.MinLogZ)/Param.Stellar.LogZDelta);
	int upID;
	int downID;
	
	if (logZ >= Param.Stellar.LogZGrid[closestMetallicityID])
	{
		downID = closestMetallicityID;
		upID = std::min(closestMetallicityID+1, Param.Stellar.LogZResolution -1);
		if (upID == downID)
		{
			--downID; 
		}
	}
	else
	{
		upID = closestMetallicityID;
		downID = std::max(0,closestMetallicityID - 1);
		if (upID == downID)
		{
			++upID; 
		}
	}
	double downLogZ = Param.Stellar.LogZGrid[downID];
	double upLogZ = Param.Stellar.LogZGrid[upID];
	double interpolationFactor = (logZ - downLogZ)/(upLogZ - downLogZ);
	
	double initMass = Param.Stellar.MassGrid[mass];
	
	////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
	double remnantMass = 0.3*initMass;
	
	double ejectaMass = Nstars * (initMass - remnantMass); //need to change!
	
	
	const std::vector<GasStream> & birthStreams = birthReservoir.GetHistory(birthIndex);
	for (int p = 0; p < ProcessCount; ++p)
	{
		SourceProcess proc = (SourceProcess)p;
		double initBirthMass = birthStreams[proc].ColdMass()+ 1e-88; //basic offset to prevent zero division
		GasStream chunk(proc);
		for (int e = 0; e < ElementCount; ++e)
		{
			ElementID elem = (ElementID)e;
			double birthFraction = birthStreams[proc].Cold(elem) / initBirthMass;
			double synthesisFraction = 0;
			if (p == Process)
			{
				double upSynth = Grid[mass - MassOffset][upID][elem];
				double downSynth = Grid[mass - MassOffset][downID][elem];
				synthesisFraction = downSynth + (upSynth - downSynth) * interpolationFactor;
			}
			double outputFraction = std::max(0.0,birthFraction + synthesisFraction);
			double massOfElem = ejectaMass * outputFraction / 1e9;
			
			
			chunk.Cold(elem) = massOfElem * hotInjectionFraction;
			chunk.Hot(elem) = massOfElem * (1.0 - hotInjectionFraction);
			//~ GrossOutputStream[birthIndex][proc][elem] += ejectaMass * outputFraction;
		}
		
		scatteringReservoir.AbsorbMemory(birthIndex, chunk);
	}
	
	//deal with remnants
	RemnantOutput output;
	output.Type = WhiteDwarf;
	output.Mass = Nstars * remnantMass;
	return output;
}


void YieldGrid::InitialiseLargeGrid(int mSize, int zSize)
{
	int extraElements = 1; //add one extra element for remnant masses
	int yieldElements = ElementCount + extraElements;
	RemnantLocation = yieldElements -1;
	Grid = std::vector<std::vector<std::vector<double>>>(mSize, std::vector<std::vector<double>>(zSize, std::vector<double>(yieldElements,0.0)));
	RidgeStorage.resize(yieldElements);
}

void YieldGrid::CCSN_Initialise()
{
	double ccsnCut = Param.Yield.CCSN_MassCut;
	int mID = 0;
	while (mID < Param.Stellar.MassResolution && Param.Stellar.MassGrid[mID] < ccsnCut)
	{
		++mID;
	}
	MassOffset = mID;
	int ccsnGridSize = Param.Stellar.MassResolution - MassOffset;
	InitialiseLargeGrid(ccsnGridSize, Param.Stellar.LogZResolution);
	hotInjectionFraction = Param.Thermal.HotInjection_CCSN;
	
	SourcePriority.resize(SourceCount);
	SourcePriority[Orfeo] = 3;
	SourcePriority[Limongi] = 2;
	SourcePriority[Maeder] = 1;
	
	LoadOrfeoYields();
	LoadLimongiYields();
	LoadMaederYields();

	CreateGrid();
}

void YieldGrid::AGB_Initialise()
{
	double ccsnCut = Param.Yield.CCSN_MassCut;
	int mID = 0;
	while (mID < Param.Stellar.MassResolution -1 && Param.Stellar.MassGrid[mID+1] < ccsnCut)
	{
		++mID;
	}
	MassOffset = 0;
	int ccsnGridSize = mID;
	InitialiseLargeGrid(ccsnGridSize, Param.Stellar.LogZResolution);
	hotInjectionFraction = Param.Thermal.HotInjection_AGB;
	
	SourcePriority.resize(SourceCount);
	SourcePriority[Marigo] = 3;
	SourcePriority[Maeder] = 1;
	
	LoadMarigoYields();
	LoadMaederYields();
	CreateGrid();
}

void YieldGrid::CreateGrid()
{
	for (int i = 0; i < RidgeStorage.size(); ++i)
	{
		if (RidgeStorage[i].size() > 0)
		{
			for (int mIndex = 0; mIndex < Grid.size(); ++mIndex)
			{
				double mass = Param.Stellar.MassGrid[mIndex + MassOffset];
				//~ std::cout << mIndex << "  " << MassOffset << "  " << mass <<std::endl;
				for (int zIndex = 0; zIndex < Param.Stellar.LogZResolution; ++zIndex)
				{
					double z = pow(10,Param.Stellar.LogZGrid[zIndex]);
					YieldBracket pair = GetBracket(i,mass,z);
					Grid[i][mIndex][zIndex] = pair.Interpolate(mass,z);
				}
			}
			
		}
	}
}

YieldBracket YieldGrid::GetBracket(int id, double mass, double z)
{
	bool hasLower = false;
	bool hasUpper = false;
	YieldRidge lower;
	YieldRidge upper;
	double overhang = Param.Yield.MassOverhang;
	
	
	
	for (int i = 0; i < RidgeStorage[id].size(); ++i)
	{
		int nPoints = RidgeStorage[id][i].Points.size();
		double lowerRidgeMass = RidgeStorage[id][i].Points[0].Mass - overhang;
		double upperRidgeMass = RidgeStorage[id][i].Points[nPoints-1].Mass + overhang;
		
		if (MassOffset > 0)
		{
			lowerRidgeMass = std::max(lowerRidgeMass, Param.Yield.CCSN_MassCut.Value);
		}
		else
		{
			upperRidgeMass = std::min(upperRidgeMass, Param.Yield.CCSN_MassCut.Value);
		}
		bool withinMassRange = (lowerRidgeMass - overhang < mass) && (upperRidgeMass + overhang > mass);
		if (withinMassRange)
		{
			if (RidgeStorage[id][i].Z > z)
			{
				if (!hasUpper)
				{
					upper = RidgeStorage[id][i];
					hasUpper = true;
				}
				else
				{
					bool differentZ = abs(upper.Z - RidgeStorage[id][i].Z)/upper.Z > 0.03;
					bool closer = abs(upper.Z - z) > abs(RidgeStorage[id][i].Z - z);
					bool higherPriority = SourcePriority[RidgeStorage[id][i].Source] > SourcePriority[upper.Source];
					
					if ( (differentZ || higherPriority) && closer)
					{
						upper = RidgeStorage[id][i];
					}
				}
			}
			else
			{
				if (!hasLower)
				{
					lower = RidgeStorage[id][i];
					hasLower = true;
				}
				else
				{
					bool differentZ = abs(upper.Z - RidgeStorage[id][i].Z)/upper.Z > 0.03;
					bool closer = abs(upper.Z - z) > abs(RidgeStorage[id][i].Z - z);
					bool higherPriority = SourcePriority[RidgeStorage[id][i].Source] > SourcePriority[upper.Source];
					if ((differentZ || higherPriority) && closer)
					{
						lower = RidgeStorage[id][i];
					}
				}
			}
		}
		
	}
	
	
	YieldBracket output;
	if (hasUpper && hasLower)
	{
		output = YieldBracket(upper,lower);
	}
	else if (hasUpper)
	{
		output = YieldBracket(upper);
	}
	else if (hasLower)
	{
		output = YieldBracket(lower);
	}
	else
	{
		throw std::runtime_error("Could not locate a bracket when there should be one....I hate life!");
	}
	
	return output;
}
