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
	UnsortedGrid.resize(0);
}
double bounder(double a)
{
	return std::min( std::max(0.0,a),1.0);
}
std::vector<IsochroneEntry> IsochroneTracker::GetProperties(std::vector<int> mass, double z, double age)
{
	//supposedly runs quicker than a brute force method!
	
	//~ age = 1e-3;
	//~ z = 0.001;
	
	auto closestZID = std::upper_bound(CapturedZs.begin(),CapturedZs.end(), z);
	int corresponding = closestZID - CapturedZs.begin();
	int zMax = CapturedZs.size() - 1;
	int zUpperID = std::min(std::max(1,corresponding),zMax);
	
	int zLowerID = zUpperID - 1;
	
	int tUpperID;
	int tLowerID;
	double startTime = CapturedTs[0];
	double endTime = CapturedTs[CapturedTs.size() - 1];
	double logAge = log10(age)+9;
	if (logAge < startTime)
		logAge = startTime;
	if (logAge > endTime)
		logAge = endTime;
	
	if (isTimeLogUniform)
	{		
		double delta = std::max(0.0,logAge - startTime);
		tLowerID = floor(delta / DeltaLogT);
		tUpperID = ceil(delta / DeltaLogT);
		if (tLowerID == tUpperID)
		{
			++ tUpperID;
		}
	}
	else //manually search!
	{
		tUpperID = 1;
		while ( tUpperID < CapturedTs.size() && CapturedTs[tUpperID] > logAge)
		{
			++tUpperID;
		}
		tLowerID = tUpperID - 1;
	}

	std::vector<IsochroneEntry> output(mass.size());
	double zInterp = bounder(log10(z / CapturedZs[zLowerID]) / log10(CapturedZs[zUpperID]/CapturedZs[zLowerID]));
	
	for (int i = 0; i < mass.size(); ++i)
	{
		int highZT = tLowerID;
		int lowZT = tLowerID;
		
		bool modified = false;
		while (Grid[zUpperID][highZT].size() <= mass[i])
		{
			--highZT;
			modified = true;
			if (highZT < 0)
			{
				std::cout << "Something has gone horribly wrong with your isochrones!" << std::endl;
				exit(5);
			}
		}
		while (Grid[zLowerID][lowZT].size() <= mass[i])
		{
			--lowZT;
			modified = true;
			if (lowZT < 0)
			{
				std::cout << "Something has gone horribly wrong with your isochrones!" << std::endl;
				exit(5);
			}
		}

		const IsochroneEntry & downTupZ = Grid[zUpperID][highZT][mass[i]];
		const IsochroneEntry & downTdownZ = Grid[zLowerID][lowZT][mass[i]];

		//~ std::cout << "About to insert things" <<std::endl;
		for (int k = 0; k < PropertyCount; ++k)
		{
			double vTDown = downTdownZ.Properties[k] + zInterp * (downTupZ.Properties[k] - downTdownZ.Properties[k]);			
			output[i].Properties[k] = vTDown;
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
					
					for (int k = 0; k < PropertyCount; ++k)
					{
						NewIso.Properties[k] = PrevIso.Properties[k] + interp * (CurrentIso.Properties[k] - PrevIso.Properties[k]); 
					}
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
