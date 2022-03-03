#include "IsochroneTracker.h"



void IsochroneTracker::IsoLog(std::string val)
{
	if (Param.Meta.Verbosity > 0)
	{
		std::cout << "\t" << val << std::endl;
	}
}




IsochroneTracker::IsochroneTracker(const GlobalParameters & param): Param(param)
{
	IsoLog("Isochrone Tracker Initialised.\n\t\tPlease Note: Isochrone Grid not populated until explicitly called!");
}

void IsochroneTracker::Construct()
{
	IsoLog("Constructing Isochrone Grids\n\t\tReading data from file");
	std::vector<double> zs;
	for (const auto & entry : std::filesystem::directory_iterator(Param.Resources.IsochroneRepository.Value))
	{
		std::string fileLoc = entry.path();
        ParseFile(fileLoc);
	}

	Grid.resize(UnsortedGrid.size());
	
	std::vector<size_t> sorter = JSL::SortIndices(CapturedZs);
	std::vector<double> zCopy = CapturedZs;
	for (int j = 0; j < UnsortedGrid.size(); ++j)
	{
		Grid[j].resize(UnsortedGrid[j].size());
		for (int i = 0; i < sorter.size(); ++i) 
		{
			Grid[j][i] = UnsortedGrid[j][sorter[i]];
			UnsortedGrid[j][sorter[i]].resize(0);
			CapturedZs[i] = zCopy[sorter[i]];
		}
	}
	IsoLog("\tSorted Isochrones by metallicity");
	
	//check temporal grid for uniformity
	isTimeLogUniform = true;
	DeltaLogT = CapturedTs[1] - CapturedTs[0];
	for (int i = 2; i < CapturedTs.size(); ++i)
	{
		double dt = CapturedTs[i] - CapturedTs[i-1];
		if (abs(DeltaLogT - dt)/DeltaLogT > 1e-3)
		{
			isTimeLogUniform = false;
			IsoLog("\tIsochrones are not log-unform. Brute force methods employed for indexing");
			break;
		}
	}
	if (isTimeLogUniform)
	{
		IsoLog("\tIsochrones are log-uniform. Analytical methods employed for indexing");
	}
	UnsortedGrid.resize(0); //clear the memory so only have 1 copy of the grid!
	
	
	//~ std::cout << "Attempting a test....." << std::endl;
	//~ std::vector<int> ms = {1,50,150,280};
	//~ std::vector<double> logZs = {-3,-2,-1};
	//~ std::vector<double> ages = {0,0.1,1,10};
	
	//~ for (int m = 0; m < ms.size(); ++m)
	//~ {
		//~ int mID = ms[m];
		//~ double mass = Param.Stellar.MassGrid[mID];
		//~ for (int z = 0; z < logZs.size(); ++z)
		//~ {
			//~ double Z = pow(10,logZs[z]);
			//~ for (int y = 0; y < ages.size(); ++y)
			//~ {
				//~ double age = ages[y];
				
				
				
				//~ int z_ID = upperBounder(Z,CapturedZs);
				//~ z_ID = std::min((int)CapturedZs.size()-1, std::max(1,z_ID)); //z_ID is upper bound on Z
				//~ int lower_z = z_ID - 1;
				//~ int upper_z = z_ID;
				//~ int t_Up = Grid[mID][upper_z].size() - 1;
				//~ int t_Down = Grid[mID][lower_z].size() - 1;
				
				//~ IsochroneCube iso = GetProperties(mID,Z,age);
				//~ int N = iso.Count();
				
				//~ if (N > 0)
				//~ {
					
					//~ std::cout << "The following properties were retrieved for a star with M=" << mass << ", z = " << Z << " and age " << age << "Gyr" << std::endl;
					//~ std::cout << "\tI predict the max lifetime to be in the range:" <<  pow(10,CapturedTs[std::max(t_Down,t_Up)]-9) << "Gyr" << std::endl;
					//~ for (int n = 0; n < N; ++n)
					//~ {
						//~ std::cout << "\t\tSampled from M =" << std::setw(10) << iso.Ms[n] << ", Z = " << iso.Zs[n] << " Age = " << pow(10,iso.Ts[n]-9) << " Vmag = " << iso.Value(n,VMag)<<std::endl;
					//~ }
				//~ }
				//~ else
				//~ {
					//~ std::cout << "No data was available for stars of m=" << mass << ", z = " << Z << " and age " << age << "Gyr" << std::endl;
				//~ }
			//~ }
		//~ }
	//~ }

}



IsochroneCube IsochroneTracker::GetProperties(int massID, double z, double age)
{
	int N = Param.Catalogue.SampleCount;
	IsochroneCube output;
	for (int n = 0; n < N; ++n)
	{
		double logZ = log10(z);
		double sigmaZ = std::min(0.1,abs(logZ)*0.2);
		double sampledLogZ = NormalSample(logZ,sigmaZ);
		

		double sampledAge = UniformSample(age,age + Param.Meta.TimeStep);
		int sampledMass = massID;
		//~ sampledMass = std::min(Param.Stellar.MassResolution -1, std::max(0,sampledMass));
		ExtractSample(output,sampledMass, pow(10,sampledLogZ),sampledAge);
		
		
	}
	return output;
}

void IsochroneTracker::ExtractSample(IsochroneCube & output, int sampleMass, double sampleZ, double sampleAge)
{
	int z_ID = JSL::UpperBoundLocator(sampleZ,CapturedZs);
	z_ID = std::min((int)CapturedZs.size()-1, std::max(1,z_ID)); //z_ID is upper bound on Z
	int lower_z = z_ID - 1;
	int upper_z = z_ID;
	double zWeight = JSL::FractionBounder( (log10(sampleZ) - log10(CapturedZs[lower_z])) / (log10(CapturedZs[upper_z]) - log10(CapturedZs[lower_z])	) );
	if (zWeight < 0.5)
	{
		z_ID = lower_z;
	}
	double latchedZ = CapturedZs[z_ID];
	
	
	double logAge = log10(sampleAge) + 9;
	int t_ID = (logAge - CapturedTs[0])/DeltaLogT;
	t_ID = std::min((int)CapturedTs.size()-2, std::max(0,t_ID)); // t_ID is lower bound on time
	int lower_t = t_ID;
	int upper_t = t_ID + 1;
	
	
	double mockUpAge = pow(10,CapturedTs[upper_t]-9);
	double mockDownAge = pow(10,CapturedTs[lower_t]-9);
	double tWeight = (sampleAge - mockDownAge)/(mockUpAge - mockDownAge);
	tWeight = JSL::FractionBounder(tWeight);
	
	if (tWeight > 0.5)
	{
		t_ID = upper_t;
	}
	double latchedTs = CapturedTs[t_ID];
	double latchedMs = Param.Stellar.MassGrid[sampleMass];
	
	
	if (Grid[sampleMass][z_ID].size() > t_ID)
	{
		output.Data.push_back(&Grid[sampleMass][z_ID][t_ID]);
	}
}

void IsochroneTracker::ParseFile(std::string file)
{

	IsochroneEntry PrevIso;
	IsochroneEntry CurrentIso;
	IsochroneEntry NewIso;
	double prevMass = 0;
	int m_ID = 0;
	std::vector<int> ColumnIDs(PropertyCount);
	ColumnIDs[logL] = 6;
	ColumnIDs[TEff] = 7;
	ColumnIDs[Logg] = 8;
	ColumnIDs[BolometricMag] = 27;
	ColumnIDs[UMag] = 28;
	ColumnIDs[BMag] = 29;
	ColumnIDs[VMag] = 30;
	ColumnIDs[RMag] = 31;
	ColumnIDs[IMag] = 32;
	ColumnIDs[JMag] = 33;
	ColumnIDs[HMag] = 34;
	ColumnIDs[KMag] = 35;
	
	int Nm = Param.Stellar.MassResolution;
	int Nz = CapturedZs.size();
	int Nt = CapturedTs.size();
	UnsortedGrid.resize(Nm);
	
	forLineVectorIn(file, ' ',
		
		std::string firstChar = FILE_LINE_VECTOR[0];
		if (firstChar != "#" && firstChar != "#isochrone")
		{
			double z = std::stod(firstChar);
			int z_ID = JSL::FindXInY(z,CapturedZs);
			if (z_ID < 0)
			{
				//resize
				CapturedZs.push_back(z);
				Nz = CapturedZs.size();
				z_ID = Nz - 1;
				for (int m = 0; m < Nm; ++m)
				{
					UnsortedGrid[m].resize(Nz);
				}
			}
			
			
			double time = std::stod(FILE_LINE_VECTOR[2]);
			int t_ID = JSL::FindXInY(time,CapturedTs);
			if (t_ID < 0)
			{
				CapturedTs.push_back(time);
				Nt = CapturedTs.size();
				t_ID = Nt - 1;
			}
			
			
			int flag = std::stoi(FILE_LINE_VECTOR[9]);
			if (flag < 8)
			{
				for (int k = 0; k < PropertyCount; ++k)
				{
					CurrentIso.Properties[k] = std::stod(FILE_LINE_VECTOR[ColumnIDs[k]]);
				}
				
				double mass = std::stod(FILE_LINE_VECTOR[3]);
				if (prevMass == 0)
				{
					PrevIso = CurrentIso;
					prevMass = mass*0.999;
				}
				while (m_ID < Param.Stellar.MassResolution && Param.Stellar.MassGrid[m_ID] <= mass)
				{
					
					double interp = (Param.Stellar.MassGrid[m_ID] - prevMass)/(mass - prevMass);
					
					NewIso = PrevIso;
					
					
					
					for (int k = 0; k < PropertyCount; ++k)
					{
						IsochroneProperties kk = (IsochroneProperties) k;
						NewIso[kk] = PrevIso[kk] + interp * (CurrentIso[kk] - PrevIso[kk]);
					}
					
					
					if (UnsortedGrid[m_ID][z_ID].size() <= t_ID)
					{
						UnsortedGrid[m_ID][z_ID].resize(t_ID+1);
					}
					UnsortedGrid[m_ID][z_ID][t_ID] = NewIso;
					//~ UnsortedGrid[zID][tID].push_back(NewIso);
					++m_ID;
					
				}
				
				PrevIso = CurrentIso;
				prevMass = mass;
			}
		}
		else
		{
			m_ID = 0;
			prevMass = 0;
		}
	);
	
}

double IsochroneTracker::NormalSample(double mu, double sigma)
{
	return mu + sigma * distribution(generator);
}
double IsochroneTracker::UniformSample(double a, double b)
{
	double r = (double)rand() / RAND_MAX;
	return a + r * (b - a);
}
