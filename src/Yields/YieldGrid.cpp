#include "YieldGrid.h"

YieldGrid::YieldGrid(const GlobalParameters & param, SourceProcess process): Param(param), Process(process)
{
	if (param.Meta.Verbosity > 0)
	{
		std::cout << "\t" << Param.Yield.ProcessNames[Process] << " yield grid initialising...." << std::flush;
	}
	MassOffset = 0;
	
	switch(Process)
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
		case NSM:
		{
			NSM_Initialise();
			break;
		}
		case SNIa:
		{
			SNIa_Initialise();
			break;
		}
		default:
		{
			throw std::runtime_error("You have tried to initialise a yield grid for which there is no rule to create - ID = " +std::to_string(Process) + "...I am forced to quit");
		}
	}
	
	
	if (param.Meta.Verbosity > 0)
	{
		std::cout << "complete" << std::endl;
	}
}

void YieldGrid::NetInject( std::vector<std::vector<Gas>> & GrossOutputStream, int Nstars, int mass, double z, int birthIndex, GasReservoir & birthReservoir)
{
	if (mass - MassOffset < 0)
	{
		throw std::runtime_error("You have called a yield injection on a star which is outside the scope of this yield grid - likely you have asked for the CCSN from a low mass star");
	}
	double logZ = std::max(log10(z),Param.Stellar.MinLogZ.Value);
	int closestMetallicityID = round((logZ - Param.Stellar.MinLogZ)/Param.Stellar.LogZDelta);
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
	
	double ejectaMass = Param.Stellar.MassGrid[mass]; //need to change!
	
	
	const std::vector<GasStream> & birthStreams = birthReservoir.GetHistory(birthIndex);
	for (int p = 0; p < ProcessCount; ++p)
	{
		SourceProcess proc = (SourceProcess)p;
		double initBirthMass = birthStreams[proc].ColdMass();
		for (int e = 0; e < ElementCount; ++e)
		{
			ElementID elem = (ElementID)e;
			double birthFraction = birthStreams[proc].Cold(elem) / initBirthMass;
			double synthesisFraction = 0;
			if (p == Process)
			{
				double upSynth = Grid[mass - MassOffset][upID][elem];
				double downSynth = Grid[mass - MassOffset][downID][elem];
				synthesisFraction = downSynth + (upSynth - downSynth) * interpolationFactor;
			}
			double outputFraction = std::max(0.0,birthFraction + synthesisFraction);
			GrossOutputStream[birthIndex][proc][elem] += ejectaMass * outputFraction;
		}
	}
	
	
}

void YieldGrid::InitialiseLargeGrid(int mSize, int zSize)
{
	int extraElements = 0;
	int yieldElements = ElementCount + extraElements;
	Grid = std::vector<std::vector<std::vector<double>>>(mSize, std::vector<std::vector<double>>(zSize, std::vector<double>(yieldElements,0.0)));
}
void YieldGrid::CCSN_Initialise()
{
	double ccsnCut = Param.Yield.CCSN_MassCut;
	int mID = 0;
	while (mID < Param.Stellar.MassResolution && Param.Stellar.MassGrid[mID] < ccsnCut)
	{
		++mID;
	}
	MassOffset = mID;
	int ccsnGridSize = Param.Stellar.MassResolution - MassOffset;

	std::cout << "CCSN grid works between " << Param.Stellar.MassGrid[MassOffset] << "->" << Param.Stellar.MassGrid[Param.Stellar.MassResolution -1] << "Inclusive" <<std::endl;
	InitialiseLargeGrid(ccsnGridSize, Param.Stellar.LogZResolution);
	IsNet = true;
	hotInjectionFraction = Param.Thermal.HotInjection_CCSN;
}
void YieldGrid::AGB_Initialise()
{
	double ccsnCut = Param.Yield.CCSN_MassCut;
	int mID = 0;
	while (mID < Param.Stellar.MassResolution -1 && Param.Stellar.MassGrid[mID+1] < ccsnCut)
	{
		++mID;
	}
	MassOffset = mID;
	int ccsnGridSize = MassOffset;
	std::cout << "AGB grid works between " << Param.Stellar.MassGrid[0] << "->" << Param.Stellar.MassGrid[MassOffset] << "Inclusive" <<std::endl;
	InitialiseLargeGrid(ccsnGridSize, Param.Stellar.LogZResolution);
	IsNet = true;
	hotInjectionFraction = Param.Thermal.HotInjection_AGB;
}
void YieldGrid::SNIa_Initialise()
{
	IsNet = false;
	hotInjectionFraction = Param.Thermal.HotInjection_SNIa;
}
void YieldGrid::NSM_Initialise()
{
	IsNet = false;
	hotInjectionFraction = Param.Thermal.HotInjection_NSM;
}
