#include "YieldGrid.h"

//Several of the more in-depth functions to do with the loading and interpolation of the data within YieldGrid and StellarYield objects are found within the YieldLoaders file.

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
	
	if (Opts->Element.PrintYieldRidges)
	{
		for (StellarYield element : Element)
		{
			element.PrintRidges();
		}
	}
	if (Opts->Element.PrintYieldGrid)
	{
		SaveYields();
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

void YieldGrid::SaveYields()
{
	log(1) << "Saving yield grid\n";
	

	
	std::ostringstream text;
	std::vector<std::string> titles = {"Mass", "Metallicity", "Relic Mass", "Relic Type"};
	titles.insert(titles.end(), Opts->Element.ElementNames.begin(), Opts->Element.ElementNames.end() );
	int width = 15;
	for (auto title : titles)
	{
		text << std::setw(width) << std::left << title;
	}
	text << "\n";
	int N = Element[0].GridSize;
	for(int mIndex = 0; mIndex < N; ++mIndex)
	{
		double M = Element[0].MFromIndex(mIndex);
		std::cout << M <<std::endl;
		for (int zIndex = 0; zIndex < N; ++zIndex)
		{
			double Z = Element[0].ZFromIndex(zIndex);
			text <<  std::setw(width) << std::left << M;	
			text << std::setw(width) << std::left  << Z;	
			text << std::setw(width) << std::left  << "??";
			text << std::setw(width) << std::left  << "??";
			
			
			
			for (StellarYield elem : Element)
			{
				text << std::setw(width) << std::left << elem.GrabYield(mIndex,zIndex);			
			}
			text << "\n";
		}
		
		
		
	}
	
	//saves this output to both the Resources folder for later use, and the output folder for processing
	std::vector<std::string> savedYieldsFiles = {Opts->Simulation.Resources + "ChemicalData/ProcessedYields.dat", Opts->Simulation.FileRoot + "ProcessedYields.dat"};
	
	for (auto fileName : savedYieldsFiles)
	{
		std::fstream file;
		file.open(fileName, std::fstream::out);
		
		file << text.str();
		file.close();
		
	}

}
