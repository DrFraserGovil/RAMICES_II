#include "IsochroneTracker.h"



void IsochroneTracker::IsoLog(std::string val)
{
	if (Param.Meta.Verbosity > 0)
	{
		std::cout << "\t" << val << std::endl;
	}
}

//gets first id such that y[id] == x. If no such id exists, returns negative value
template<class T>
int FindXInY(T x, std::vector<T> y)
{
	for (int j = 0; j < y.size(); ++j)
	{
		if (abs((y[j] - x)/x) < 1e-6)
		{
			return j; 
		}
	}
	return -1;
}
template <typename T>
std::vector<size_t> sortIndices(const std::vector<T> &v) {

  // initialize original index locations
  std::vector<size_t> idx(v.size());
  std::iota(idx.begin(), idx.end(), 0);

  // sort indexes based on comparing values in v
  // using std::stable_sort instead of std::sort
  // to avoid unnecessary index re-orderings
  // when v contains elements of equal values 
  stable_sort(idx.begin(), idx.end(),
       [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

  return idx;
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
	
	std::vector<size_t> sorter = sortIndices(CapturedZs);
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
	//~ std::vector<double> logZs = {-3,-2,-1,0};
	//~ std::vector<double> ages = {0,0.01,0.1,1,5,10};
	
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
				
				//~ std::cout << "The following properties were retrieved for a star with M=" << mass << ", z = " << Z << " and age " << age << "Gyr" << std::endl;
				//~ IsochroneCube iso = GetProperties(mID,Z,age);
				
				
				//~ for (int n = 0; n < iso.Weighting.size(); ++n)
				//~ {
					//~ std::cout << "\tFrom " << std::setw(10) << iso.Ts[n] << "  " << std::setw(10)  <<iso.Zs[n] << ", with weighting " << std::setw(10) << iso.Weighting[n] << " Vmag = " << iso.Data[n]->Properties[VMag] <<std::endl;
				//~ }
			//~ }
		//~ }
	//~ }
}
double bounder(double a)
{
	return std::min( std::max(0.0,a),1.0);
}


int upperBounder(double val, const std::vector<double> & valArray)
{
	int id = 0;
	bool stillSearching = true;
	while (stillSearching)
	{
		if (valArray[id] > val)
		{
			stillSearching = false;
		}
		else
		{
			++id;
			if (id>= valArray.size())
			{
				stillSearching = false;
				id = valArray.size() - 1;
			}
		}
	}
	return id;
}


IsochroneCube IsochroneTracker::GetProperties(int m_ID, double z, double age)
{
	//~ age = 0;
	int z_ID = upperBounder(z,CapturedZs);
	z_ID = std::min((int)CapturedZs.size()-1, std::max(1,z_ID)); //z_ID is upper bound on Z
	int lower_z = z_ID - 1;
	int upper_z = z_ID;
	double zWeight = bounder( (log10(z) - log10(CapturedZs[lower_z])) / (log10(CapturedZs[upper_z]) - log10(CapturedZs[lower_z])	) );
	
	int Nt = Param.Catalogue.TemporalSpoofResolution;
	double ddt = Param.Meta.TimeStep / (Nt- 1);
	double baseTimeWeighting = 1.0/Nt;
	
	IsochroneCube output;
	output.Data.resize(0);
	output.Weighting.resize(0);

	for (int t = 0; t < Nt; ++t)
	{ 
	
		double mockAge = age + t * ddt;
			
		double logAge = log10(mockAge) + 9;
		int t_ID = (logAge - CapturedTs[0])/DeltaLogT;
		t_ID = std::min((int)CapturedTs.size()-2, std::max(3,t_ID)); // t_ID is lower bound on time
		int lower_t = t_ID;
		int upper_t = t_ID + 1;
		
		
		double mockUpAge = pow(10,CapturedTs[upper_t]-9);
		double mockDownAge = pow(10,CapturedTs[lower_t]-9);
		double tWeight = (mockAge - mockDownAge)/(mockUpAge - mockDownAge);
		tWeight = bounder(tWeight);
		
		
		int upper_z_maxAge = Grid[m_ID][upper_z].size();
		int lower_z_maxAge = Grid[m_ID][lower_z].size();
		double lowZ = CapturedZs[lower_z];
		double upZ = CapturedZs[upper_z];
		if (lower_t < upper_z_maxAge)
		{
			double w = zWeight * (1.0 - tWeight) * baseTimeWeighting;
			output.Data.push_back(&Grid[m_ID][upper_z][lower_t]);
			output.Weighting.push_back(w);
			output.Zs.push_back(upZ);
			output.Ts.push_back(mockDownAge);
			if (upper_t < upper_z_maxAge)
			{
				w = zWeight * (tWeight) * baseTimeWeighting;
				output.Data.push_back(&Grid[m_ID][upper_z][upper_t]);
				output.Weighting.push_back(w);
				output.Zs.push_back(upZ);
				output.Ts.push_back(mockUpAge);
			}
			else
			{
				int s = output.Weighting.size();
				output.Weighting[s-1] =  zWeight * tWeight * baseTimeWeighting;;
			}	
		}
		if (lower_t < lower_z_maxAge)
		{
			double w = (1.0 - zWeight) * (1.0 - tWeight) * baseTimeWeighting;
			output.Data.push_back(&Grid[m_ID][lower_z][lower_t]);
			output.Weighting.push_back(w);
			output.Zs.push_back(lowZ);
			output.Ts.push_back(mockDownAge);
			if (upper_t < lower_z_maxAge)
			{				
				w = (1.0-zWeight) * (tWeight) * baseTimeWeighting;
				output.Data.push_back(&Grid[m_ID][lower_z][upper_t]);
				output.Weighting.push_back(w);
				output.Zs.push_back(lowZ);
				output.Ts.push_back(mockUpAge);
			}
			else
			{
				int s = output.Weighting.size();
				output.Weighting[s-1] = (1.0 - zWeight) * baseTimeWeighting;;
			}
		}
			
			
		
	}
	
	int n = output.Weighting.size();
	double total = 0;
	for (int i = 0; i < n; ++i)
	{
		total += output.Weighting[i];
	}
	for (int i = 0; i < n; ++i)
	{
		output.Weighting[i]/=total;
	}
	
	return output;
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
			int z_ID = FindXInY(z,CapturedZs);
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
			int t_ID = FindXInY(time,CapturedTs);
			if (t_ID < 0)
			{
				CapturedTs.push_back(time);
				Nt = CapturedTs.size();
				t_ID = Nt - 1;
			}
			
			
			int flag = std::stoi(FILE_LINE_VECTOR[9]);
			if (flag < 8 && flag > 0)
			{
				for (int k = 0; k < PropertyCount; ++k)
				{
					CurrentIso.Properties[k] = std::stod(FILE_LINE_VECTOR[ColumnIDs[k]]);
				}
				
				double mass = std::stod(FILE_LINE_VECTOR[3]);
				while (m_ID < Param.Stellar.MassResolution && Param.Stellar.MassGrid[m_ID] <= mass)
				{
					
					double interp = (Param.Stellar.MassGrid[m_ID] - prevMass)/(mass - prevMass);
					
					NewIso = PrevIso;
					if (interp > 0.5)
					{
						NewIso = CurrentIso;
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
