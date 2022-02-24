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
		if (abs(y[j] - x)/x < 1e-6)
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
	for (int i = 0; i < sorter.size(); ++i) 
	{
		Grid[i] = UnsortedGrid[sorter[i]];
		UnsortedGrid[sorter[i]].resize(0);
		CapturedZs[i] = zCopy[sorter[i]];
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
}
double bounder(double a)
{
	return std::min( std::max(0.0,a),1.0);
}
std::vector<IsochroneCube> IsochroneTracker::GetProperties(std::vector<int> mass, double z, double age)
{
	int z_ID = 0;
	bool stillSearching = true;
	while (stillSearching)
	{
		if (CapturedZs[z_ID] > z)
		{
			stillSearching = false;
		}
		else
		{
			++z_ID;
			if (z_ID >= CapturedZs.size())
			{
				stillSearching = false;
				z_ID = CapturedZs.size() - 1;
			}
		}
	}
	double logAge = log10(age) + 9;
	int t_ID = (logAge - CapturedTs[0])/DeltaLogT;
	z_ID = std::min((int)CapturedZs.size()-1, std::max(1,z_ID)); //z_ID is upper bound on Z
	t_ID = std::min((int)CapturedTs.size()-1, std::max(0,t_ID)); // t_ID is lower bound on time
	
	
	int lower_t = t_ID;
	int upper_t = t_ID + 1;
	int lower_z = z_ID - 1;
	int upper_z = z_ID;
	
	double tWeight = (logAge - CapturedTs[lower_t])/(CapturedTs[upper_t] - CapturedTs[lower_t]);
	double zWeight = (z - CapturedZs[lower_z]) / (CapturedTs[upper_z] - CapturedZs[lower_z]);
	
	
	
	

	std::vector<IsochroneCube> output(mass.size());
	
	std::vector<double> ts;
	std::vector<double> zs;
	for (int m = 0; m < mass.size(); ++m)
	{
		
		int m_ID = mass[m];
		double mm = Param.Stellar.MassGrid[m_ID];
		int temp_upper_t = upper_t;
		
		int a = Grid[lower_z][upper_t].size();
		int b = Grid[upper_z][upper_t].size();
		int maxSize = std::max(a,b);
		int modified = 0;
		double tempWeight = tWeight;
		while (maxSize -2<= m_ID)
		{
			--temp_upper_t;
			a = Grid[lower_z][temp_upper_t].size();
			b = Grid[upper_z][temp_upper_t].size();
			maxSize = std::max(a,b);
			++modified;
			if (temp_upper_t < 1)
			{
				maxSize = m_ID + 10;
			}
			tempWeight = 0;
		}
		
	
		
		temp_upper_t = std::max(1,temp_upper_t);
		int temp_lower_t = temp_upper_t - 1;
		
		
		
		//older, low metal
		
		
		double totalWeighting = 0;
		if (Grid[lower_z][temp_upper_t].size() > m_ID)
		{
			output[m].Data.push_back(Grid[lower_z][temp_upper_t][m_ID]);
			double w = tempWeight * (1.0 - zWeight);
			output[m].Weighting.push_back(w);
			totalWeighting += w;
			
			ts.push_back(CapturedTs[temp_upper_t]);
			zs.push_back(CapturedZs[lower_z]);
		}
		if (Grid[lower_z][temp_lower_t].size() > m_ID)
		{
			output[m].Data.push_back(Grid[lower_z][temp_lower_t][m_ID]);
			double w = (1.0 - tempWeight) * (1.0 - zWeight);
			output[m].Weighting.push_back(w);
			totalWeighting += w;
			
			ts.push_back(CapturedTs[temp_lower_t]);
			zs.push_back(CapturedZs[lower_z]);
		}
		
		//older, low metal
		
		
		if (Grid[upper_z][temp_upper_t].size() > m_ID)
		{
			output[m].Data.push_back(Grid[upper_z][temp_upper_t][m_ID]);
			double w = tempWeight * zWeight;
			output[m].Weighting.push_back(w);
			totalWeighting += w;
			ts.push_back(CapturedTs[temp_upper_t]);
			zs.push_back(CapturedZs[upper_z]);
		}
		if (Grid[upper_z][temp_lower_t].size() > m_ID)
		{
			output[m].Data.push_back(Grid[upper_z][temp_lower_t][m_ID]);
			double w = (1.0 - tempWeight) * zWeight;
			output[m].Weighting.push_back(w);
			totalWeighting += w;
			ts.push_back(CapturedTs[temp_lower_t]);
			zs.push_back(CapturedZs[upper_z]);
		}
		
		int n = output[m].Weighting.size();
		if (n == 0)
		{
			std::cout << "ERROR! Could not find any isochrones for " << mm << "  " << z << "  " << age << "  " << maxSize << "  " << m_ID << std::endl;
			exit(5);
		}
		//~ std::cout << "

		for (int q = 0; q < n; ++q)
		{
			output[m].Weighting[q]/=totalWeighting;
		}
		
		std::cout << "For m = " << mm <<"  age = " << logAge << "  z = " << z << " I found the following isochronic data:\n";
		for (int q = 0; q < n; ++q)
		{
			std::cout << "(tau,z) =(" << ts[q] << ", " << zs[q] << "),   w = " << output[m].Weighting[q] << std::endl;
		}
	}
	return output;
}

void IsochroneTracker::ParseFile(std::string file)
{
	double prevZ = -10;
	double prevT = -10;
	int zID = 0;
	int tID = 0;
	int mID = 0;
	IsochroneEntry PrevIso;
	IsochroneEntry CurrentIso;
	IsochroneEntry NewIso;
	double prevMass = 0;
	
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
	forLineVectorIn(file, ' ',
		std::string firstChar = FILE_LINE_VECTOR[0];
		if (firstChar != "#" && firstChar != "#isochrone")
		{
			double z = std::stod(firstChar);
			if (abs((z - prevZ)/prevZ) > 1e-3)
			{
				CapturedZs.push_back(z);
				zID = CapturedZs.size() -1;
				prevZ = z;
				
				std::vector<IsochroneEntry> m;
				UnsortedGrid.push_back( std::vector<std::vector<IsochroneEntry>>(CapturedTs.size(), m) );

				mID = 0;
			}
			double time = std::stod(FILE_LINE_VECTOR[2]);
			double tID = FindXInY(time,CapturedTs);
			if (tID < 0)
			{
				CapturedTs.push_back(time);
				tID = CapturedTs.size() -1;
				for (int i = 0; i < CapturedZs.size(); ++i)
				{
					UnsortedGrid[i].resize(CapturedTs.size());
					//~ for (int k = 0; k < CapturedTs.size(); ++k)
					//~ {
						//~ UnsortedGrid[i][k].resize(Param.Stellar.MassResolution);
					//~ }
				}
				
			}
			if (abs( (prevT - time)/prevT) > 1e-3)
			{
				mID = 0;
			}
			prevT = time;
			
			//~ std::cout << zID << "  " << tID << "  " << UnsortedGrid.size() << "  " << UnsortedGrid[0].size() << std::endl;
			
			//~ std::cout << "FLAG CHECK = " << FILE_LINE_VECTOR[9] << std::endl;
			int flag = std::stoi(FILE_LINE_VECTOR[9]);
			if (flag < 8)
			{
				for (int k = 0; k < PropertyCount; ++k)
				{
					CurrentIso.Properties[k] = std::stod(FILE_LINE_VECTOR[ColumnIDs[k]]);
				}
				
				double mass = std::stod(FILE_LINE_VECTOR[3]);
				while (mID < Param.Stellar.MassResolution && Param.Stellar.MassGrid[mID] <= mass)
				{
					
					double interp = (Param.Stellar.MassGrid[mID] - prevMass)/(mass - prevMass);
					
					NewIso = CurrentIso;
					//~ if (interp > 0.5)
					//~ {
						//~ NewIso = CurrentIso;
					//~ }
					
					//~ for (int k = 0; k < PropertyCount; ++k)
					//~ {
						//~ NewIso.Properties[k] = PrevIso.Properties[k] + interp * (CurrentIso.Properties[k] - PrevIso.Properties[k]); 
					//~ }
					UnsortedGrid[zID][tID].push_back(NewIso);
					++mID;
					//~ std::cout << "\tI just used " << prevMass << " < " << Param.Stellar.MassGrid[mID] << " < " << mass << " at " << zID << "  " << tID << std::endl;
				}
				
				PrevIso = CurrentIso;
				prevMass = mass;
			}
			//~ std::cout <<"Loop complete" << std::endl;
		}
	
	);
	
}
