#include "YieldGrid.h"
void YieldGrid::LoadMarigoYields()
{
	//loads data from Marigo (2001)
	log(2) << "\t\tMarigo Yields \n";
	
	//hardcoded properties of the Marigo dataset
	std::vector<int> nMassPointsInFile = {22,23,23};
	
	std::vector<double> metallicityOfFile = {0.004, 0.008, 0.019};
	std::vector<double> heliumOfFile = {0.24,0.25,0.273};
	
	int nElementsPresent = 6;
	
	double mixingParameter = 1.68;
	
	for (int file = 0; file < 3; ++file)
	{		
		double fileZ = metallicityOfFile[file];
		
		std::vector<YieldRidge> Ridges(nElementsPresent, YieldRidge(Opts->Element.MarigoID,fileZ, nMassPointsInFile[file]));
		
		
		std::string fileName = "Resources/ChemicalData/MarigoYields/totyieldsz0";
		if (fileZ < 0.01)
		{
			fileName += "0";
		}
		fileName += std::to_string( (int)(1000*fileZ)) + ".dat";
		
		int lineCount = 0;
		forLineVectorInFile(fileName,' ',
			
			//file contains multiple bits of data, choose the mixing param with the most masses present
			if (std::stod(FILE_LINE_VECTOR[0]) != mixingParameter)
			{
				break;
			} 
			
			double mass = std::stod(FILE_LINE_VECTOR[1]);
			double ejectaMass = std::stod(FILE_LINE_VECTOR[2]);
			
			double hYield = std::stod(FILE_LINE_VECTOR[3]);
			double heYield = std::stod(FILE_LINE_VECTOR[4])+std::stod(FILE_LINE_VECTOR[5]);
			double metYield = std::stod(FILE_LINE_VECTOR[13]);
			
			double cYield = std::stod(FILE_LINE_VECTOR[6])+std::stod(FILE_LINE_VECTOR[7]);
			double oYield = std::stod(FILE_LINE_VECTOR[10])+std::stod(FILE_LINE_VECTOR[11]) + +std::stod(FILE_LINE_VECTOR[11]);
			
			double remnantMass = mass - ejectaMass;
			
			
			//manual ids for this grouping
			std::vector<double> yields = {hYield, heYield, metYield, cYield, oYield, remnantMass * ejectaMass / mass};
						
			for (int i = 0; i < nElementsPresent; ++i)
			{
				double yield =  yields[i] / ejectaMass;
				Ridges[i].Points[lineCount] = YieldPoint(mass, yield);
			}
		
			++lineCount;
		);
		std::vector<double> ids = {Opts->Element.HydrogenID, Opts->Element.HeliumID, Opts->Element.MetalsID, Opts->Element.CarbonID, Opts->Element.OxygenID};
		
		for (int i = 0; i < ids.size(); ++i)
		{
			Element[ids[i]].Ridges.push_back(Ridges[i]);
		}
		
		RelicMass.Ridges.push_back(Ridges[Ridges.size() - 1]);
	}
	
}

void YieldGrid::LoadOrfeoYields()
{
	//loads data from Limongi & Chieffi (2004)
	log(2) << "\t\tOrfeo Yields \n";
	
	//hardcoded stuff from datafile
		
	int nMassPoints = 15;
	double x = 0.7;
	double z = 0.0198;
	double y = 0.28;	
	
	int nElementsPresent = 12;
	std::vector<double> orfeoSolarValues(nElementsPresent,0.0);
	
	orfeoSolarValues[Opts->Element.HydrogenID] = x;
	orfeoSolarValues[Opts->Element.HeliumID] = y;
	orfeoSolarValues[Opts->Element.MetalsID] = z;
	orfeoSolarValues[Opts->Element.IronID] = 0.0012682823;
	orfeoSolarValues[Opts->Element.CarbonID] = 0.003058221;
	orfeoSolarValues[Opts->Element.OxygenID] = 0.009585329;
	orfeoSolarValues[Opts->Element.MagnesiumID] = 0.0006578465;
	orfeoSolarValues[Opts->Element.CalciumID] = 0.00006176178;
	orfeoSolarValues[Opts->Element.SiliconID] = 0.0007083979;
	orfeoSolarValues[Opts->Element.ManganeseID] = 0.000013245451;
	orfeoSolarValues[Opts->Element.ChromiumID] = 1.77185127e-05;
	orfeoSolarValues[Opts->Element.CobaltID]= 3.3467360e-06;
	
	std::vector<double> masses = {11.0,12.0,13.0,14.0,15.0,16.0,17.0,20.0,25.0,30.0,35.0,40.0,60.0,80.0,120.0};
	
	//matches elements in the file to their internal ids

	std::vector<int> elementsPresent = {Opts->Element.HydrogenID, Opts->Element.HeliumID, Opts->Element.MetalsID, Opts->Element.IronID, Opts->Element.CarbonID, Opts->Element.OxygenID, Opts->Element.MagnesiumID, Opts->Element.SiliconID, Opts->Element.CalciumID, Opts->Element.ManganeseID, Opts->Element.ChromiumID, Opts->Element.CobaltID,};
	std::vector<int> elementMasses = {1,2,-1,26,8,12,6,14,20,25,24,27};
	
	std::vector<YieldRidge> Ridges = std::vector<YieldRidge>(elementsPresent.size(), YieldRidge(Opts->Element.OrfeoID,z, masses.size()));
	YieldRidge	RemnantRidge = YieldRidge(Opts->Element.OrfeoID,z,masses.size());
	
	for (int i = 0; i < masses.size() ; ++i)
	{
		int m = masses[i];
		std::string fileName = "Resources/ChemicalData/OriginalOrfeo/z22m";
		
		if (m < 100)
		{
			fileName += "0";
		}
	
		fileName += std::to_string(m) + "elecum.ngms";
		
		//must search through the file for the point which gives the correct NI56 yield
		std::vector<std::string> highNiLine;
		std::vector<std::string> lowNiLine;
		double targetNickelYield = 0.1;
		
		forLineVectorInFile(fileName,' ',
			double ni56 = std::stod(FILE_LINE_VECTOR[FILE_LINE_VECTOR.size() -1]);
			
			if (ni56 < targetNickelYield)
			{
				lowNiLine = FILE_LINE_VECTOR;
				break;
			}
			highNiLine = FILE_LINE_VECTOR;
		);
		
		// with correct yield found, begin extracting data
		double highNi = std::stod(highNiLine[highNiLine.size() -1]);
		double lowNi = std::stod(lowNiLine[lowNiLine.size() -1]);
		double linInterp = (targetNickelYield - lowNi)/(highNi - lowNi);
		
		double highNiRemnant = std::stod(highNiLine[0]);
		double lowNiRemnant = std::stod(lowNiLine[0]);
		double remnantMass = lowNiRemnant + linInterp * (highNiRemnant - lowNiRemnant);
		double ejectaMass = m - remnantMass;

		
		RemnantRidge.Points[i] = YieldPoint(m,remnantMass / m);
		for (int j = 0; j < Ridges.size(); ++j)
		{
			
			int column = elementMasses[j];

			if (column > -1)
			{
				double highYield = std::stod(highNiLine[column]);
				double lowYield= std::stod(lowNiLine[column]);
				
				double yield = lowYield + linInterp * (highYield - lowYield);

				Ridges[j].Points[i] = YieldPoint(m,yield);
			}
			else
			{
				//this is for metallicity
				double z = m  - remnantMass - (Ridges[0].Points[i].Yield + Ridges[1].Points[i].Yield) ;
				//above is fraction of star which is metals - need to convert into fraction of ejecta which is metals 

				Ridges[j].Points[i] = YieldPoint(m,z);
			}
		}
		
		//loop back through now all linking has been completed
		for (int j = 0; j < Ridges.size(); ++j)
		{
			Ridges[j].Points[i].Yield = Ridges[j].Points[i].Yield / ejectaMass - orfeoSolarValues[elementsPresent[j]];
		}
	}
	
	RelicMass.Ridges.push_back(RemnantRidge);
	for (int i = 0; i < Ridges.size(); ++i)
	{
		int ridgeId = elementsPresent[i];
		if (ridgeId != Opts->Element.MagnesiumID)
		{
			std::cout << "ORFEO loading..." << Opts->Element.ElementNames[ridgeId] << std::endl;
			Element[ridgeId].Ridges.push_back(Ridges[i]);
		}
	}
}

void YieldGrid::LoadLimongiYields()
{
	//The "medium yield" data set is a set of yields in M-Z space drawn from a Limongi & Chieffi data set
	//The data was originally hand-written into the code: I have extracted this data set into a file, which this function reads in.
	//This therefore assumes the data file is ordered as follows:
	
	log(2) << "\t\tUnknown Limongi Yields\n";
	
	//hardcoded data
	std::vector<double> masses = {13.0,15.0,20.0,25.0,30.0,35.0};
	std::vector<double> metallicity = {0.0, 0.000001, 0.0001, 0.001, 0.006,0.02};
	std::vector<double> heliumContent = {0.23,0.23,0.23,0.23,0.26,0.28};
	
	std::string fileName = "Resources/ChemicalData/LimoChiefYields.dat";
	
	std::vector<int> elementsPresent = {Opts->Element.HydrogenID, Opts->Element.HeliumID, Opts->Element.MetalsID, Opts->Element.IronID, Opts->Element.OxygenID, Opts->Element.MagnesiumID,  Opts->Element.CarbonID, Opts->Element.SiliconID, Opts->Element.CalciumID, Opts->Element.ManganeseID, Opts->Element.ChromiumID, Opts->Element.CobaltID};
	

	std::vector<std::vector<YieldRidge>> Ridges;
	Ridges.resize(elementsPresent.size());
	
	for (int j = 0; j < Ridges.size() ; ++j)
	{
		Ridges[j].resize(metallicity.size());
		
		for (int i =0; i < metallicity.size(); ++i)
		{
			Ridges[j][i] = YieldRidge(Opts->Element.LimongiID,metallicity[i], masses.size() );
		}
	}
	
	
	std::vector<YieldRidge> RemnantRidges;
	RemnantRidges.resize(metallicity.size());
	int i = 0;
	for (YieldRidge ridge : RemnantRidges)
	{
		RemnantRidges[i] = YieldRidge(Opts->Element.LimongiID,metallicity[i], masses.size() );
		++i;
	}
	
	int lineCount  = 0;
	
	forLineVectorInFile(fileName, '\t',
		
		
	
		int mIndex = (int) lineCount / 6;
		int zIndex = lineCount - mIndex * 6;
		
		double mass = masses[mIndex];
		double z = metallicity[zIndex];
		double y = heliumContent[zIndex];
		double x = 1.0 - z - y;
		
		std::vector<double> solarElements = {x,y,z, Opts->Element.SolarIron,  Opts->Element.SolarOxygen,  Opts->Element.SolarMagnesium,  Opts->Element.SolarCarbon,  Opts->Element.SolarSilicon,  Opts->Element.SolarCalcium,  Opts->Element.SolarManganese,  Opts->Element.SolarChromium,  Opts->Element.SolarCobalt};
		
		
		
		double relicMass = std::stod(FILE_LINE_VECTOR[3]);
		double ejectaMass = mass - relicMass;
		

		RemnantRidges[zIndex].Points[mIndex] = YieldPoint(mass, relicMass / mass);
		for (int j = 0; j < elementsPresent.size(); ++j)
		{	
			double solarValue = solarElements[j];
			if (j > 2)
			{
				solarValue *= z / Opts->Element.SolarMetals;
			}
			double yield = std::stod(FILE_LINE_VECTOR[5+j])/ejectaMass - solarElements[j];

			Ridges[j][zIndex].Points[mIndex] = YieldPoint(mass, yield);
		}
		
		
		++lineCount;
	);
	
	for (int i = 0; i < elementsPresent.size() ; ++i)
	{
		if (elementsPresent[i] != Opts->Element.IronID)
		{
			for (YieldRidge ridge : Ridges[i])
			{
				Element[elementsPresent[i]].Ridges.push_back(ridge);
			}
		}
	}
	
	for (YieldRidge relics : RemnantRidges)
	{
		RelicMass.Ridges.push_back(relics);
	}
	
}

void YieldGrid::LoadMaederYields()
{
	log(2) << "\t\tMaeder Yields\n";
	
	//hardcoded data
	std::vector<double> masses = {3.0,5.0,9.0,12.0,15.0,20.0,25.0,40.0,60.0,85.0,120.0};
	std::vector<double> metallicity = {0.001,0.02};
	std::vector<double> heliumContent = {0.24,0.28};
	
	std::string fileName = "Resources/ChemicalData/MaederYields.dat";
	
	std::vector<int> elementsPresent = {Opts->Element.HeliumID,  Opts->Element.CarbonID,Opts->Element.OxygenID, Opts->Element.MetalsID};
	
	std::vector<std::vector<YieldRidge>> Ridges;
	Ridges.resize(elementsPresent.size());
	
	for (int j = 0; j < Ridges.size() ; ++j)
	{
		Ridges[j].resize(metallicity.size());
		
		for (int i =0; i < metallicity.size(); ++i)
		{
			Ridges[j][i] = YieldRidge(Opts->Element.MaederID,metallicity[i], masses.size() );
		}
	}
	
	
	std::vector<YieldRidge> RemnantRidges;
	RemnantRidges.resize(metallicity.size());
	int i = 0;
	for (YieldRidge ridge : RemnantRidges)
	{
		RemnantRidges[i] = YieldRidge(Opts->Element.MaederID,metallicity[i], masses.size() );
		++i;
	}
	
	int lineCount  = 0;
	
	forLineVectorInFile(fileName, '\t',
		
		
		int mIndex = (int) lineCount / metallicity.size();
		int zIndex = lineCount - mIndex * metallicity.size();
		
		double mass = masses[mIndex];
		double z = metallicity[zIndex];
		double y = heliumContent[zIndex];
		double x = 1.0 - z - y;
		
		double relicMass = std::stod(FILE_LINE_VECTOR[2]);
		double ejectaMass = mass - relicMass;
		
		RemnantRidges[zIndex].Points[mIndex] = YieldPoint(mass, relicMass / mass);
		for (int j = 0; j < elementsPresent.size(); ++j)
		{	
			double rawYield = std::stod(FILE_LINE_VECTOR[3+j]);
			double yield = rawYield/ ejectaMass;
			Ridges[j][zIndex].Points[mIndex] = YieldPoint(mass, yield);
		}
		
		++lineCount;
	);
	
	for (int i = 0; i < elementsPresent.size() ; ++i)
	{
		for (YieldRidge ridge : Ridges[i])
		{
			Element[elementsPresent[i]].Ridges.push_back(ridge);
		}
	}
	
	for (YieldRidge relics : RemnantRidges)
	{
		RelicMass.Ridges.push_back(relics);
	}
	
}

YieldRidge MergeRidges(std::vector<YieldRidge> candidates, Options * opts)
{
	//order goes from left to right - first element takes highest priority
	std::vector<double> PriorityOrder = {opts->Element.OrfeoID,opts->Element.MaederID, opts->Element.MarigoID, opts->Element.LimongiID};
	

	
	std::vector<YieldPoint> mergedPoints = {};

	double sumZ = 0;
	for (int i = 0; i < candidates.size(); ++i)
	{		
		sumZ += candidates[i].Z;
		auto basePriority = std::find(PriorityOrder.begin(), PriorityOrder.end(), candidates[i].SourceID);
		for (YieldPoint point : candidates[i].Points)
		{
			bool noConflict = true;
			
			
			
			for (int j = 0; j < candidates.size(); ++j)
			{
				if (i != j)
				{	
					YieldRidge ridge2 = candidates[j];
					auto [minM2, maxM2] = std::minmax_element(ridge2.Points.begin(), ridge2.Points.end(), [](const YieldPoint& a, const YieldPoint& b) {  return a.Mass < b.Mass;});
					YieldPoint low2 = *minM2;
					YieldPoint top2= *maxM2;
				
					bool liesInsideRegion = (  point.Mass <= top2.Mass) && (point.Mass >= low2.Mass);
					
					if (liesInsideRegion)
					{
						auto comparisonPriority = std::find(PriorityOrder.begin(), PriorityOrder.end(), candidates[j].SourceID);
						
						if (comparisonPriority < basePriority)
						{
							noConflict = false;
						}
					}
					
				}
			}
			
			if (noConflict)
			{
				mergedPoints.push_back(point);
			}
		
		}
	}
	
	sort(mergedPoints.begin(), mergedPoints.end(), [](const YieldPoint& a, const YieldPoint& b) {  return a.Mass < b.Mass;});
	
	double meanZ = sumZ / candidates.size();
	YieldRidge merger;
	merger.Z = meanZ;
	merger.Points = mergedPoints;
	merger.Merged = true;
	merger.SourceID = opts->Element.MixedID;
	return merger;
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
	if (Ridges.size() == 0)
	{
		log(3) << "\nWARNING: The yield grid associated with " + Opts->Element.ElementNames[Element] + " has not been assigned any data ridges. The yield grid will default to zero yield at all points\n";
		return;
	}
	
	if (Ridges.size() == 1)
	{
		log(3) << "\nWARNING: The yield grid associated with " +  Opts->Element.ElementNames[Element] + " has been assigned only a single data ridge. This means the results will be metallicity independent\n";
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



double YieldRidge::MassInterp(double mass,Options * opts)
{
	bool isOverweight = mass > Points[Points.size()-1].Mass;
	bool isUnderweight = mass <= Points[0].Mass;
	
	int topID = 0;
	if ( isOverweight || isUnderweight)
	{
		if (isOverweight)
		{
			topID = Points.size() - 1;
		}
		else
		{
			topID = 1;
		}
	}
	else
	{
		while ( Points[topID].Mass < mass)
		{
			++topID;
		}
	}

	double topMass = Points[topID].Mass;
	double lowMass = Points[topID-1].Mass;
	
	double interpFactor = (mass - lowMass)/(topMass - lowMass);
	double maxInterp = 1.0 + opts->Element.maxInterpolationFactor;
	double minInterp = - opts->Element.maxInterpolationFactor;
	if (interpFactor > maxInterp)
	{
		interpFactor = maxInterp;
	}
	if (interpFactor < minInterp)
	{
		interpFactor = minInterp;
	}
	
	double midYield =  Points[topID-1].Yield + (Points[topID].Yield - Points[topID - 1].Yield) * interpFactor;
	return midYield;
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
				
				//~ if (Element == Opts->Element.HeliumID)
				//~ {
					//~ std::cout << mass << ", " << Z 
				//~ }
				
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
	double smoothingLength = 1;
	
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
