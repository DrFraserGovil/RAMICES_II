#include "YieldGrid.h"


void YieldGrid::LoadOrfeoYields()
{
	//~ SourceID source = Orfeo;
	if (Param.Meta.Verbosity > 0)
	{
		std::cout << "\t\tLoading data from Limongi & Chieffi 2004" << std::endl;
	}
	
	std::vector<ElementID> elementsInSet = {Hydrogen, Helium, Metals, Iron, Carbon,Oxygen,Magnesium, Silicon, Calcium, Manganese, Chromium, Cobalt};
	
	int nElements = elementsInSet.size();
	std::string directory = Param.Yield.YieldRoot.Value + "OriginalOrfeo/";
	std::string metaData = directory + "orfeoPrescription.dat";
	
	std::vector<double> masses;
	std::vector<double> orfeoSolarAbundances(ElementCount);
	int i = 0;
	forLineVectorIn(metaData,',',
	
		if (i == 0)
		{
			for (int j = 3; j < FILE_LINE_VECTOR.size(); ++j)
			{
				masses.push_back(std::stod(FILE_LINE_VECTOR[j]));
			}
		}
		else
		{
			for (int k = 0; k < nElements; ++k)
			{
				bool nameMatches = (FILE_LINE_VECTOR[0] == Param.Element.ElementNames[elementsInSet[k]]);
				if (nameMatches)
				{
					orfeoSolarAbundances[elementsInSet[k]] = std::stod(FILE_LINE_VECTOR[1]);
				} 
			}
		}
		++i;
	
	);
	double z = orfeoSolarAbundances[elementsInSet[Metals]];
	std::vector<YieldRidge> ElementRidges = std::vector<YieldRidge>(ElementCount, YieldRidge(Orfeo,z,masses.size()));
	YieldRidge RemnantRidge = YieldRidge(Orfeo,z,masses.size());
	
	for (int i = 0; i < masses.size(); ++i)
	{
		double m = masses[i];
		
		//generate the correct filename for this mass
		std::string fileName = directory + "z22m";
		if (m < 100)
		{
			fileName += "0";
		} 
		fileName += std::to_string( (int)m) + "elecum.ngms";
		
		//each file has multiple simulations within it representing different mass cuts - must search through to find the one which gives the correct Ni56 yield
		//will grab the two data lines either side of the target yield, and then lineraly interpolate between them to get as close as possible
		double targetNickelYield = Param.Yield.TargetNi56Yield;
		std::vector<std::string> highNiLine;
		std::vector<std::string> lowNiLine;
		double lowNi;
		double highNi;
		forLineVectorIn(fileName,' ',
			double ni56 = std::stod(FILE_LINE_VECTOR[FILE_LINE_VECTOR.size() -1]);
			
			if (ni56 < targetNickelYield)
			{
				lowNiLine = FILE_LINE_VECTOR;
				lowNi = ni56;
				break;
			}
			highNiLine = FILE_LINE_VECTOR;
			highNi = ni56;
		);
		if (lowNiLine.size() == 0)
		{
			throw std::runtime_error("ORFEO Data readin could not find a line with the correct Ni56 yield, so cannot perform the required interpolation");
			exit(4);
		}
		
		double interpFactor = (targetNickelYield - lowNi)/(highNi - lowNi);
		double lowRemnant = std::stod(lowNiLine[0]);
		double highRemnant = std::stod(highNiLine[0]);
		double remnantMass = lowRemnant + interpFactor * (highRemnant - lowRemnant);
		double ejectaMass = m - remnantMass;
		
		RemnantRidge.Points[i].Mass = m;
		RemnantRidge.Points[i].Yield = remnantMass / m;
		
		for (int j = 0; j < nElements; ++j)
		{
			ElementID elem = elementsInSet[j];
			int column = Param.Element.ProtonCounts[elem];
			if (column > 0)
			{
				double highYield = std::stod(highNiLine[column]);
				double lowYield= std::stod(lowNiLine[column]);
				
				double yield = lowYield + interpFactor * (highYield - lowYield);
				
				double amount = yield/ejectaMass - orfeoSolarAbundances[elem];
				ElementRidges[elem].Points[i] = YieldPoint(m,amount);
			}
		}
		//net X+Y+Z must sum to 0, so force in here:
		double zEject = 0.0 - ElementRidges[Hydrogen].Points[i].Yield -ElementRidges[Helium].Points[i].Yield;
		ElementRidges[Metals].Points[i] = YieldPoint(m,zEject);
	}
	
	
	for (int i =0; i < nElements; ++i)
	{
		ElementID elem = elementsInSet[i];
		RidgeStorage[elem].push_back(ElementRidges[elem]);
	}
	RidgeStorage[RemnantLocation].push_back(RemnantRidge);
}

void YieldGrid::LoadMarigoYields()
{
	 //Note that this file is already given as yields, so no subtraction is needed.

	if (Param.Meta.Verbosity > 0)
	{
		std::cout << "\t\tLoading data from Marigo (2001)" << std::endl;
	}
	
	//hardcoded properties of the Marigo dataset
	std::vector<int> nMassPointsInFile = {22,23,23};
	
	std::vector<double> metallicityOfFile = {0.004, 0.008, 0.019};
	std::vector<double> heliumOfFile = {0.24,0.25,0.273};
	
	int nElementsPresent = 6;
	
	std::vector<ElementID> ids = {Hydrogen,Helium,Metals,Carbon,Oxygen};
	
	double mixingParameter = 1.68;
	
	for (int file = 0; file < 3; ++file)
	{		
		double fileZ = metallicityOfFile[file];
		
		std::vector<YieldRidge> Ridges(nElementsPresent, YieldRidge(Marigo,fileZ, nMassPointsInFile[file]));
		
		
		std::string fileName = Param.Yield.YieldRoot.Value + "/MarigoYields/totyieldsz0";
		if (fileZ < 0.01)
		{
			fileName += "0";
		}
		fileName += std::to_string( (int)(1000*fileZ)) + ".dat";
		
		int lineCount = 0;
		forLineVectorIn(fileName,' ',
			
			//file contains multiple bits of data, choose the mixing param with the most masses present
			if (std::stod(FILE_LINE_VECTOR[0]) != mixingParameter)
			{
				break;
			} 
			
			//collect the isotopic masses from the correct file columns
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
		
		
		for (int i = 0; i < ids.size(); ++i)
		{
			ElementID elem = ids[i];
			RidgeStorage[elem].push_back(Ridges[i]);
		}
		
		RidgeStorage[RemnantLocation].push_back(Ridges[Ridges.size() - 1]);
	}
	
}
void YieldGrid::LoadLimongiYields()
{
	//The "medium yield" data set is a set of yields in M-Z space drawn from a Limongi & Chieffi data set
	//The data was originally hand-written into the code: I have extracted this data set into a file, which this function reads in.
	//This therefore assumes the data file is ordered as follows:
	
	if (Param.Meta.Verbosity > 0)
	{
		std::cout << "\t\tLoading data from Limongi & Chieffi (2008)" << std::endl;
	}
	
	//hardcoded data
	std::vector<double> masses = {13.0,15.0,20.0,25.0,30.0,35.0};
	std::vector<double> metallicity = {1e-8, 0.000001, 0.0001, 0.001, 0.006,0.02};
	std::vector<double> heliumContent = {0.23,0.23,0.23,0.23,0.26,0.28};
	
	std::string fileName = Param.Yield.YieldRoot.Value +"LimoChiefYields.dat";

	std::vector<ElementID> elementsPresent = {Hydrogen,Helium,Metals,Iron,Oxygen,Magnesium,Carbon,Silicon,Calcium,Manganese,Chromium,Cobalt};
	

	std::vector<std::vector<YieldRidge>> Ridges;
	Ridges.resize(elementsPresent.size());
	for (int j = 0; j < Ridges.size() ; ++j)
	{
		//~ Ridges[j].resize(metallicity.size());
		
		for (int i =0; i < metallicity.size(); ++i)
		{
			Ridges[j].push_back(YieldRidge(Limongi,metallicity[i], masses.size() ));
		}
	}
	
	
	std::vector<YieldRidge> RemnantRidges;
	//~ RemnantRidges.resize(metallicity.size());
	
	for (int i = 0; i < metallicity.size(); ++i)
	{
		RemnantRidges.push_back(YieldRidge(Limongi,metallicity[i], masses.size() ));
	}
	int lineCount  = 0;
	
	forLineVectorIn(fileName, '\t',
		
	
		int mIndex = (int) lineCount / 6;
		int zIndex = lineCount - mIndex * 6;
		
		double mass = masses[mIndex];
		double z = metallicity[zIndex];
		double y = heliumContent[zIndex];
		double x = 1.0 - z - y;
		std::vector<double> solarElements = Param.Element.SolarAbundances;

		solarElements[Hydrogen] = x;
		solarElements[Helium] = y;
		solarElements[Metals] = z;
		
		double relicMass = std::stod(FILE_LINE_VECTOR[3]);
		double ejectaMass = mass - relicMass;

		RemnantRidges[zIndex].Points[mIndex] = YieldPoint(mass, relicMass / mass);
		for (int j = 0; j < elementsPresent.size(); ++j)
		{	
			double solarValue = solarElements[elementsPresent[j]];
			if (j > 2)
			{
				solarValue *= z / Param.Element.SolarAbundances[Metals];
			}
			double yield = std::stod(FILE_LINE_VECTOR[5+j])/ejectaMass-solarValue ;//?? perhaps not....was generated from Ralph's hardcoding, so maybe not

			Ridges[j][zIndex].Points[mIndex] = YieldPoint(mass, yield);
		}
		
		
		++lineCount;
	);
	
	for (int i = 0; i < elementsPresent.size() ; ++i)
	{
		ElementID elem = elementsPresent[i];
		//~ if (elem != Iron)
		//~ {
			for (YieldRidge ridge : Ridges[i])
			{
				RidgeStorage[elem].push_back(ridge);
			}
		//~ }
	}
	
	for (YieldRidge relics : RemnantRidges)
	{
		RidgeStorage[RemnantLocation].push_back(relics);
	}
	
}

void YieldGrid::LoadMaederYields()
{
	//Loads in the dataset from Maeder (1992), reproduced in Pagel
        //Despite covering a large mass range (3-120 M_solar), Maeder only traces H,He and CNO
        //Hence this dataset cannot be used when interpolating the more massive metal grids
        //Note that these are once again net yields, so no subtraction is necessary

		if (Param.Meta.Verbosity > 0)
	{
		std::cout << "\t\tLoading data from Maeder (1992)" << std::endl;
	}
	
	
	//hardcoded data
	std::vector<double> masses = {3.0,5.0,9.0,12.0,15.0,20.0,25.0,40.0,60.0,85.0,120.0};
	std::vector<double> metallicity = {0.001,0.02};
	std::vector<double> heliumContent = {0.24,0.28};
	
	std::string fileName =  Param.Yield.YieldRoot.Value + "MaederYields.dat";
	
	std::vector<ElementID> elementsPresent = {Helium, Carbon, Oxygen,Metals,Hydrogen};
	
	std::vector<std::vector<YieldRidge>> Ridges;
	Ridges.resize(elementsPresent.size());
	
	for (int j = 0; j < Ridges.size() ; ++j)
	{
		//~ Ridges[j].resize(metallicity.size());
		
		for (int i =0; i < metallicity.size(); ++i)
		{
			Ridges[j].push_back(YieldRidge(Maeder,metallicity[i], masses.size() ));
		}
	}
	
	
	std::vector<YieldRidge> RemnantRidges;
	for (int i = 0; i < metallicity.size(); ++i)
	{
		RemnantRidges.push_back(YieldRidge(Maeder,metallicity[i], masses.size() ));
	}
	
	int lineCount  = 0;
	
	forLineVectorIn(fileName, '\t',
		
		int mIndex = (int) lineCount / metallicity.size();
		int zIndex = lineCount - mIndex * metallicity.size();
		
		double mass = masses[mIndex];
		double z = metallicity[zIndex];
		double y = heliumContent[zIndex];
		double x = 1.0 - z - y;
		
		double relicMass = std::stod(FILE_LINE_VECTOR[2]);
		double ejectaMass = mass - relicMass;
		
		RemnantRidges[zIndex].Points[mIndex] = YieldPoint(mass, relicMass / mass);
		for (int j = 0; j < elementsPresent.size()-1; ++j)
		{	
			double rawYield = std::stod(FILE_LINE_VECTOR[3+j]);
			double yield = rawYield/ ejectaMass;

	
			Ridges[j][zIndex].Points[mIndex] = YieldPoint(mass, yield);
		}
		
		double hydrogenYield = -Ridges[0][zIndex].Points[mIndex].Yield-Ridges[3][zIndex].Points[mIndex].Yield;
		Ridges[elementsPresent.size()-1][zIndex].Points[mIndex] = YieldPoint(mass,hydrogenYield);
		++lineCount;
	);
	
	for (int i = 0; i < elementsPresent.size() ; ++i)
	{
		for (YieldRidge ridge : Ridges[i])
		{
			RidgeStorage[elementsPresent[i]].push_back(ridge);
		}
	}
	
	for (YieldRidge relics : RemnantRidges)
	{
		RidgeStorage[RemnantLocation].push_back(relics);
		int n = RidgeStorage[RemnantLocation].size();
	}
	
}



