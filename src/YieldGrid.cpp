#include "YieldGrid.h"

//Several of the more in-depth functions to do with the loading and interpolation of the data within YieldGrid and StellarYield objects are found within the YieldLoaders file.

StellarYield::StellarYield()
{
	//default initializer?
}

StellarYield::StellarYield(Options * opts,int ElementalID)
{
	Opts = opts;
	GridSize = 100;
	Element = ElementalID;
	Yield = std::vector<std::vector<double>>(GridSize, std::vector<double>(GridSize,0.0));	
}

void StellarYield::Print()
{
	
	std::string gridFile = Opts->Simulation.FileRoot + "/YieldTable_" + Opts->Element.ElementNames[Element] + ".dat";
	std::string ridgeFile = Opts->Simulation.FileRoot + "/Ridges_" + Opts->Element.ElementNames[Element] + ".dat";
	
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
	
	
	file.open(gridFile, std::fstream::out);
	for (int mIndex = 0; mIndex < GridSize; ++mIndex)
	{
		for (int zIndex = 0; zIndex < GridSize; ++zIndex)
		{
			file << MFromIndex(mIndex) << "\t" << ZFromIndex(zIndex) <<"\t" << Yield[mIndex][zIndex] << std::endl;
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
	double metDistance = log(Z/Opts->Stellar.MinZ) / log(Opts->Stellar.MaxZ / Opts->Stellar.MinZ);
	
	//offset of one is because i = 0 corresponds to Z = 0, and i = 1 corresponds to Z_min, since logarithms cannot go to zero!
	return 1 + round((GridSize - 1)* metDistance);
}

double StellarYield::GrabYield(double M, double Z)
{
	int MIndex = IndexFromM(M);
	int ZIndex = IndexFromZ(Z);
	
	return Yield[MIndex][ZIndex];
}

void StellarYield::PrepareGrids()
{
	log(3) << "\t\tInterpolating " + Opts->Element.ElementNames[Element] + " Grid\n";
	FilterRidges();
	
	InterpolateGrid();
	
	SmoothGrid();
	
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
		
		log(1) << "\tBeginning Grid Interpolation\n";
		for (int i = 0; i < Element.size(); ++i)
		{
			Element[i].PrepareGrids();
		}
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
	Merged = false;

	
	Points = std::vector<YieldPoint>(nPoints, YieldPoint(0.0,0.0));
}
