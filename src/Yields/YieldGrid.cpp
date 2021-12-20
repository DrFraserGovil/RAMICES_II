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

void YieldGrid::operator()(GasReservoir & scatteringReservoir, RemnantPopulation & remnantReservoir, int Nstars, int mass, double z, int birthIndex, GasReservoir & birthReservoir) const
{
	if (!IsBasic)
	{
			StellarInject(scatteringReservoir, remnantReservoir, Nstars, mass, z, birthIndex, birthReservoir);
	}
	else
	{
		throw std::runtime_error("You are calling for a simple yield grid on a stellar population, but this functionality does not exist yet!");
	}
}
void YieldGrid::operator()(GasReservoir & scatteringReservoir, int nObjects, int birthIndex) const
{
	if (!IsBasic)
	{
			throw std::runtime_error("You are calling for a simple yield grid on a remnant population, but this functionality does not exist yet!");
	}
	else
	{
		RemnantInject(scatteringReservoir,nObjects,birthIndex);
	}
}

void YieldGrid::StellarInject( GasReservoir & scatteringReservoir, RemnantPopulation & remnantReservoir, int Nstars, int mass, double z, int birthIndex, GasReservoir & birthReservoir) const
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
	
	double initMass = Param.Stellar.MassGrid[mass];
	double remnantMass = 0.7*initMass;
	
	double ejectaMass = Nstars * (initMass - remnantMass); //need to change!
	
	
	const std::vector<GasStream> & birthStreams = birthReservoir.GetHistory(birthIndex);
	for (int p = 0; p < ProcessCount; ++p)
	{
		SourceProcess proc = (SourceProcess)p;
		double initBirthMass = birthStreams[proc].ColdMass()+ 1e-88; //basic offset to prevent zero division
		GasStream chunk(proc);
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
			double massOfElem = ejectaMass * outputFraction / 1e9;
			
			
			chunk.Cold(elem) = massOfElem * hotInjectionFraction;
			chunk.Hot(elem) = massOfElem * (1.0 - hotInjectionFraction);
			//~ GrossOutputStream[birthIndex][proc][elem] += ejectaMass * outputFraction;
		}
		
		scatteringReservoir.AbsorbMemory(birthIndex, chunk);
	}
	
	//deal with remnants
	
	remnantReservoir.Feed(birthIndex,0,Nstars * remnantMass,0.0);
	
}

void YieldGrid::RemnantInject(GasReservoir & scatteringReservoir, int nObjects, int birthIndex) const
{
	GasStream TempStream(Process);
	for (int e = 0; e < ElementCount; ++e)
	{
		ElementID elem = (ElementID)e;
		double amountInjected = nObjects * Grid[0][0][elem];
		TempStream.Cold(elem) = amountInjected * (1.0 - hotInjectionFraction);
		TempStream.Hot(elem) = amountInjected * hotInjectionFraction;
	}
	
	scatteringReservoir.AbsorbMemory(birthIndex,TempStream);
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

	InitialiseLargeGrid(ccsnGridSize, Param.Stellar.LogZResolution);
	IsBasic = false;
	hotInjectionFraction = Param.Thermal.HotInjection_CCSN;
	
	
	/// spoof in the yields temporaily
	double xVal = -0.01;
	double yVal = 0.05;
	double zVal = 0.05;
	double feVal = zVal * 0.3;
	double mgVal = zVal  - feVal;
	for (int m = 0; m < MassOffset; ++m)
	{
		for (int z = 0; z < Param.Stellar.LogZResolution; ++z)
		{
			Grid[m][z][Hydrogen] = xVal;
			Grid[m][z][Helium] = yVal;
			Grid[m][z][Metals] = zVal;
			Grid[m][z][Iron] = feVal;
			Grid[m][z][Magnesium] = mgVal;
		}
	}
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
	InitialiseLargeGrid(ccsnGridSize, Param.Stellar.LogZResolution);
	IsBasic = false;
	hotInjectionFraction = Param.Thermal.HotInjection_AGB;
}
void YieldGrid::SNIa_Initialise()
{
	IsBasic = true;
	hotInjectionFraction = Param.Thermal.HotInjection_SNIa;
	Grid.resize(1);
	Grid[0].resize(1);
	Grid[0][0] = std::vector<double>(ElementCount,0.0);
	
	Grid[0][0][Iron] = 1;
}
void YieldGrid::NSM_Initialise()
{
	IsBasic = true;
	hotInjectionFraction = Param.Thermal.HotInjection_NSM;
}
