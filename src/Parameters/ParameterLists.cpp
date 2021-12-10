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
