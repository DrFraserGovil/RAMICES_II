#include "YieldGrid.h"

StellarYield::StellarYield()
{
	//default initializer?
}
StellarYield::StellarYield(Options * opts,int ElementalID)
{
	Opts = opts;
	GridSize = 200;
	Element = ElementalID;
	Yield = std::vector<std::vector<double>>(GridSize, std::vector<double>(GridSize,0.0));
	
	
}


void StellarYield::Print()
{
	
	std::string outFile = Opts->Simulation.FileRoot + "/YieldTable_" + Opts->Element.ElementNames[Element] + ".dat";
	
	std::fstream file;
	file.open(outFile,std::fstream::out ); 
	for (YieldRidge ridge : Ridges)
	{
		for (int i = 0; i < ridge.Masses.size(); ++i)
		{
			file << ridge.Masses[i] << "\t" << ridge.Z << "\t" << ridge.Yields[i] << "\n";
		}
	}
	file.close();
	
}

//you MUST make sure that the forward AND backward extrapolations of indices match!

double StellarYield::MFromIndex(int index)
{
	double factor = (double)index/(GridSize - 1);
	return factor*factor *(Opts->Stellar.MaxMass - Opts->Stellar.MinMass) + Opts->Stellar.MaxMass;
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
	double powerfactor = (double)(index-1)/(GridSize - 1);
	double basefactor = Opts->Stellar.MaxZ / Opts->Stellar.MinZ;
	
	return std::pow(basefactor,powerfactor);
}



int StellarYield::IndexFromZ(double Z)
{
	if (Z < Opts->Stellar.MinZ)
	{
		return 0;
	}
	double metDistance = log(Z/Opts->Stellar.MinZ) / log(Opts->Stellar.MaxZ / Opts->Stellar.MinZ);
	
	//offset of one is because i = 0 corresponds to Z = 0, and i = 1 corresponds to Z_min, since logarithms cannot go to zero!
	return 1 + round((GridSize - 1)* metDistance);
}

double::StellarYield::GrabYield(double M, double Z)
{
	int MIndex = IndexFromM(M);
	int ZIndex = IndexFromZ(Z);
	
	return Yield[MIndex][ZIndex];
	
}

YieldGrid::YieldGrid()
{
	//default initializer?
}


YieldGrid::YieldGrid(Options * opts)
{
	Opts = opts;
	
	Element.resize(0);
	for (int i = 0; i < Opts->Element.NSpecies; ++i)
	{
		Element.push_back( StellarYield(opts, i));
	}
	RelicMass = StellarYield(opts,-1);
	RelicType = StellarYield(opts,-2);
	
	if (opts->Simulation.UseOldYieldGrid == true)
	{
		ReUseYields();
	}
	else
	{
		CalculateYields();
	}
	
	for (StellarYield element : Element)
	{
		element.Print();
	}
}


double YieldGrid::Synthesis(int ElementCode, double M, double Z)
{
	return Element[ElementCode].GrabYield(M,Z);
}

void YieldGrid::ReUseYields()
{
	
}
void YieldGrid::CalculateYields()
{
	//comment out/add sections here to use additional data in the yield calculations
	if (Opts->Simulation.UseSpinningYields == true)
	{
		//do something here
	}
	else
	{
		log(1) << "Re-calculating the yield grid. \n \tBeginning Yield Read-ins\n";
		LoadMarigoYields();			
		LoadOrfeoYields();
		LoadLimongiYields();
		LoadMaederYields();
	}
}

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
		
		std::vector<YieldRidge> Ridges(nElementsPresent, YieldRidge(fileZ, nMassPointsInFile[file]));
		
		
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
				Ridges[i].Masses[lineCount] = mass; 
				Ridges[i].Yields[lineCount] = yields[i] / ejectaMass;
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

	std::vector<int> elementsPresent = {Opts->Element.HydrogenID, Opts->Element.HeliumID, Opts->Element.MetalsID, Opts->Element.CarbonID, Opts->Element.OxygenID, Opts->Element.MagnesiumID, Opts->Element.SiliconID, Opts->Element.CalciumID, Opts->Element.ManganeseID, Opts->Element.ChromiumID, Opts->Element.CobaltID};
	std::vector<int> elementMasses = {1,2,-1,26,8,12,6,14,20,25,24,26};
	
	std::vector<YieldRidge> Ridges = std::vector<YieldRidge>(elementsPresent.size(), YieldRidge(z, masses.size()));
	YieldRidge	RemnantRidge = YieldRidge(z,masses.size());
	
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
		RemnantRidge.Masses[i] = m;
		RemnantRidge.Yields[i] = remnantMass / m;
		
		for (int j = 0; j < Ridges.size(); ++j)
		{
			
			int column = elementMasses[j];
			Ridges[j].Masses[i] = m;
			if (column > -1)
			{
				double highYield = std::stod(highNiLine[column]);
				double lowYield= std::stod(lowNiLine[column]);
				
				double yield = lowYield + linInterp * (highYield - lowYield);
				
				Ridges[j].Yields[i] = yield;
			}
			else
			{
				//this is for metallicity
				double z = m  - remnantMass - (Ridges[0].Yields[i] + Ridges[1].Yields[i]) ;
				//above is fraction of star which is metals - need to convert into fraction of ejecta which is metals 
				
				Ridges[j].Yields[i] = z;
			}
		}
		
		//loop back through now all linking has been completed
		for (int j = 0; j < Ridges.size(); ++j)
		{
			Ridges[j].Yields[i] = Ridges[j].Yields[i] / ejectaMass - orfeoSolarValues[elementsPresent[j]];
		}
	}
	
	RelicMass.Ridges.push_back(RemnantRidge);
	for (int i = 0; i < Ridges.size(); ++i)
	{
		int ridgeId = elementsPresent[i];
		Element[ridgeId].Ridges.push_back(Ridges[i]);
		
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
			Ridges[j][i] = YieldRidge(metallicity[i], masses.size() );
		}
	}
	
	
	std::vector<YieldRidge> RemnantRidges;
	RemnantRidges.resize(metallicity.size());
	int i = 0;
	for (YieldRidge ridge : RemnantRidges)
	{
		RemnantRidges[i] = YieldRidge(metallicity[i], masses.size() );
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
		
		
		RemnantRidges[zIndex].Masses[mIndex] = mass;
		RemnantRidges[zIndex].Yields[mIndex] = relicMass / mass;
		for (int j = 0; j < elementsPresent.size(); ++j)
		{	
			double solarValue = solarElements[j];
			if (j > 2)
			{
				solarValue *= z / Opts->Element.SolarMetals;
			}
			double yield = std::stod(FILE_LINE_VECTOR[5+j])/ejectaMass - solarElements[j];
			Ridges[j][zIndex].Masses[mIndex] = mass;
			Ridges[j][zIndex].Yields[mIndex] = yield;
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
			Ridges[j][i] = YieldRidge(metallicity[i], masses.size() );
		}
	}
	
	
	std::vector<YieldRidge> RemnantRidges;
	RemnantRidges.resize(metallicity.size());
	int i = 0;
	for (YieldRidge ridge : RemnantRidges)
	{
		RemnantRidges[i] = YieldRidge(metallicity[i], masses.size() );
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
		
		
		
		
		double ejectaMass = std::stod(FILE_LINE_VECTOR[2]);
		double relicMass = mass - ejectaMass;
		
		
		RemnantRidges[zIndex].Masses[mIndex] = mass;
		RemnantRidges[zIndex].Yields[mIndex] = relicMass / mass;
		for (int j = 0; j < elementsPresent.size(); ++j)
		{	
			
			double yield = std::stod(FILE_LINE_VECTOR[3+j])/ejectaMass;
			Ridges[j][zIndex].Masses[mIndex] = mass;
			Ridges[j][zIndex].Yields[mIndex] = yield;
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

YieldRidge::YieldRidge()
{
	Z = 0;
}
YieldRidge::YieldRidge(double z, int nPoints)
{
	Z = z;
	Masses = std::vector<double>(nPoints,0.0);
	Yields = std::vector<double>(nPoints,0.0);
}
