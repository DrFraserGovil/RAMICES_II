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

RemnantOutput YieldGrid::operator()(GasReservoir & scatteringReservoir, double Nstars, int mass, double z, int birthIndex, GasReservoir & birthReservoir) const
{
	return StellarInject(scatteringReservoir, Nstars, mass, z, birthIndex, birthReservoir);
}


RemnantOutput YieldGrid::StellarInject( GasReservoir & scatteringReservoir,  double Nstars, int mass, double z, int birthIndex, GasReservoir & birthReservoir) const
{
	//~ z = 0;

	int massSpoof = mass;

	if (mass - MassOffset < 0)
	{
		throw std::runtime_error("You have called a yield injection on a star which is outside the scope of this yield grid - likely you have asked for the CCSN from a low mass star");
	}
	double logZ = std::max(log10(z),Param.Stellar.MinLogZ.Value);
	int closestMetallicityID = round((logZ - Param.Stellar.MinLogZ)/Param.Stellar.LogZDelta);
	closestMetallicityID = std::min(closestMetallicityID, Param.Stellar.LogZResolution-1);
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
	double remnantMassUp = Grid[massSpoof-MassOffset][upID][RemnantLocation];
	double remnantMassDown = Grid[massSpoof-MassOffset][downID][RemnantLocation];
	double remnantMass = remnantMassDown + interpolationFactor * (remnantMassUp - remnantMassDown);
	
	double ejectaMass = Nstars * (initMass - remnantMass); 
	
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
				double upSynth = Grid[massSpoof - MassOffset][upID][elem];
				double downSynth = Grid[massSpoof - MassOffset][downID][elem];
				synthesisFraction = downSynth + (upSynth - downSynth) * interpolationFactor;
				
			}
			double outputFraction = std::max(0.0,birthFraction + synthesisFraction);
			double massOfElem = ejectaMass * outputFraction / 1e9;
			
			chunk.Cold(elem) = massOfElem * hotInjectionFraction;
			chunk.Hot(elem) = massOfElem * (1.0 - hotInjectionFraction);			
		}
		
		scatteringReservoir.AbsorbMemory(birthIndex, chunk);
	}
	
	//deal with remnants
	RemnantOutput output;
	if (initMass > Param.Yield.Collapse_MassCut)
	{
		output.Type = BlackHole;
		//~ std::cout << "BH with " << initMass << std::endl;
	}
	else if (initMass > Param.Yield.ECSN_MassCut)
	{
		output.Type = NeutronStar;
		//~ std::cout << "NS with " << initMass << std::endl;
	}
	else if (initMass > Param.Yield.CODwarf_MassCut)
	{
		output.Type = CODwarf;
		//~ std::cout << "CO with " << initMass << std::endl;
	}
	else
	{
		output.Type = DormantDwarf;
		//~ std::cout << "Dormant with " << initMass << std::endl;
	}
	
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
	SourcePriority[Orfeo] = 2;
	SourcePriority[Limongi] = 1;
	SourcePriority[Maeder] = 3;
	
	LoadOrfeoYields();
	LoadLimongiYields();
	LoadMaederYields();

	CreateGrid();
	SaveGrid("CCSN");
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
	SaveGrid("AGB");
}

void YieldGrid::CreateGrid()
{
	if (Param.Meta.Verbosity> 0)
	{
		std::cout << "\t\tBeginning grid interpolation\n";
	}
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
					YieldBracket pair = GetBracket(i,mass,z,false);
					if (pair.hasSingle || !pair.isEnclosed)
					{
						YieldBracket pair2 = GetBracket(i,mass,z,true);
						if (pair.hasSingle && !pair2.hasSingle || !pair.isEnclosed)
						{
							pair = pair2;
						}
					}
					if (pair.isEnclosed)
					{
						Grid[mIndex][zIndex][i] = pair.Interpolate(mass,z);
					}
					else
					{
						Grid[mIndex][zIndex][i] = 0;
					}
				}
			}
			
		}
	}
}

YieldBracket YieldGrid::GetBracket(int id, double mass, double z, bool overhanging)
{
	bool hasLower = false;
	bool hasUpper = false;
	YieldRidge lower;
	YieldRidge upper;
	bool lowerLax = false;
	bool upperLax = false;
	
	
	double overhang = Param.Yield.MassOverhang/10;
	
	if (overhanging)
	{
		overhang = Param.Yield.MassOverhang;
	}
	
	
	for (int i = 0; i < RidgeStorage[id].size(); ++i)
	{
		int nPoints = RidgeStorage[id][i].Points.size();
		double lowerRidgeMass = RidgeStorage[id][i].Points[0].Mass;
		double upperRidgeMass = RidgeStorage[id][i].Points[nPoints-1].Mass;
		
		if (MassOffset > 0)
		{
			lowerRidgeMass = std::max(lowerRidgeMass, Param.Yield.CCSN_MassCut.Value);
		}
		else
		{
			upperRidgeMass = std::min(upperRidgeMass, Param.Yield.CCSN_MassCut.Value);
		}
		bool withinMassRange = (lowerRidgeMass - overhang < mass) && (upperRidgeMass + overhang > mass);
		bool withinTightMassRange = (lowerRidgeMass < mass) && (upperRidgeMass > mass);
		if (withinMassRange)
		{
			if (RidgeStorage[id][i].Z > z)
			{
				if (!hasUpper)
				{
					upper = RidgeStorage[id][i];
					hasUpper = true;
					upperLax = false;
					if (!withinTightMassRange)
					{
						upperLax = true;
					}
				}
				else
				{
					bool differentZ = abs(upper.Z - RidgeStorage[id][i].Z)/upper.Z > 0.05;
					bool closer = abs(upper.Z - z) > abs(RidgeStorage[id][i].Z - z);
					bool higherPriority = SourcePriority[RidgeStorage[id][i].Source] > SourcePriority[upper.Source];
					bool tighterThanCurrent = (withinTightMassRange && upperLax);
					if ( (differentZ || higherPriority || (tighterThanCurrent)) && (closer ))
					{
						upperLax = false;
						upper = RidgeStorage[id][i];
						if (!withinTightMassRange)
						{
							upperLax = true;
						}

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
					bool differentZ = abs(lower.Z - RidgeStorage[id][i].Z)/lower.Z > 0.03;
					bool closer = abs(lower.Z - z) > abs(RidgeStorage[id][i].Z - z);
					bool higherPriority = SourcePriority[RidgeStorage[id][i].Source] > SourcePriority[lower.Source];
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

	
	return output;
}

void YieldGrid::SaveGrid(std::string name)
{
	if (Param.Meta.Verbosity > 0)
	{
		std::cout << "\t\tBeginning filesave" << std::endl;
	}
	std::string fileName = Param.Output.YieldSubdir.Value + name + "_yields.dat";
	JSL::initialiseFile(fileName);
	
	std::stringstream output;
	output << "Mass, logZ";
	for (int i = 0; i < ElementCount;++i)
	{
		output << ", " << Param.Element.ElementNames[i];
	}
	output << ", RemnantFraction\n";
	for (int mIndex = 0; mIndex < Grid.size(); ++mIndex)
	{
		double mass = Param.Stellar.MassGrid[mIndex + MassOffset];
		//~ std::cout << mIndex << "  " << MassOffset << "  " << mass <<std::endl;
		for (int zIndex = 0; zIndex < Param.Stellar.LogZResolution; ++zIndex)
		{
			double logz = Param.Stellar.LogZGrid[zIndex];
			output << mass << ", " << logz;
			for (int i = 0; i < ElementCount; ++i)
			{
				output << ", " << Grid[mIndex][zIndex][i];
			}
			output << ", " << Grid[mIndex][zIndex][RemnantLocation] << "\n";
		
			//~ YieldBracket pair = GetBracket(i,mass,z);
			//~ Grid[i][mIndex][zIndex] = pair.Interpolate(mass,z);
		}
	}
	JSL::writeStringToFile(fileName,output.str());
	
	for (int i = 0; i < RidgeStorage.size(); ++i)
	{
		std::string subname;
		if (i < ElementCount)
		{
			subname = Param.Element.ElementNames[i];
		}
		else
		{
			subname = "Remnant";
		}
		subname = Param.Output.YieldSubdir.Value + subname + "_ridges_" + name + ".dat";
		JSL::initialiseFile(subname);
		std::stringstream output2;
		output2 << "Mass, logZ, Value\n";
		for (int j = 0; j < RidgeStorage[i].size(); ++j)
		{
			for (int m = 0; m < RidgeStorage[i][j].Points.size(); ++m)
			{
				output2 << RidgeStorage[i][j].Points[m].Mass << ", " << log10(RidgeStorage[i][j].Z) << ", " << RidgeStorage[i][j].Points[m].Yield <<"\n";
			}
		}
		JSL::writeStringToFile(subname,output2.str());
	}
}
