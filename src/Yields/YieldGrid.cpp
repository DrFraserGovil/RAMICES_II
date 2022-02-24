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
		case ECSN:
		{
			ECSN_Initialise();
			break;
		}
		default:
		{
			throw std::runtime_error("You have tried to initialise a yield grid for which there is no rule to create - ID = " +std::to_string(Process) + "...I am forced to quit");
		}
	}
	//clear the data to reduce memory usage
	RidgeStorage.resize(0);
}

RemnantOutput YieldGrid::operator()(GasReservoir & scatteringReservoir, double Nstars, int mass, double z, const std::vector<GasStream> & birthGas) const
{
	return StellarInject(scatteringReservoir, Nstars, mass, z, birthGas);
}

Interpolator YieldGrid::MetallicityInterpolation(double z) const
{
	double logZ = std::max(log10(z),Param.Stellar.MinLogZ.Value);
	int closestMetallicityID = round((logZ - Param.Stellar.MinLogZ)/Param.Stellar.LogZDelta); // critical assumption that logZ grid is uniform!
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
	
	Interpolator output;
	output.LowerID = downID;
	output.UpperID = upID;
	output.LinearFactor = interpolationFactor;
	
	//~ std::cout<< "Yield requested for " << z << "I compute " << downLogZ << "  < " << logZ << " < " << upLogZ << " with factor " << interpolationFactor<< std::endl;
	return output;
}

void YieldGrid::ElementProduction(ElementID element, double synthesisFraction, double ejectaMass,std::vector<GasStream> & output, const std::vector<GasStream> & birthStreams) const
{
	double birthReservoirMass = 0;
	for (int p = 0 ; p < ProcessCount;++p)
	{
		SourceProcess proc = (SourceProcess)p;
		birthReservoirMass += birthStreams[proc].ColdMass();
	}
	for (int p = 0 ; p < ProcessCount;++p)
	{
		SourceProcess proc = (SourceProcess)p;
		double initialProcessMass = birthStreams[proc].ColdMass();
		
		double birthFraction = 0;
		if (initialProcessMass > 0)
		{
			birthFraction = birthStreams[proc].Cold(element) / birthReservoirMass;
		}
		double returnFraction = birthFraction + (proc == Process) * synthesisFraction; // double check this line works!
		double massReturned = ejectaMass * returnFraction / 1e9;
		output[proc].Cold(element) = massReturned * (1.0-hotInjectionFraction);
		output[proc].Hot(element) = massReturned * hotInjectionFraction;	
	}
	
}

void YieldGrid::ElementDestruction(ElementID element, double synthesisFraction, double ejectaMass,std::vector<GasStream> & output, const std::vector<GasStream> & birthStreams) const
{
	//~ if (element == Oxygen)
		//~ std::cout << "I am destroying " << Param.Element.ElementNames[element] << " with fraction " << synthesisFraction << std::endl;
	double totalElemMass = 0;
	double totalMass = 0;
	
	for (int p = 0 ; p < ProcessCount;++p)
	{
		SourceProcess proc = (SourceProcess)p;
		totalMass += birthStreams[proc].ColdMass();
		totalElemMass += birthStreams[proc].Cold(element);
	}
	double birthFraction_noStreaming = totalElemMass / totalMass;
	double deathFraction_noStreaming = birthFraction_noStreaming + synthesisFraction;
	if (deathFraction_noStreaming < 0)
	{
		if (element == Oxygen)
		{
			std::cout << "I am trying to destroy more than is present of " << Param.Element.ElementNames[element] << " since I was born with " << birthFraction_noStreaming << " and synthesised " << synthesisFraction << " this will most likely cause a mass deficit" <<std::endl;
		}
		deathFraction_noStreaming = 0;
	}
	double outputMass = deathFraction_noStreaming * ejectaMass / 1e9;
	for (int p = 0 ; p < ProcessCount;++p)
	{
		SourceProcess proc = (SourceProcess)p;
		double initialWeighting = birthStreams[proc].Cold(element)/ totalElemMass;
		double massReturned = initialWeighting * outputMass;
		output[proc].Cold(element) = massReturned * (1.0-hotInjectionFraction);
		output[proc].Hot(element) = massReturned * hotInjectionFraction;	
		
	}
}


RemnantOutput YieldGrid::StellarInject( GasReservoir & scatteringReservoir,  double Nstars, int mass, double z, const std::vector<GasStream> & birthGas) const
{
	if (mass - MassOffset < 0)
	{
		throw std::runtime_error("You have called a yield injection on a star which is outside the scope of this yield grid - likely you have asked for the CCSN from a low mass star");
	}
	
	Interpolator LogZ = MetallicityInterpolation(z);
	double initMass = Param.Stellar.MassGrid[mass];

	double remnantMass =  initMass * LogZ.Interpolate(Grid[mass-MassOffset][LogZ.LowerID][RemnantLocation], Grid[mass-MassOffset][LogZ.UpperID][RemnantLocation]);
	//~ std::cout << "For a star of mass " << initMass << "   " << remnantMass << " is in the remnant" << std::endl;
	double ejectaMass = Nstars * (initMass - remnantMass); 
	

	std::vector<GasStream> chunkCatcher;
	for (int p = 0; p < ProcessCount; ++p)
	{
		SourceProcess proc = (SourceProcess)p;
		GasStream chunk(proc);
		chunkCatcher.push_back(chunk);
	} 
	for (int e = 0; e < ElementCount; ++e)
	{
		ElementID elem = (ElementID)e;
		double upSynth = Grid[mass - MassOffset][LogZ.UpperID][elem];
		double downSynth = Grid[mass - MassOffset][LogZ.LowerID][elem];
		double synthesisFraction = LogZ.Interpolate(downSynth,upSynth);
		
		if (synthesisFraction >= 0)
		{
			ElementProduction(elem,synthesisFraction,ejectaMass,chunkCatcher,birthGas);
		}
		else
		{
			if (elem == Hydrogen || elem == Helium)
			{
				ElementDestruction(elem,synthesisFraction,ejectaMass,chunkCatcher,birthGas);
			}
		}
		
		if (elem == Oxygen)
		{
			//~ std::cout << "For a star with mass " << Param.Stellar.MassGrid[mass] << " and logZ = " << log10(z) << " my net synthesis frac is = " << synthesisFraction << std::endl;
		}
	}
	
	double xSum = 0;
	double ySum = 0;
	double zSum = 0;
	for (int p = 0; p < ProcessCount; ++p)
	{
		scatteringReservoir.Absorb(chunkCatcher[p]);
		xSum += chunkCatcher[p].Cold(Hydrogen) + chunkCatcher[p].Hot(Hydrogen);
		ySum += chunkCatcher[p].Cold(Helium) + chunkCatcher[p].Hot(Helium);
		zSum += chunkCatcher[p].Cold(Metals) + chunkCatcher[p].Hot(Metals);
	}
	xSum *= 1e9;
	ySum *= 1e9;
	zSum *= 1e9;
	
	double abberation = ejectaMass - xSum - ySum - zSum;
	//~ std::cout << "ejecting " << xSum + ySum + zSum << std::endl;
	//~ std::cout << "With an ejecta mass of " << ejectaMass << " I synthesised (X,Y,Z) = ("<< xSum << ", " << ySum << ", " << zSum << ") Mass deficit = " << ejectaMass - xSum - ySum - zSum << std::endl;
	
	//~ if (abs(abberation/ejectaMass) > 0.1)
	//~ {
		//~ std::cout << "\n\nYield abberation detected! X + Y + Z should be 0, I found it to be" << abberation/ejectaMass <<std::endl;
		//~ std::cout << "The most likely cause of this is that the material has become hyper-enriched and I am trying to destroy more hydrogen than is present. The birth registry of this star is:" << std::endl;
		//~ for (int p = 0; p < ProcessCount; ++p)
		//~ {
			//~ std::cout << "Proc " << birthStreams[p].Source << "  had XYZ = " << birthStreams[p].Cold(Hydrogen) << "  " << birthStreams[p].Cold(Helium) << "   " << birthStreams[p].Cold(Metals) << " with mass " << birthStreams[p].ColdMass() << std::endl;
		//~ }
		//~ exit(-20);
	//~ }
	
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
		//~ std::cout << "CO with " << " mass " << remnantMass << " from star with M = " << initMass << " z = " << z << "  remFrac = " << Grid[mass-MassOffset][LogZ.LowerID][RemnantLocation] << " - > " << Grid[mass-MassOffset][LogZ.UpperID][RemnantLocation] << std::endl;
	}
	else
	{
		output.Type = DormantDwarf;
	}
	
	output.Mass = std::max(0.0,Nstars * remnantMass);
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

void YieldGrid::PurityEnforce()
{
	for (int i = 0; i < Grid.size(); ++i)
	{
		for (int j = 0; j < Param.Stellar.LogZResolution; ++j)
		{
			Grid[i][j][Hydrogen] = -1.0 * (Grid[i][j][Metals] + Grid[i][j][Helium]);
		}
	}
}

void YieldGrid::CCSN_Initialise()
{
	double ccsnCut = std::min(Param.Yield.ECSN_MassCut,Param.Yield.CCSN_MassCut); // ensures an effective overlap!
	int mID = 0;
	while (mID < Param.Stellar.MassResolution && Param.Stellar.MassGrid[mID] < ccsnCut)
	{
		++mID;
	}
	MassOffset = mID;
	
	//~ std::cout << "CCSN yield grid accounts for stars > " << Param.Stellar.MassGrid[mID] << std::endl;	
	int ccsnGridSize = Param.Stellar.MassResolution - MassOffset + 2;
	InitialiseLargeGrid(ccsnGridSize, Param.Stellar.LogZResolution);
	hotInjectionFraction = Param.Thermal.HotInjection_CCSN;
	
	std::vector<int> basePriority(SourceCount,0.0);
	basePriority[Orfeo] = 3;
	basePriority[Limongi] = 2;
	basePriority[Maeder] = 1;
	
	SourcePriority = std::vector<std::vector<int>>(RidgeStorage.size(),basePriority);
	
	SourcePriority[Oxygen][Orfeo] = 0;
	SourcePriority[Iron][Limongi] = 0;
	SourcePriority[Carbon][Maeder] = 0;
	SourcePriority[Silicon][Orfeo] = -35;
	SourcePriority[Calcium][Orfeo] = -35;
	SourcePriority[Magnesium][Orfeo] = -35;
	LoadOrfeoYields();
	LoadLimongiYields();
	LoadMaederYields();

	CreateGrid();
	
	PurityEnforce();
	SaveGrid("CCSN");
	
}


void YieldGrid::ECSN_Initialise()
{
	double ecsnCut = Param.Yield.ECSN_MassCut;
	int mUp = 0;
	int mDown = 0;
	for (int i = 0; i < Param.Stellar.MassResolution; ++i)
	{
		
		double m = Param.Stellar.MassGrid[i];
		
		if (m < Param.Yield.ECSN_MassCut)
		{
			++mDown;
		}
		if (m < Param.Yield.CCSN_MassCut)
		{
			++mUp;
		} 
	}
	mDown = std::max(0,--mDown);
	MassOffset = mDown;
	
	int ecsnGridSize = std::max(2,mUp - mDown + 2);
	InitialiseLargeGrid(ecsnGridSize, Param.Stellar.LogZResolution);
	std::string ecsnFile = Param.Resources.YieldRoot.Value + "ECSN_Yield.dat";
	std::vector<double> rawFracs(ElementCount+1,0.0);
	
	double nominalMass = 8.8;
	forLineVectorIn(ecsnFile, ',',
		for (int i = 0; i < ElementCount; ++i)
		{
			if (FILE_LINE_VECTOR[0] == Param.Element.ElementNames[i])
			{
				double val = std::stod(FILE_LINE_VECTOR[1]);
				double solarVal = Param.Element.SolarAbundances[i];
				rawFracs[i] = (val - solarVal) * 8.8;
			}
		}
		
		if (FILE_LINE_VECTOR[1] == "Remnant")
		{
			double val = std::stod(FILE_LINE_VECTOR[1]);
			rawFracs[ElementCount] = val * 8.8;
		}
	
	);
	hotInjectionFraction = Param.Thermal.HotInjection_SNIa;
	
	for (int m = 0; m < ecsnGridSize; ++m)
	{
		double mass = Param.Stellar.MassGrid[m + MassOffset];
		for (int z = 0; z < Param.Stellar.LogZResolution; ++z)
		{
			for (int i = 0; i < RidgeStorage.size(); ++i)
			{
				Grid[m][z][i] = rawFracs[i]/ mass;
			
			}
		}
	}
	
	SaveGrid("ECSN");
	//~ exit(10);
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
	int ccsnGridSize = mID + 2;
	InitialiseLargeGrid(ccsnGridSize, Param.Stellar.LogZResolution);
	hotInjectionFraction = Param.Thermal.HotInjection_AGB;
	

	std::vector<int> basePriority(SourceCount,0.0);
	basePriority[Marigo] = 1;
	basePriority[Maeder] = 2;
	
	SourcePriority = std::vector<std::vector<int>>(RidgeStorage.size(),basePriority);
	
	SourcePriority[Carbon][Maeder] = 0;
	
	
	LoadMarigoYields();
	LoadMaederYields();
	CreateGrid();
	PurityEnforce();
	SaveGrid("AGB");
	//~ exit(5);
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
					
					if (!pair.isEnclosed)
					{
						YieldBracket pair2 = GetBracket(i,mass,z,true);
						if (pair.hasSingle && !pair2.hasSingle || !pair.isEnclosed)
						{
							pair = pair2;
						}
					}
					if (pair.isEnclosed || pair.hasSingle)
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
		SourceID s = RidgeStorage[id][i].Source;
		int sourceMessage = SourcePriority[id][s];
		bool useRidge = (sourceMessage > 0 || (sourceMessage < 0 && mass > abs(sourceMessage)));
		if (useRidge)
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
