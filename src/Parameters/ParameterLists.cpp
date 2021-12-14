#include "ParameterLists.h"

void MetaValues::Initialise(std::string resourceRoot)
{
	SimulationSteps = ceil(SimulationDuration / TimeStep);
}

void OutputValues::Initialise(std::string resourceRoot)
{
	JSL::mkdir(Root.Value);
	GalacticDirectory.Value = Root.Value + "/" + GalacticDirectory.Value;
	JSL::mkdir(GalacticDirectory.Value);
	
	GalaxyMassFile.Value = GalacticDirectory.Value + "/" + GalaxyMassFile.Value;
	JSL::initialiseFile(GalaxyMassFile.Value);
	
	RingDirectory.Value = Root.Value + "/" + RingDirectory.Value;
	JSL::mkdir(RingDirectory.Value);
}


void ElementValues::GiveElementsNames()
{
	//pretty sure there must be a more intelligent way to do this....but I can guarantee that this preserves the ordering of the enum!
	ElementNames.resize(ElementCount);
	ElementNames[Hydrogen] = "H";
	ElementNames[Helium] = "He";
	ElementNames[Metals] = "Z";
	ElementNames[Iron] = "Fe";
	ElementNames[Oxygen] = "O";
	ElementNames[Magnesium] = "Mg";
	ElementNames[Carbon] = "C";
	ElementNames[Silicon] = "Si";
	ElementNames[Calcium] = "Ca";
	ElementNames[Manganese] = "Mn";
	ElementNames[Chromium]= "Cr";
	ElementNames[Cobalt] = "Co";
	ElementNames[Europium] = "Eu";
}

void ElementValues::Initialise(std::string resourceRoot)
{
	GiveElementsNames();
	
	//read in the solar abundance data from file
	std::string solarFile = resourceRoot + (std::string)SolarAbundanceFile;
	SolarAbundances = std::vector<double>(ElementCount,1e-20);
	int i = 0;

	forLineVectorIn(solarFile,',',
		if (i > 0)
		{
			int c = 0;
			while (c < ElementCount) 
			{
				if (FILE_LINE_VECTOR[SolarAbundanceFileNameColumn] == ElementNames[c])
				{
					SolarAbundances[c] = std::stod(FILE_LINE_VECTOR[SolarAbundanceFileDataColumn]);
					c = ElementCount;
				}
				++c;
			}
		}
		++i;
	);
}

void StellarValues::Initialise(std::string resourceRoot)
{
	//Allows for arbitrary mass steps without altering the rest of the code (hopefully!)
	MassGrid = std::vector<double>(MassResolution.Value);
	MassDeltas = std::vector<double>(MassResolution.Value);
	//~ double gridWidth = (MaxStellarMass - ImmortalMass)/MassResolution
	
	double alpha = 1.05;
	double sumFactor;
	if (alpha == 1)
	{
		sumFactor = MassResolution.Value;
	}
	else
	{
		sumFactor = (pow(alpha,MassResolution.Value) - 1.0)/(alpha - 1);
	}
	double w = (MaxStellarMass - ImmortalMass)/sumFactor;
	double x = ImmortalMass;
	
	for (int i = 0; i < MassResolution; ++i)
	{
		x += w/2;
		MassGrid[i] = x;
		MassDeltas[i] = w;
		x += w/2;
		w = w * alpha;
	}
	
	LogZGrid = std::vector<double>(LogZResolution);
	double zwidth = (MaxLogZ - MinLogZ)/(LogZResolution-1);
	for (int i =0; i < LogZResolution; ++i)
	{
		LogZGrid[i] = MinLogZ + (i)*zwidth;
	}
	LogZDelta = zwidth;
}
