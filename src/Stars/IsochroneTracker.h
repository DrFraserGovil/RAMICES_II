#pragma once
#include "../Parameters/GlobalParameters.h"
#include <filesystem>
#include <numeric>      // std::iota
#include <algorithm>  

enum IsochroneProperties {logL,BolometricMag, UMag,BMag,VMag, RMag,IMag,JMag, HMag,KMag, TEff, Logg, PropertyCount};
struct IsochroneEntry
{
	std::vector<double> Properties;
	IsochroneEntry()
	{
		Properties = std::vector<double>(PropertyCount,0.0);
	}
};
class IsochroneTracker
{
	
	
	public:
		IsochroneTracker(const GlobalParameters & param);
		void Construct();
		
		std::vector<IsochroneEntry> GetProperties(std::vector<int> mass, double z, double age);
		
	private:
		const GlobalParameters Param;
		void IsoLog(std::string val);
		void ParseFile(std::string file);
		std::vector<double> CapturedZs;
		std::vector<double> CapturedTs;
		std::vector<std::vector<std::vector<IsochroneEntry>>> Grid;
		std::vector<std::vector<std::vector<IsochroneEntry>>> UnsortedGrid;
		bool isTimeLogUniform;
		double DeltaLogT;
};
