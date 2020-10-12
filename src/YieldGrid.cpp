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

YieldRidge::YieldRidge()
{
	Z = 0;
}
YieldRidge::YieldRidge(int ID, double z, int nPoints)
{
	SourceID = ID;
	Z = z;
	Masses = std::vector<double>(nPoints,0.0);
	Yields = std::vector<double>(nPoints,0.0);
}
