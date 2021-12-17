#include "SLF.h"

double SLF_Functor::PredictLifetime(double mass, double logz)
{
	//units = Gyr
	return 10 * pow(mass,-2.5);
}

SLF_Functor::SLF_Functor(const GlobalParameters & param) : Param(param)
{
	PrecomputedGrid = std::vector<std::vector<double>>(Param.Stellar.MassResolution, std::vector<double>(Param.Stellar.LogZResolution,NotComputed));
	if (Param.Meta.Verbosity > 0)
	{
		std::cout << "\tSLF Functor Initialised" << std::endl;
	}
}

int SLF_Functor::operator()(int mass, double metallicity)
{

	double logZ = std::max(log10(metallicity),Param.Stellar.MinLogZ.Value);
	int closestMetallicityID = round((logZ - Param.Stellar.MinLogZ)/Param.Stellar.LogZDelta);
	int upID;
	int downID;
	
	if (logZ >= Param.Stellar.LogZGrid[closestMetallicityID])
	{
		downID = closestMetallicityID;
		upID = std::min(closestMetallicityID+1, Param.Stellar.LogZResolution -1);
		
	}
	else
	{
		upID = closestMetallicityID;
		downID = std::max(0,closestMetallicityID - 1);
	}
	
	double lifeTime;
	if (upID == downID)
	{
		lifeTime = ValueInquiry(mass,upID);
	}
	else
	{
		double upLife = ValueInquiry(mass,upID);
		double downLife = ValueInquiry(mass,downID);
		double upZ = Param.Stellar.LogZGrid[upID];
		double downZ = Param.Stellar.LogZGrid[downID];
		
		lifeTime = downLife + (upLife -downLife)/(upZ - downZ) * (logZ - downZ);
	}
		
	//Turn the lifetime into timestep units
	return round(lifeTime / Param.Meta.TimeStep);
}
double SLF_Functor::ValueInquiry(int m, int z)
{
	double currentValue = PrecomputedGrid[m][z];
	if (currentValue == NotComputed)
	{
		currentValue = PredictLifetime(Param.Stellar.MassGrid[m],Param.Stellar.LogZGrid[z]);
		PrecomputedGrid[m][z] = currentValue;
	}
	return currentValue;
}
