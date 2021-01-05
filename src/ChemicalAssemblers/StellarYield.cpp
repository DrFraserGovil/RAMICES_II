#include "StellarYield.h"
#include "YieldRidge.h"
StellarYield::StellarYield()
{
	//default initializer?
}

StellarYield::StellarYield(Options * opts,int ElementalID)
{
	Opts = opts;
	GridSize = 150;
	Element = ElementalID;
	Yield = std::vector<std::vector<double>>(GridSize, std::vector<double>(GridSize,0.0));	
}

void StellarYield::PrintRidges()
{
	std::string chemFile = Opts->Simulation.FileRoot + "ChemicalRidges/";
	
	mkdirReturn checker = mkdirSafely(chemFile);
	
	log(3) << checker.Message;
	if (checker.Successful == false)
	{
		exit(2);
	}
	
	std::string fileID;
	if (Element < Opts->Element.ElementNames.size())
	{
		fileID = Opts->Element.ElementNames[Element];
	}
	else
	{
		if (Element == -1)
		{
			fileID = "RelicMass";
		}
		else
		{
			fileID = "RelicType";
		}
	}
	std::string ridgeFile = chemFile + fileID + ".dat";
	
	std::fstream file;
	file.open(ridgeFile,std::fstream::out ); 
	for (YieldRidge ridge : Ridges)
	{
		for (int i = 0; i < ridge.Points.size(); ++i)
		{
			file << ridge.Points[i].Mass << "\t" << ridge.Z << "\t" << ridge.Points[i].Yield << "\t" << ridge.SourceID << "\n";
		}
	}
	file.close();	
}

//you MUST make sure that the forward AND backward extrapolations of indices match!
double StellarYield::MFromIndex(int index)
{
	double factor = (double)index/(GridSize - 1);
	return factor*factor *(Opts->Stellar.MaxMass - Opts->Stellar.MinMass) + Opts->Stellar.MinMass;
}

int StellarYield::IndexFromM(double M)
{
	double massDistance = (M - Opts->Stellar.MinMass)/(Opts->Stellar.MaxMass - Opts->Stellar.MinMass);
	double fracIndex = (GridSize - 1) * sqrt( massDistance);
	
	double index = round(fracIndex);
	if (index >= GridSize)
	{
		log(2) << "A mass index greater than grid size was called. Defaulting to max grid size" ;
		index = GridSize - 1;
	}
	if (index < 0)
	{
		log(2) << "A mass index less than zero was called. Defaulting to 0" ;
		index = 0;
	}
	return index;
}

double StellarYield::ZFromIndex(int index)
{
	if (index == 0)
	{
		return 0;
	}
	//this correction is to ensure that Z = 0 is included in the otherwise-log scaled grid
	double powerfactor = (double)(index-1)/(GridSize - 2);
	double basefactor = Opts->Stellar.MaxZ / Opts->Stellar.MinZ;
	
	return std::pow(basefactor,powerfactor) * Opts->Stellar.MinZ;
}

int StellarYield::IndexFromZ(double Z)
{
	if (Z < Opts->Stellar.MinZ)
	{
		return 0;
	}
	if (Z >= Opts->Stellar.MaxZ)
	{
		return GridSize - 1;
	}
	double metDistance = log(Z/Opts->Stellar.MinZ) / log(Opts->Stellar.MaxZ / Opts->Stellar.MinZ);
	
	//offset of one is because i = 0 corresponds to Z = 0, and i = 1 corresponds to Z_min, since logarithms cannot go to zero!
	int ID = 1 + round((GridSize - 1)* metDistance);
	
	if (ID >= GridSize - 1)
	{
		return GridSize -1;
	}
	return ID;
}

double StellarYield::GrabYield(double M, double Z)
{
	int MIndex = IndexFromM(M);
	int ZIndex = IndexFromZ(Z);
	
	return Yield[MIndex][ZIndex];
}

double StellarYield::GrabYield(int mIndex, int zIndex)
{
	return Yield[mIndex][zIndex];
}

void StellarYield::PrepareGrids()
{
	if (Element < Opts->Element.ElementNames.size() )
	{
		log(3) << "\t\tInterpolating " + Opts->Element.ElementNames[Element] + " Grid\n";
	}
	FilterRidges();
	
	InterpolateGrid();
	
	SmoothGrid();
	
}

bool isSharedMetal(double Z, double Reference)
{
	const double checkSensitivity = 0.1;
	
	double solFactor = Z / Reference;
	
	return ( solFactor < (1.0 + checkSensitivity) ) && (  solFactor > (1.0 - checkSensitivity)  );
}

void StellarYield::FilterRidges()
{
	//Filtering ridges means checking for any overlaps + removing excess data
	
	std::string name = "Relics";
	if (Element < Opts->Element.ElementNames.Size() )
	{
		name = Opts->Element.ElementNames[Element];
	}
	if (Ridges.size() == 0)
	{
		log(3) << "\nWARNING: The yield grid associated with " + name + " has not been assigned any data ridges. The yield grid will default to zero yield at all points\n";
		return;
	}
	
	if (Ridges.size() == 1)
	{
		log(3) << "\nWARNING: The yield grid associated with " +  name + " has been assigned only a single data ridge. This means the results will be metallicity independent\n";
		return;
	}
	sort(Ridges.begin(), Ridges.end(), [](const YieldRidge& a, const YieldRidge& b) {  return a.Z < b.Z;});

	
	

	std::vector<YieldRidge> FinalisedRidges;
	
	
	for (int i = 0; i < Ridges.size() - 1; ++i)
	{
		
		if (Ridges[i].Merged == false)
		{
			double baseZ = Ridges[i].Z;
			std::vector<YieldRidge> ConcatenateRepository = {Ridges[i]};
			
			for (int j = i + 1; j < Ridges.size(); ++j)
			{
				double compZ = Ridges[j].Z;
				
				if (isSharedMetal(baseZ, compZ))
				{
					ConcatenateRepository.push_back(Ridges[j]);
					Ridges[j].Merged = true;
				}
			}
			
			if (ConcatenateRepository.size() > 1)
			{
				YieldRidge merged = MergeRidges(ConcatenateRepository, Opts);
				Ridges[i] = merged;
			}
			
			FinalisedRidges.push_back(Ridges[i]);
		}
	}
	
	Ridges = FinalisedRidges;
}

void StellarYield::InterpolateGrid()
{
	for (int mIndex = 0; mIndex < GridSize; ++mIndex)
	{
		
		double mass = MFromIndex(mIndex);
		for (int zIndex = 0; zIndex < GridSize; ++zIndex)
		{
			double Z = ZFromIndex(zIndex);
			
			
			YieldBracket bracket = FindBracket(mass, Z);
			
			if (bracket.isEnclosed)
			{
				double topYield = bracket.UpperRidge.MassInterp(mass,Opts);
				double lowYield = bracket.LowerRidge.MassInterp(mass,Opts);
				
				
				bool logInterpActive = false;
				double interpolatedYield = 0;
				
				double bruchFactor = (Z - bracket.LowerRidge.Z)/(bracket.UpperRidge.Z - bracket.LowerRidge.Z);
				
				if (logInterpActive)
				{
					interpolatedYield = lowYield * pow(topYield/lowYield, bruchFactor);
				}
				else
				{
					interpolatedYield = lowYield + (topYield - lowYield) * bruchFactor;
				}
				
				Yield[mIndex][zIndex] = interpolatedYield;
			}
			else
			{
				if (bracket.hasSingle)
				{
					YieldRidge singleRidge = bracket.UpperRidge;
					double yield = singleRidge.MassInterp(mass,Opts);
					Yield[mIndex][zIndex] = yield;
				}
			}
		}
	}
}

YieldBracket StellarYield::FindBracket(double mass, double Z)
{
	YieldBracket bracket;
	
	double upperFudge = 30;
	double lowerFudge = 2;
	
	//~ if (Element == Opts->Element.IronID)
	//~ {
		//~ lowerFudge = 4;
	//~ }
	if (Element == Opts->Element.MagnesiumID)
	{
		upperFudge = 100;
	}
	
	
	bool hasLower = false;
	bool hasUpper = false;
	
	bool hasFudgedUpper = false;
	bool hasFudgedLower = false;
	
	int previousRidgeID = -1;
	int savedRidgeID = -1;
	int fudgedLow = -1;
	int fudgedTop = -1;
	for (int i = 0; i < Ridges.size(); ++i)
	{

		double topMass = Ridges[i].Points[Ridges[i].Points.size()-1].Mass;
		double lowMass = Ridges[i].Points[0].Mass;
		
		bool isInMassRange = ( mass < topMass ) && (mass > lowMass);
		
		double fudgedTopMass = topMass + upperFudge;
		double fudgedLowerMass = lowMass - lowerFudge;
		
		bool isInFudgedRange = ( mass < fudgedTopMass ) && (mass > fudgedLowerMass);
		
		if (isInMassRange || isInFudgedRange)
		{
			if (Ridges[i].Z > Z)
			{
				if(isInMassRange)
				{
					savedRidgeID = i;
					hasUpper = true;
					
					i = Ridges.size();
				}
				else
				{
					fudgedTop = i;
					hasFudgedUpper = true;
				}
			}
			else
			{
				if (isInMassRange)
				{
					previousRidgeID = i;
					hasLower = true;
				}
				else
				{
					fudgedLow = i;
					hasFudgedLower = true;
				}
			}
		}
	}
	
	
	if ( hasUpper == false && hasFudgedUpper)
	{
		hasUpper = true;
		savedRidgeID = fudgedTop;
	}
	if( hasLower == false && hasFudgedLower)
	{
		hasLower = true;
		previousRidgeID = fudgedLow;
	}
	
	
	
	if (hasUpper && hasLower)
	{
		YieldRidge topRidge = Ridges[savedRidgeID];
		YieldRidge lowRidge = Ridges[previousRidgeID];
		
		bracket.isEnclosed = true;
		bracket.LowerRidge = lowRidge;
		bracket.UpperRidge = topRidge;
	}
	
	
		
	if (hasUpper & !(hasLower))
	{
		bracket = YieldBracket(Ridges[savedRidgeID]);
	}
	if (hasLower & !(hasUpper))
	{
		bracket = YieldBracket(Ridges[previousRidgeID]);
	}
	return bracket;
}

void StellarYield::SmoothGrid()
{
	double smoothingLength = 2;
	
	std::vector<std::vector<double>> smoothedYield = Yield;
	
	for (int mIndex = 0; mIndex < GridSize; ++mIndex)
	{
		for (int zIndex = 0; zIndex < GridSize; ++zIndex)
		{
			int nSmoothers = 0;
			double meaner = 0;
			bool anyAdded = false;
			for (int modM = mIndex - smoothingLength; modM <= mIndex + smoothingLength; ++modM)
			{
				
				for (int modZ = zIndex - smoothingLength; modZ <= zIndex + smoothingLength; ++ modZ)
				{
					bool validZ = (modZ >=0 ) && (modZ < GridSize);
					bool validM = (modM >=0) && (modM < GridSize);
					
					if (validZ && validM)
					{
						meaner += Yield[modM][modZ];
						anyAdded = true;
						++nSmoothers;
					}
					
					
				}
				
				
			}
			smoothedYield[mIndex][zIndex] = meaner/nSmoothers;
		}
	}
	Yield = smoothedYield;
}

int relicType(double mass, double z)
{
	int type;
	if (mass <= 3.2)
	{
		type = 0;
	}
	
	if (3.2 < mass && mass < 8.5)
	{
		type = 3;
	}
	if (8.5 <= mass && mass < 40)
	{
		type = 1;
	}
	if (mass >= 40)
	{
		type = 2;
	}
	return type;
}

void StellarYield::PrepareTypeGrid()
{
	for (int mIndex = 0; mIndex < GridSize; ++mIndex)
	{
		double m = MFromIndex(mIndex);
		for (int zIndex = 0; zIndex < GridSize; ++zIndex)
		{
			double z = ZFromIndex(zIndex);
			Yield[mIndex][zIndex] = relicType(m,z);
		}
		
	}
	
}
