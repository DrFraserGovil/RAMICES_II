#include "SLF.h"


SLF_Functor::SLF_Functor(const GlobalParameters & param) : Param(param)
{
	PrecomputedGrid = std::vector<std::vector<double>>(Param.Stellar.MassResolution, std::vector<double>(Param.Stellar.LogZResolution,NotComputed));
	
	PrecomputeGrid();
	if (Param.Meta.Verbosity > 0)
	{
		std::cout << "\tSLF Functor Initialised" << std::endl;
	}
}

int SLF_Functor::operator()(int mass, double metallicity)
{

	double logZ = std::max(log10(metallicity),Param.Stellar.MinLogZ.Value);
	if (logZ > Param.Stellar.MaxLogZ)
	{
		std::cout << "ERROR - the metallicity (z = " << logZ << ") has exceeded the value and overflowed the edge of the grid (" << Param.Stellar.MaxLogZ << "). Please recompute with a higher metallicity boundary" << std::endl;
		exit(10);
	}
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
	return currentValue;
}

void SLF_Functor::PrecomputeGrid()
{
	//extract list of data into a useful grid, with only assumption being a consistent grid on M and logZ
	std::vector<double> fileM;
	std::vector<double> fileLogZ;
	std::vector<double> fileTau;
	
	int i = 0;
	forLineVectorIn(Param.Resources.LifeTimeFile.Value,',',
	
		if (i > 0)
		{
			//~ std::cout << FILE_LINE_VECTOR[0] << "  " << FILE_LINE_VECTOR[1] << " " << FILE_LINE_VECTOR[2] << std::endl;
			double m = std::stod(FILE_LINE_VECTOR[0]);
			double logZ= std::stod(FILE_LINE_VECTOR[1]);
			double tau = pow(10,std::stod(FILE_LINE_VECTOR[2])) / 1e9;
			
			fileM.push_back(m);
			fileLogZ.push_back(logZ);
			fileTau.push_back(tau);
		}
		++i;
	);
	
	//exctract unique values of M and logZ from pointwise arrays....
	std::vector<double> uniqueM;
	std::vector<double> uniqueLogZ;
	for (int i = 0; i < fileM.size(); ++i)
	{
		double m = fileM[i];
		double logZ = fileLogZ[i];
		
		if (std::find(uniqueM.begin(),uniqueM.end(),m) == uniqueM.end())
		{
			uniqueM.push_back(m);
		}
		if (std::find(uniqueLogZ.begin(),uniqueLogZ.end(),logZ) == uniqueLogZ.end())
		{
			uniqueLogZ.push_back(logZ);
		}
	}
	
	//now unfold 1D array into a useful grid.....
	std::vector<std::vector<double>> fileGrid(uniqueM.size(), std::vector<double>(uniqueLogZ.size(),0.0));
	for (int i = 0; i < uniqueM.size(); ++i)
	{
		double m = uniqueM[i];
		for (int j = 0; j < uniqueLogZ.size(); ++j)
		{
			double logZ = uniqueLogZ[j];
			
			for (int k = 0; k < fileTau.size();++k)
			{
				if (fileM[k] == m && fileLogZ[k] == logZ)
				{
					fileGrid[i][j] = fileTau[k];
				}
			}
		}
	}
	
	//now int/extrapolate the provided grid onto our own internal mass/logZ grid
	//not necessarily trivial as our grids may be non uniform etc.
	
	for (int i = 0; i < Param.Stellar.MassResolution; ++i)
	{
		double gridM = Param.Stellar.MassGrid[i];
		
		//find closest mass points
		int fileM_id = 0;
		while (fileM_id < uniqueM.size() -1 && uniqueM[fileM_id]  <= gridM )
		{
			++fileM_id;
		}
		int m_Up = fileM_id;
		int m_Down = fileM_id - 1;
		if (m_Down < 0)
		{
			m_Down = 0;
			m_Up = 1;
		}
		double bruch_Overflow = 0;
		double m_interp = (gridM - uniqueM[m_Down])/(uniqueM[m_Up] - uniqueM[m_Down]);
		m_interp = std::max(std::min(m_interp,1.0 + bruch_Overflow),-bruch_Overflow);
		
		for (int j = 0; j < Param.Stellar.LogZResolution; ++j)
		{
			double gridZ = Param.Stellar.LogZGrid[j];
			
			int fileZ_id = 0;
			while (fileZ_id < uniqueLogZ.size() -1 && uniqueLogZ[fileZ_id]  <= gridZ )
			{
				++fileZ_id;
			}
			int z_Up = fileZ_id;
			int z_Down = fileZ_id - 1;
			if (z_Down < 0)
			{
				z_Down = 0;
				z_Up = 1;
			}
			double z_interp = (gridZ - uniqueLogZ[z_Down])/(uniqueLogZ[z_Up] - uniqueLogZ[z_Down]);
			z_interp = std::max(std::min(z_interp,1.0 + bruch_Overflow),-bruch_Overflow);
			
			
			double zUpBranch = fileGrid[m_Down][z_Up] + m_interp * (fileGrid[m_Up][z_Down] - fileGrid[m_Down][z_Up]);
			double zDownBranch = fileGrid[m_Down][z_Down] + m_interp * (fileGrid[m_Up][z_Down] - fileGrid[m_Down][z_Down]);
			
			double interpolatedValue = zDownBranch + z_interp * (zUpBranch - zDownBranch);
			interpolatedValue = std::min(std::max(1e-3,interpolatedValue),11.0);
			PrecomputedGrid[i][j] = interpolatedValue;
			
			//~ std::cout << "For grid coord ( " << gridM << ", " << gridZ << "), I recommend \n\tM = " << uniqueM[m_Down] << " -> " << uniqueM[m_Up] <<  " (fac = " << m_interp << ")  \n\tZ = " << uniqueLogZ[z_Down] << " -> " << uniqueLogZ[z_Up] << "(fac = " << z_interp << ")\n\t Giving tau = " << log10(interpolatedValue) << std::endl;
		}
	}
}
