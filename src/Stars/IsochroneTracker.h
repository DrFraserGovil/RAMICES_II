#pragma once
#include "../Parameters/GlobalParameters.h"
#include <filesystem>
#include <numeric>      // std::iota
#include <algorithm>  

enum IsochroneProperties {logL,BolometricMag, UMag,BMag,VMag, RMag,IMag,JMag, HMag,KMag, TEff, Logg, PropertyCount};
const std::vector<std::string> PropertyNames = {"logL", "BolometricMag", "UMag","BMag","VMag", "RMag","IMag","JMag","HMag","KMag","TEff","Logg"};
struct IsochroneEntry
{
	std::vector<double> Properties;
	IsochroneEntry()
	{
		Properties = std::vector<double>(PropertyCount,0.0);
	}
	double & operator[](IsochroneProperties p) 
	{
		return Properties[p];
	}
	const double & operator[](IsochroneProperties p) const 
	{
		return Properties[p];
	}
};


struct IsochroneCube
{
	std::vector<double> Weighting;
	std::vector<IsochroneEntry *> Data;
	std::vector<double> Zs;
	std::vector<double> Ts;
};
class IsochroneTracker
{
	
	
	public:
		IsochroneTracker(const GlobalParameters & param);
		void Construct();
		
		std::vector<IsochroneCube> GetProperties(std::vector<int> mass, double z, double age);
		
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
